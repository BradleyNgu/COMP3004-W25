#include "pumpcontroller.h"
#include <QDir>
#include <QDateTime>
#include <QRandomGenerator>
#include <QSettings>

PumpController::PumpController(QObject *parent)
    : QObject(parent),
      running(false),
      controlIQEnabled(true),
      simulationSpeedFactor(30) // Simulation runs 30x faster than real-time
{
    // Initialize models
    pumpModel = new PumpModel(this);
    profileModel = new ProfileModel(this);
    glucoseModel = new GlucoseModel(this);
    insulinModel = new InsulinModel(this);
    controlIQAlgorithm = new ControlIQAlgorithm(this);
    dataStorage = new DataStorage(this);
    errorHandler = new ErrorHandler(this);
    
    // Connect error handler to data storage for history recording
    errorHandler->setHistoryManager(dataStorage);
    
    // Make sure we create an alert controller
    alertController = new AlertController(this);
    alertController->setPumpModel(pumpModel);
    alertController->setGlucoseModel(glucoseModel);
    alertController->setInsulinModel(insulinModel);

    // Start monitoring
    alertController->startMonitoring();

    // Connect alert controller signals
    connect(alertController, &AlertController::criticalAlertActive, this, [this](const QString &message) {
        emit alertTriggered(message, PumpModel::Critical);
    });
    
    // Set up timers
    setupTimers();
    
    // Connect model signals
    connectModelSignals();
    
    // Try to load data if available
    loadPumpState();
    
    // Initialize simulator with 48 hours of data
    initializeSimulator();
}

PumpController::~PumpController()
{
    // Save state before shutdown
    savePumpState();
    
    // Stop timers
    batteryTimer->stop();
    glucoseTimer->stop();
    iobTimer->stop();
    controlIQTimer->stop();
    reminderTimer->stop();
    occlusionTimer->stop();
    basalConsumptionTimer->stop();
}

void PumpController::initializeSimulator() {
    // Generate 48 hours of fixed pattern glucose data
    glucoseModel->generateFixedPattern(48);
    
    // Generate matching insulin delivery history
    generateHistoricalInsulinData(48);
    
    // Start the pump with our prepared data
    if (!running) {
        startPump();
    }
}

void PumpController::generateHistoricalInsulinData(int hoursBack) {
    QDateTime current = QDateTime::currentDateTime();
    QDateTime start = current.addSecs(-hoursBack * 3600);
    
    // Get default profile
    Profile defaultProfile = profileModel->getActiveProfile();
    double basalRate = defaultProfile.basalRate;
    
    // Generate basal history segments in 4-hour blocks
    QDateTime segmentStart = start;
    while (segmentStart < current) {
        QDateTime segmentEnd = segmentStart.addSecs(4 * 3600);
        if (segmentEnd > current) {
            segmentEnd = current;
        }
        
        // Determine if this should be a Control-IQ segment or regular basal
        bool isControlIQ = QRandomGenerator::global()->bounded(100) < 70; // 70% chance of Control-IQ
        
        // Slightly vary the basal rate for Control-IQ segments
        double adjustedRate = basalRate;
        if (isControlIQ) {
            adjustedRate = basalRate + (QRandomGenerator::global()->generateDouble() - 0.5) * 0.6;
            adjustedRate = qMax(0.1, adjustedRate); // Ensure positive
        }
        
        // Create the basal segment
        InsulinModel::BasalDelivery segment;
        segment.startTime = segmentStart;
        segment.endTime = segmentEnd;
        segment.rate = adjustedRate;
        segment.profileName = defaultProfile.name;
        segment.automatic = isControlIQ;
        
        // Add to history
        insulinModel->addBasalToHistory(segment);
        
        // Move to next segment
        segmentStart = segmentEnd;
    }
    
    // Add boluses for each day in the history
    QDateTime day = start;
    while (day < current) {
        // Breakfast bolus (around 7:15 AM)
        QDateTime breakfastTime = day;
        breakfastTime.setTime(QTime(7, 15));
        if (breakfastTime >= start && breakfastTime <= current) {
            double units = 4.0 + (QRandomGenerator::global()->generateDouble() - 0.5) * 1.0;
            insulinModel->addBolusToHistory(breakfastTime, units, "Breakfast", false, 0, true);
        }
        
        // Lunch bolus (around 12:30 PM)
        QDateTime lunchTime = day;
        lunchTime.setTime(QTime(12, 30));
        if (lunchTime >= start && lunchTime <= current) {
            double units = 5.0 + (QRandomGenerator::global()->generateDouble() - 0.5) * 1.5;
            insulinModel->addBolusToHistory(lunchTime, units, "Lunch", false, 0, true);
        }
        
        // Dinner bolus (around 6:45 PM)
        QDateTime dinnerTime = day;
        dinnerTime.setTime(QTime(18, 45));
        if (dinnerTime >= start && dinnerTime <= current) {
            double units = 6.0 + (QRandomGenerator::global()->generateDouble() - 0.5) * 2.0;
            
            // Sometimes make this an extended bolus
            bool extended = QRandomGenerator::global()->bounded(100) < 30; // 30% chance
            int duration = extended ? QRandomGenerator::global()->bounded(1, 4) * 30 : 0; // 30-120 minutes
            
            insulinModel->addBolusToHistory(dinnerTime, units, "Dinner", extended, duration, true);
        }
        
        // Random correction bolus (afternoon or evening)
        if (QRandomGenerator::global()->bounded(100) < 40) { // 40% chance per day
            QDateTime correctionTime = day;
            int hour = QRandomGenerator::global()->bounded(14, 22); // 2 PM to 10 PM
            correctionTime.setTime(QTime(hour, QRandomGenerator::global()->bounded(60)));
            
            if (correctionTime >= start && correctionTime <= current) {
                double units = 1.5 + QRandomGenerator::global()->generateDouble() * 1.5;
                insulinModel->addBolusToHistory(correctionTime, units, "Correction", false, 0, true);
            }
        }
        
        // Move to next day
        day = day.addDays(1);
    }
    
    // Update IOB based on generated history
    insulinModel->updateIOB();
}
    
void PumpController::setupTimers()
{
    // Battery drain timer (every 5 minutes in sim time)
    batteryTimer = new QTimer(this);
    connect(batteryTimer, &QTimer::timeout, this, &PumpController::simulateBatteryDrain);
    batteryTimer->setInterval(300000 / simulationSpeedFactor); // 5 minutes / speed factor
    
    // Glucose reading timer (every 5 minutes in sim time)
    glucoseTimer = new QTimer(this);
    connect(glucoseTimer, &QTimer::timeout, this, &PumpController::simulateGlucoseReading);
    glucoseTimer->setInterval(300000 / simulationSpeedFactor); // 5 minutes / speed factor
    
    // Insulin on board timer (every minute in sim time)
    iobTimer = new QTimer(this);
    connect(iobTimer, &QTimer::timeout, this, &PumpController::updateInsulinOnBoard);
    iobTimer->setInterval(60000 / simulationSpeedFactor); // 1 minute / speed factor
    
    // Control-IQ timer (every 5 minutes in sim time)
    controlIQTimer = new QTimer(this);
    connect(controlIQTimer, &QTimer::timeout, this, &PumpController::runControlIQ);
    controlIQTimer->setInterval(300000 / simulationSpeedFactor); // 5 minutes / speed factor
    
    // Reminder check timer (every minute in real time)
    reminderTimer = new QTimer(this);
    connect(reminderTimer, &QTimer::timeout, this, &PumpController::checkReminders);
    reminderTimer->setInterval(60000); // 1 minute (real time)
    
    // Occlusion check timer (rare event, every minute in real time)
    occlusionTimer = new QTimer(this);
    connect(occlusionTimer, &QTimer::timeout, this, &PumpController::checkForOcclusion);
    occlusionTimer->setInterval(60000); // 1 minute (real time)

    // Set up timer for basal consumption updates (every 5 seconds in sim time)
    basalConsumptionTimer = new QTimer(this);
    connect(basalConsumptionTimer, &QTimer::timeout, this, &PumpController::updateBasalConsumption);
    basalConsumptionTimer->setInterval(5000 / simulationSpeedFactor); // 5 seconds / speed factor
}

void PumpController::connectModelSignals()
{
    // PumpModel signals
    connect(pumpModel, &PumpModel::batteryLevelChanged, this, &PumpController::batteryLevelChanged);
    connect(pumpModel, &PumpModel::insulinRemainingChanged, this, &PumpController::insulinRemainingChanged);
    connect(pumpModel, &PumpModel::insulinOnBoardChanged, this, &PumpController::insulinOnBoardChanged);
    connect(pumpModel, &PumpModel::alertAdded, this, &PumpController::alertTriggered);
    
    // GlucoseModel signals
    connect(glucoseModel, &GlucoseModel::newReading, this, &PumpController::processGlucoseReading);
    connect(glucoseModel, &GlucoseModel::trendDirectionChanged, this, &PumpController::glucoseTrendChanged);
    
    // InsulinModel signals
    connect(insulinModel, &InsulinModel::basalRateChanged, this, &PumpController::basalRateChanged);
    connect(insulinModel, &InsulinModel::bolusStarted, this, &PumpController::bolusDeliveryStarted);
    connect(insulinModel, &InsulinModel::bolusCompleted, this, &PumpController::bolusDeliveryCompleted);
    connect(insulinModel, &InsulinModel::bolusCancelled, this, &PumpController::bolusDeliveryCancelled);
    connect(insulinModel, &InsulinModel::insulinOnBoardChanged, this, [this](double units) {
        pumpModel->updateInsulinOnBoard(units);
    });
    connect(insulinModel, &InsulinModel::controlIQAdjustmentChanged, this, [this](double adjustment) {
        emit controlIQActionChanged(adjustment);
    });
    
    // ProfileModel signals
    connect(profileModel, &ProfileModel::activeProfileChanged, this, [this](const QString &name) {
        emit profileChanged(name);
        
        // Apply profile settings when changed
        if (running) {
            Profile profile = profileModel->getProfile(name);
            insulinModel->startBasal(profile.basalRate, name);
        }
    });
}

void PumpController::startPump()
{
    if (running) {
        return;
    }
    
    running = true;
    pumpModel->setPumpState(PumpModel::PoweredOn);
    
    // Start active profile basal
    Profile activeProfile = profileModel->getActiveProfile();
    insulinModel->startBasal(activeProfile.basalRate, activeProfile.name);
    
    // Start simulation
    startSimulation();
    
    emit pumpStarted();
}

void PumpController::stopPump()
{
    if (!running) {
        return;
    }
    
    running = false;
    pumpModel->setPumpState(PumpModel::PoweredOff);
    
    // Stop insulin delivery
    insulinModel->stopBasal();
    
    // Stop simulation
    stopSimulation();
    
    emit pumpStopped();
}

bool PumpController::isPumpRunning() const
{
    return running;
}

int PumpController::getBatteryLevel() const
{
    return pumpModel->getBatteryLevel();
}

bool PumpController::isCharging() const
{
    return pumpModel->isCharging();
}

void PumpController::startCharging()
{
    pumpModel->startCharging();
    
    // Simulate fast charging (1% every few seconds)
    QTimer *chargeTimer = new QTimer(this);
    connect(chargeTimer, &QTimer::timeout, this, [this, chargeTimer]() {
        int level = pumpModel->getBatteryLevel();
        
        if (level < 100) {
            pumpModel->updateBatteryLevel(level + 1);
        } else {
            chargeTimer->stop();
            chargeTimer->deleteLater();
            pumpModel->stopCharging();
        }
    });
    
    chargeTimer->start(3000); // 3 seconds per 1%
    
    emit chargingStateChanged(true);
}

void PumpController::stopCharging()
{
    pumpModel->stopCharging();
    emit chargingStateChanged(false);
}

double PumpController::getInsulinRemaining() const
{
    return pumpModel->getInsulinRemaining();
}

double PumpController::getCurrentBasalRate() const
{
    return insulinModel->getCurrentBasalRate();
}

double PumpController::getInsulinOnBoard() const
{
    return pumpModel->getInsulinOnBoard();
}

double PumpController::getCurrentGlucose() const
{
    return glucoseModel->getCurrentGlucose();
}

QDateTime PumpController::getLastGlucoseReading() const
{
    return glucoseModel->getLastReadingTime();
}

GlucoseModel::TrendDirection PumpController::getGlucoseTrend() const
{
    return glucoseModel->getTrendDirection();
}

double PumpController::getControlIQDelivery() const
{
    return insulinModel->getLastControlIQAdjustment();
}

void PumpController::enableControlIQ(bool enable)
{
    controlIQEnabled = enable;
}

bool PumpController::isControlIQEnabled() const
{
    return controlIQEnabled;
}

void PumpController::setActiveProfile(const QString &profileName)
{
    profileModel->setActiveProfile(profileName);
}

Profile PumpController::getActiveProfile() const
{
    if (!profileModel) {
        return Profile();
    }
    
    return profileModel->getActiveProfile();
}

QString PumpController::getActiveProfileName() const
{
    return profileModel->getActiveProfileName();
}

QVector<Profile> PumpController::getAllProfiles() const
{
    return profileModel->getAllProfiles();
}

bool PumpController::createProfile(const Profile &profile)
{
    return profileModel->createProfile(profile);
}

bool PumpController::updateProfile(const QString &name, const Profile &profile)
{
    return profileModel->updateProfile(name, profile);
}

bool PumpController::deleteProfile(const QString &name)
{
    return profileModel->deleteProfile(name);
}

QVector<QPair<QDateTime, double>> PumpController::getGlucoseHistory(const QDateTime &start, const QDateTime &end) const
{
    return glucoseModel->getReadings(start, end);
}

QVector<QPair<QDateTime, double>> PumpController::getInsulinHistory(const QDateTime &start, const QDateTime &end) const
{
    QVector<QPair<QDateTime, double>> result;
    
    // Get bolus history
    QVector<InsulinModel::BolusDelivery> bolusHistory = insulinModel->getBolusHistory(start, end);
    for (const auto &bolus : bolusHistory) {
        result.append(qMakePair(bolus.timestamp, bolus.units));
    }
    
    // Get basal history (sampled at hourly intervals)
    QVector<InsulinModel::BasalDelivery> basalHistory = insulinModel->getBasalHistory(start, end);
    for (const auto &basal : basalHistory) {
        // Sample basal rate at hourly intervals
        QDateTime sampleTime = basal.startTime;
        while (sampleTime <= basal.endTime) {
            result.append(qMakePair(sampleTime, basal.rate));
            sampleTime = sampleTime.addSecs(60 * 60); // Add 1 hour
        }
    }
    
    // Sort by timestamp
    std::sort(result.begin(), result.end(), [](const auto &a, const auto &b) {
        return a.first < b.first;
    });
    
    return result;
}

bool PumpController::deliverBolus(double units, bool extended, int duration)
{
    if (!running) {
        return false;
    }
    
    // Check insulin remaining
    if (units > pumpModel->getInsulinRemaining()) {
        errorHandler->logError("Not enough insulin remaining for bolus", "InsulinModel", ErrorHandler::Warning);
        return false;
    }
    
    // Deliver bolus
    bool success = insulinModel->deliverBolus(units, "Manual", extended, duration);
    
    if (success) {
        // Reduce insulin reservoir
        pumpModel->reduceInsulin(units);
    }
    
    return success;
}

bool PumpController::cancelBolus()
{
    if (!running) {
        return false;
    }
    
    return insulinModel->cancelBolus();
}

bool PumpController::isBolusActive() const
{
    return insulinModel->isBolusActive();
}

bool PumpController::saveData(const QString &directory)
{
    QDir dir(directory);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    bool success = true;
    
    success &= pumpModel->saveState(directory + "/pump_state.json");
    success &= profileModel->saveProfiles(directory + "/profiles.json");
    success &= glucoseModel->saveReadings(directory + "/glucose_readings.json");
    success &= insulinModel->saveInsulinData(directory + "/insulin_data.json");
    
    return success;
}

bool PumpController::loadData(const QString &directory)
{
    bool success = true;
    
    if (QFile::exists(directory + "/pump_state.json")) {
        success &= pumpModel->loadState(directory + "/pump_state.json");
    }
    
    if (QFile::exists(directory + "/profiles.json")) {
        success &= profileModel->loadProfiles(directory + "/profiles.json");
    }
    
    if (QFile::exists(directory + "/glucose_readings.json")) {
        success &= glucoseModel->loadReadings(directory + "/glucose_readings.json");
    }
    
    if (QFile::exists(directory + "/insulin_data.json")) {
        success &= insulinModel->loadInsulinData(directory + "/insulin_data.json");
    }
    
    return success;
}

// Test panel methods implementation
void PumpController::updateBatteryLevel(int level)
{
    pumpModel->updateBatteryLevel(level);
}

void PumpController::updateInsulinRemaining(double units)
{
    pumpModel->updateInsulinRemaining(units);
}

void PumpController::updateGlucoseLevel(double value)
{
    // Add a new reading to the glucose model
    glucoseModel->addReading(value);
    
    // Update the current glucose display
    emit glucoseLevelChanged(value);
    
    // Update the graph data
    emit graphDataChanged(getGlucoseHistory(QDateTime::currentDateTime().addSecs(-3 * 60 * 60), 
                                           QDateTime::currentDateTime()));
}

void PumpController::updateGlucoseTrend(GlucoseModel::TrendDirection trend)
{
    // Force the trend direction
    glucoseModel->forceTrend(trend);
    
    // Emit signal for UI update
    emit glucoseTrendChanged(trend);
    
    // Update graph data
    emit graphDataChanged(getGlucoseHistory(QDateTime::currentDateTime().addSecs(-3 * 60 * 60), 
                                           QDateTime::currentDateTime()));
}

void PumpController::generateTestAlert(const QString &message, PumpModel::AlertLevel level)
{
    // Convert PumpModel::AlertLevel to ErrorHandler::ErrorLevel
    ErrorHandler::ErrorLevel errorLevel;
    switch (level) {
        case PumpModel::Info:
            errorLevel = ErrorHandler::Info;
            break;
        case PumpModel::Warning:
            errorLevel = ErrorHandler::Warning;
            break;
        case PumpModel::Critical:
            errorLevel = ErrorHandler::Critical;
            break;
        default:
            errorLevel = ErrorHandler::Warning;
    }
    
    // Log the error using error handler
    errorHandler->logError(message, "TestPanel", errorLevel);
    
    // Also add alert to the pump model
    pumpModel->addAlert(message, level);
}

void PumpController::simulateBatteryDrain()
{
    // Skip if charging
    if (pumpModel->isCharging()) {
        return;
    }
    
    // Drain battery
    int currentLevel = pumpModel->getBatteryLevel();
    if (currentLevel > 0) {
        pumpModel->updateBatteryLevel(currentLevel - 1);
    }
    
    // Check for low battery
    checkLowBattery();
}

void PumpController::processGlucoseReading(double value, const QDateTime &timestamp)
{
    // Update model
    pumpModel->addGlucoseReading(timestamp, value);
    
    // Update current glucose value
    emit glucoseLevelChanged(value);
    
    // Update graph data
    emit graphDataChanged(getGlucoseHistory(QDateTime::currentDateTime().addSecs(-6 * 60 * 60), QDateTime::currentDateTime()));
    
    // Check for alerts
    checkGlucoseAlerts();
}

void PumpController::updateInsulinOnBoard()
{
    if (!running) {
        return;
    }
    
    // Let the insulin model handle calculating IOB
    insulinModel->updateIOB();
}

void PumpController::updateBasalConsumption()
{
    if (!running) {
        return;
    }
    
    // Calculate basal rate for current 5-second period
    double basalRate = insulinModel->getCurrentBasalRate();
    double bolusRate = 0.0;
    
    // Check if a bolus is active
    if (insulinModel->isBolusActive()) {
        InsulinModel::BolusDelivery bolus = insulinModel->getCurrentBolus();
        
        // Calculate rate depending on bolus type
        if (bolus.extended) {
            // Extended bolus delivers evenly over the duration
            int durationSecs = bolus.duration * 60; // Convert minutes to seconds
            bolusRate = bolus.units / durationSecs; // Units per second
        } else {
            // Standard bolus delivers at fixed rate (e.g., 1u per minute)
            bolusRate = 1.0 / 60.0; // 1u per minute = 1/60 u per second
        }

        // Calculate insulin used in this 5-second period and actually use it
        double bolusUsed = bolusRate * 5.0; // for 5-second interval
        pumpModel->reduceInsulin(bolusUsed); // Actually use the calculated value
    }
    
    // Calculate basal insulin used in 5-second period
    double basalUsed = (basalRate / 3600.0) * 5.0; // Convert hourly rate to 5-second amount
    
    // Reduce insulin in reservoir
    pumpModel->reduceInsulin(basalUsed);
}

void PumpController::simulateGlucoseReading() {
    if (!running) {
        return;
    }
    
    // Get the current time
    QDateTime now = QDateTime::currentDateTime();
    
    // Fetch last 1 hour of glucose readings
    QVector<QPair<QDateTime, double>> recentReadings = 
        glucoseModel->getReadings(now.addSecs(-3600), now);
    
    // If we have no recent readings, generate one based on time of day
    if (recentReadings.isEmpty()) {
        // Get hour of day (0-23)
        int hour = now.time().hour();
        
        // Base value depending on time of day
        double baseValue = 5.5; // Default
        
        // Early morning high (dawn phenomenon)
        if (hour >= 3 && hour < 7) {
            baseValue = 7.0 + (QRandomGenerator::global()->generateDouble() - 0.5);
        } 
        // After breakfast rise
        else if (hour >= 7 && hour < 10) {
            baseValue = 8.5 + (QRandomGenerator::global()->generateDouble() - 0.5);
        }
        // Mid-day normal
        else if (hour >= 10 && hour < 12) {
            baseValue = 6.0 + (QRandomGenerator::global()->generateDouble() - 0.5);
        }
        // After lunch rise
        else if (hour >= 12 && hour < 15) {
            baseValue = 9.0 + (QRandomGenerator::global()->generateDouble() - 0.5);
        }
        // Afternoon
        else if (hour >= 15 && hour < 18) {
            baseValue = 5.5 + (QRandomGenerator::global()->generateDouble() - 0.5);
        }
        // After dinner rise
        else if (hour >= 18 && hour < 21) {
            baseValue = 8.0 + (QRandomGenerator::global()->generateDouble() - 0.5);
        }
        // Evening/night
        else {
            baseValue = 6.5 + (QRandomGenerator::global()->generateDouble() - 0.5);
        }
        
        // Add the reading
        glucoseModel->addReading(baseValue);
        return;
    }
    
    // Just advance along our pre-generated curve by taking a nearby reading 
    // and adding slight random variation
    double lastValue = recentReadings.last().second;
    double randomVariation = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.3;
    
    // Add the reading with small random change
    glucoseModel->addReading(lastValue + randomVariation);
}

void PumpController::runControlIQ() {
    if (!running || !controlIQEnabled) {
        return;
    }
    
    // Get current glucose and trend
    double currentGlucose = glucoseModel->getCurrentGlucose();
    GlucoseModel::TrendDirection trend = glucoseModel->getTrendDirection();
    
    // Get active profile
    Profile profile = profileModel->getActiveProfile();
    
    // Get a fixed basal adjustment based on current glucose and trend
    double basalAdjustment = controlIQAlgorithm->calculateBasalAdjustment(
        currentGlucose,
        trend,
        profile.basalRate,
        profile.targetGlucose,
        pumpModel->getInsulinOnBoard()
    );
    
    // If we have a non-zero adjustment, apply it
    if (qAbs(basalAdjustment) > 0.01) {
        double newBasalRate = qMax(0.0, profile.basalRate + basalAdjustment);
        
        // Apply the adjustment through insulin model
        insulinModel->adjustBasalRate(newBasalRate, true);
        
        // Update pump model state
        pumpModel->updateControlIQDelivery(basalAdjustment);
        
        // Emit signal for UI update
        emit controlIQActionChanged(basalAdjustment);
        
        // Log the action
        QString message;
        if (basalAdjustment > 0) {
            message = QString("Control-IQ increased basal rate to %1 u/hr").arg(newBasalRate, 0, 'f', 2);
            errorHandler->logError(message, "ControlIQ", ErrorHandler::Info);
        } else {
            message = QString("Control-IQ decreased basal rate to %1 u/hr").arg(newBasalRate, 0, 'f', 2);
            errorHandler->logError(message, "ControlIQ", ErrorHandler::Info);
        }
    }
    
    // Check for suspend at low glucose
    if (currentGlucose < 3.9) {
        insulinModel->suspendBasal();
        errorHandler->logError("Basal delivery suspended - Low glucose", "ControlIQ", ErrorHandler::Warning);
    } else if (insulinModel->getCurrentBasalRate() == 0.0 && currentGlucose >= 4.4) {
        // Resume basal if suspended and glucose is back up
        insulinModel->resumeBasal();
        errorHandler->logError("Basal delivery resumed", "ControlIQ", ErrorHandler::Info);
    }
}

void PumpController::checkReminders()
{
    if (!running) {
        return;
    }
    
    // Read reminders from settings
    QSettings settings("TandemDiabetes", "tslimx2simulator");
    settings.beginGroup("Alerts");
    
    int reminderCount = settings.beginReadArray("Reminders");
    for (int i = 0; i < reminderCount; ++i) {
        settings.setArrayIndex(i);
        QString type = settings.value("Type").toString();
        QDateTime time = settings.value("Time").toDateTime();
        bool acknowledged = settings.value("Acknowledged", false).toBool();
        
        // Check if the reminder is due and not acknowledged
        if (!acknowledged && time <= QDateTime::currentDateTime()) {
            // Trigger an alert using error handler
            errorHandler->logError("Reminder: " + type, "ReminderSystem", ErrorHandler::Warning);
            
            // Mark as acknowledged to prevent repeated alerts
            settings.setValue("Acknowledged", true);
        }
    }
    settings.endArray();
    settings.endGroup();
}

void PumpController::startSimulation()
{
    // Start timers
    batteryTimer->start();
    glucoseTimer->start();
    iobTimer->start();
    controlIQTimer->start();
    reminderTimer->start();
    occlusionTimer->start();
    basalConsumptionTimer->start();
    
    // Initial checks
    checkLowBattery();
    checkLowInsulin();
    
    // Run Control-IQ once immediately
    if (controlIQEnabled) {
        QTimer::singleShot(2000, this, &PumpController::runControlIQ);
    }
}

void PumpController::stopSimulation()
{
    // Stop timers
    batteryTimer->stop();
    glucoseTimer->stop();
    iobTimer->stop();
    controlIQTimer->stop();
    reminderTimer->stop();
    occlusionTimer->stop();
    basalConsumptionTimer->stop();
}

void PumpController::checkLowBattery()
{
    int level = pumpModel->getBatteryLevel();
    
    // Use the dedicated error handler method
    errorHandler->lowBatteryAlert(level);
    
    // Force shutdown if battery is extremely low
    if (level <= 1) {
        QTimer::singleShot(3000, this, [this]() {
            emit shutdownRequested();
        });
    }
}

void PumpController::checkLowInsulin()
{
    double insulin = pumpModel->getInsulinRemaining();
    
    // Use the dedicated error handler method
    errorHandler->lowInsulinAlert(insulin);
}

void PumpController::checkGlucoseAlerts()
{
    double glucose = glucoseModel->getCurrentGlucose();
    GlucoseModel::TrendDirection trend = glucoseModel->getTrendDirection();
    
    // Use the dedicated error handler methods
    errorHandler->lowGlucoseAlert(glucose);
    errorHandler->highGlucoseAlert(glucose);
    
    // Check for rapid changes in glucose
    if (trend == GlucoseModel::RisingQuickly) {
        errorHandler->logError("Glucose rising quickly", "GlucoseModel", ErrorHandler::Warning);
    } else if (trend == GlucoseModel::FallingQuickly) {
        errorHandler->logError("Glucose falling quickly", "GlucoseModel", ErrorHandler::Warning);
    }
    
    // Check CGM data gap (no readings for over 10 mins)
    QDateTime lastReadingTime = glucoseModel->getLastReadingTime();
    if (lastReadingTime.isValid() && lastReadingTime.secsTo(QDateTime::currentDateTime()) > 600) {
        int minutesSinceLastReading = lastReadingTime.secsTo(QDateTime::currentDateTime()) / 60;
        errorHandler->cgmDisconnectedAlert(minutesSinceLastReading);
    }
}

void PumpController::checkForOcclusion()
{
    // In a real implementation, this would check pressure sensors
    // For simulation, we'll randomly trigger occlusions with a very low probability
    if (running && QRandomGenerator::global()->bounded(1000) == 0) { // 1 in 1000 chance
        errorHandler->occlusionAlert();
        
        // Automatically suspend insulin delivery
        insulinModel->suspendBasal();
    }
}

void PumpController::savePumpState()
{
    // Create data directory if it doesn't exist
    QDir dataDir = QDir::home();
    dataDir.mkpath(".tslimx2simulator");
    
    // Save data
    saveData(QDir::homePath() + "/.tslimx2simulator");
}

void PumpController::loadPumpState()
{
    // Check if data directory exists
    QString dataPath = QDir::homePath() + "/.tslimx2simulator";
    if (QDir(dataPath).exists()) {
        loadData(dataPath);
    }
    //fix for battery reset
    pumpModel->updateBatteryLevel(100);

}

ErrorHandler* PumpController::getErrorHandler() const
{
    return errorHandler;
}

DataStorage* PumpController::getDataStorage() const
{
    return dataStorage;
}
