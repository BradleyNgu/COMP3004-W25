#include "profilescreen.h"
#include "ui_profilescreen.h"
#include <QMessageBox>
#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>

ProfileScreen::ProfileScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfileScreen),
    pumpController(nullptr),
    editMode(false)
{
    ui->setupUi(this);
    
    // Setup custom UI with layouts
    setupUi();
    
    // Initially hide edit form
    showEditForm(false);
}

ProfileScreen::~ProfileScreen()
{
    delete ui;
}

void ProfileScreen::setupUi()
{
    // Set dark background
    setStyleSheet("background-color: #222222;");
    
    // Main layout uses a stacked widget to switch between profile list and edit form
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setAlignment(Qt::AlignRight);

    homeButton = new QPushButton("T");
    homeButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #00B2FF; font-size: 20px; font-weight: bold; border: none; }"
        "QPushButton:pressed { color: #0077CC; }"
    );
    homeButton->setFixedSize(40, 40);

    topLayout->addWidget(homeButton);
    mainLayout->addLayout(topLayout);
    
    // Title
    titleLabel = new QLabel("Personal Profiles");
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Stacked widget for switching between list view and edit form
    stackedWidget = new QStackedWidget();
    mainLayout->addWidget(stackedWidget);
    
    // *** PROFILE LIST VIEW ***
    QWidget *profileListWidget = new QWidget();
    QVBoxLayout *profileListLayout = new QVBoxLayout(profileListWidget);
    profileListLayout->setContentsMargins(0, 8, 0, 8);
    profileListLayout->setSpacing(12);
    
    // Profile list
    profilesList = new QListWidget();
    profilesList->setStyleSheet(
        "QListWidget { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 5px; }"
        "QListWidget::item { padding: 8px; }"
        "QListWidget::item:selected { background-color: #00B2FF; }"
    );
    profilesList->setMinimumHeight(180);
    profileListLayout->addWidget(profilesList);
    
    // Button toolbar for list operations
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);
    
    newProfileButton = new QPushButton("New");
    newProfileButton->setStyleSheet(
        "QPushButton { background-color: #4CD964; color: white; border-radius: 5px; padding: 8px; }"
        "QPushButton:pressed { background-color: #3CB371; }"
    );
    
    editProfileButton = new QPushButton("Edit");
    editProfileButton->setStyleSheet(
        "QPushButton { background-color: #007AFF; color: white; border-radius: 5px; padding: 8px; }"
        "QPushButton:pressed { background-color: #0055CC; }"
    );
    
    deleteProfileButton = new QPushButton("Delete");
    deleteProfileButton->setStyleSheet(
        "QPushButton { background-color: #FF3B30; color: white; border-radius: 5px; padding: 8px; }"
        "QPushButton:pressed { background-color: #CC2F27; }"
    );
    
    buttonLayout->addWidget(newProfileButton);
    buttonLayout->addWidget(editProfileButton);
    buttonLayout->addWidget(deleteProfileButton);
    profileListLayout->addLayout(buttonLayout);
    
    // Activate button
    activateProfileButton = new QPushButton("Activate Profile");
    activateProfileButton->setStyleSheet(
        "QPushButton { background-color: #00B2FF; color: white; border-radius: 5px; font-weight: bold; padding: 10px; }"
        "QPushButton:pressed { background-color: #0080FF; }"
    );
    profileListLayout->addWidget(activateProfileButton);
    
    // Add list view to stacked widget
    stackedWidget->addWidget(profileListWidget);
    
    // *** PROFILE EDIT FORM ***
    // Create a scroll area for the edit form to handle smaller screens
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *editFormWidget = new QWidget();
    QVBoxLayout *editFormLayout = new QVBoxLayout(editFormWidget);
    editFormLayout->setContentsMargins(8, 8, 8, 8);
    editFormLayout->setSpacing(16);
    
    // Profile edit form group
    QGroupBox *editGroupBox = new QGroupBox("Profile Settings");
    editGroupBox->setStyleSheet("QGroupBox { color: white; font-weight: bold; }");
    QVBoxLayout *groupLayout = new QVBoxLayout(editGroupBox);
    
    // Form layout for profile fields
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    formLayout->setSpacing(12);
    
    // Profile name
    QLabel *nameLabel = new QLabel("Profile Name:");
    nameLabel->setStyleSheet("color: white;");
    
    profileNameEdit = new QLineEdit();
    profileNameEdit->setStyleSheet(
        "QLineEdit { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 6px; }"
    );
    formLayout->addRow(nameLabel, profileNameEdit);
    
    // Basal rate
    QLabel *basalLabel = new QLabel("Basal Rate:");
    basalLabel->setStyleSheet("color: white;");
    
    basalRateSpinBox = new QDoubleSpinBox();
    basalRateSpinBox->setRange(0.1, 5.0);
    basalRateSpinBox->setSingleStep(0.1);
    basalRateSpinBox->setValue(1.0);
    basalRateSpinBox->setSuffix(" u/hr");
    basalRateSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 6px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 20px; }"
    );
    formLayout->addRow(basalLabel, basalRateSpinBox);
    
    // Carb ratio
    QLabel *carbLabel = new QLabel("Carb Ratio:");
    carbLabel->setStyleSheet("color: white;");
    
    carbRatioSpinBox = new QDoubleSpinBox();
    carbRatioSpinBox->setRange(1.0, 50.0);
    carbRatioSpinBox->setSingleStep(0.5);
    carbRatioSpinBox->setValue(10.0);
    carbRatioSpinBox->setSuffix(" g/u");
    carbRatioSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 6px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 20px; }"
    );
    formLayout->addRow(carbLabel, carbRatioSpinBox);
    
    // Correction factor
    QLabel *correctionLabel = new QLabel("Correction Factor:");
    correctionLabel->setStyleSheet("color: white;");
    
    correctionFactorSpinBox = new QDoubleSpinBox();
    correctionFactorSpinBox->setRange(0.1, 10.0);
    correctionFactorSpinBox->setSingleStep(0.1);
    correctionFactorSpinBox->setValue(2.0);
    correctionFactorSpinBox->setSuffix(" mmol/L/u");
    correctionFactorSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 6px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 20px; }"
    );
    formLayout->addRow(correctionLabel, correctionFactorSpinBox);
    
    // Target glucose
    QLabel *targetLabel = new QLabel("Target Glucose:");
    targetLabel->setStyleSheet("color: white;");
    
    targetGlucoseSpinBox = new QDoubleSpinBox();
    targetGlucoseSpinBox->setRange(3.0, 10.0);
    targetGlucoseSpinBox->setSingleStep(0.1);
    targetGlucoseSpinBox->setValue(5.5);
    targetGlucoseSpinBox->setSuffix(" mmol/L");
    targetGlucoseSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 6px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 20px; }"
    );
    formLayout->addRow(targetLabel, targetGlucoseSpinBox);
    
    // Add form to group
    groupLayout->addLayout(formLayout);
    
    // Add group to edit form layout
    editFormLayout->addWidget(editGroupBox);
    
    // Form action buttons
    QHBoxLayout *actionButtonLayout = new QHBoxLayout();
    
    saveProfileButton = new QPushButton("Save");
    saveProfileButton->setStyleSheet(
        "QPushButton { background-color: #4CD964; color: white; border-radius: 5px; padding: 8px 16px; }"
        "QPushButton:pressed { background-color: #3CB371; }"
    );
    
    cancelEditButton = new QPushButton("Cancel");
    cancelEditButton->setStyleSheet(
        "QPushButton { background-color: #FF3B30; color: white; border-radius: 5px; padding: 8px 16px; }"
        "QPushButton:pressed { background-color: #CC2F27; }"
    );
    
    actionButtonLayout->addWidget(saveProfileButton);
    actionButtonLayout->addWidget(cancelEditButton);
    editFormLayout->addLayout(actionButtonLayout);
    
    // Add extra space at bottom
    editFormLayout->addStretch(1);
    
    // Set the widget for scroll area
    scrollArea->setWidget(editFormWidget);
    
    // Add edit form to stacked widget
    stackedWidget->addWidget(scrollArea);
    
    // *** BACK BUTTON (ALWAYS SHOWN) ***
    QHBoxLayout *backButtonLayout = new QHBoxLayout();
    backButtonLayout->addStretch();
    
    backButton = new QPushButton("Back");
    backButton->setStyleSheet(
        "QPushButton { background-color: #444444; color: white; border-radius: 5px; padding: 8px; }"
        "QPushButton:pressed { background-color: #666666; }"
    );
    backButton->setFixedWidth(80);
    
    backButtonLayout->addWidget(backButton);
    mainLayout->addLayout(backButtonLayout);
    
    // Connect signals
    connect(backButton, &QPushButton::clicked, this, &ProfileScreen::on_backButton_clicked);
    connect(newProfileButton, &QPushButton::clicked, this, &ProfileScreen::on_newProfileButton_clicked);
    connect(editProfileButton, &QPushButton::clicked, this, &ProfileScreen::on_editProfileButton_clicked);
    connect(deleteProfileButton, &QPushButton::clicked, this, &ProfileScreen::on_deleteProfileButton_clicked);
    connect(activateProfileButton, &QPushButton::clicked, this, &ProfileScreen::on_activateProfileButton_clicked);
    connect(profilesList, &QListWidget::currentRowChanged, this, &ProfileScreen::on_profilesList_currentRowChanged);
    connect(saveProfileButton, &QPushButton::clicked, this, &ProfileScreen::on_saveProfileButton_clicked);
    connect(cancelEditButton, &QPushButton::clicked, this, &ProfileScreen::on_cancelEditButton_clicked);
    connect(homeButton, &QPushButton::clicked, this, &ProfileScreen::on_homeButton_clicked);
}

void ProfileScreen::showEditForm(bool show)
{
    // Use the stacked widget to switch between views
    stackedWidget->setCurrentIndex(show ? 1 : 0);
    
    // Adjust title
    titleLabel->setText(show ? "Edit Profile" : "Personal Profiles");
}

void ProfileScreen::loadProfiles(PumpController *controller)
{
    this->pumpController = controller;
    
    // Get all profiles
    profiles = controller->getAllProfiles();
    
    // Update list widget
    updateProfilesList();
    
    // Update button states based on selection
    on_profilesList_currentRowChanged(profilesList->currentRow());
}

void ProfileScreen::updateProfilesList()
{
    // Store currently selected profile name
    QString selectedProfileName;
    if (profilesList->currentRow() >= 0 && profilesList->currentRow() < profiles.size()) {
        selectedProfileName = profiles[profilesList->currentRow()].name;
    }
    
    // Clear list
    profilesList->clear();
    
    // Add profiles to list
    int selectedIndex = -1;
    QString activeProfileName = pumpController ? pumpController->getActiveProfileName() : "";
    
    for (int i = 0; i < profiles.size(); ++i) {
        QString displayName = profiles[i].name;
        
        // Mark active profile
        if (profiles[i].name == activeProfileName) {
            displayName += " (Active)";
        }
        
        profilesList->addItem(displayName);
        
        // Check if this was previously selected
        if (profiles[i].name == selectedProfileName) {
            selectedIndex = i;
        }
    }
    
    // Restore selection if possible
    if (selectedIndex >= 0) {
        profilesList->setCurrentRow(selectedIndex);
    } else if (profilesList->count() > 0) {
        profilesList->setCurrentRow(0);
    }
}

void ProfileScreen::populateEditForm(const Profile &profile)
{
    profileNameEdit->setText(profile.name);
    basalRateSpinBox->setValue(profile.basalRate);
    carbRatioSpinBox->setValue(profile.carbRatio);
    correctionFactorSpinBox->setValue(profile.correctionFactor);
    targetGlucoseSpinBox->setValue(profile.targetGlucose);
    
    // Disable name editing for default profile
    profileNameEdit->setEnabled(profile.name != "Default");
}

Profile ProfileScreen::getProfileFromForm()
{
    Profile profile;
    profile.name = profileNameEdit->text().trimmed();
    profile.basalRate = basalRateSpinBox->value();
    profile.carbRatio = carbRatioSpinBox->value();
    profile.correctionFactor = correctionFactorSpinBox->value();
    profile.targetGlucose = targetGlucoseSpinBox->value();
    
    return profile;
}

bool ProfileScreen::validateProfileForm()
{
    // Check profile name
    QString name = profileNameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Invalid Profile", "Profile name cannot be empty.");
        return false;
    }
    
    // Check for duplicate names (except the profile being edited)
    if (!editMode || (editMode && name != editingProfileName)) {
        for (const auto &profile : profiles) {
            if (profile.name == name) {
                QMessageBox::warning(this, "Invalid Profile", "A profile with this name already exists.");
                return false;
            }
        }
    }
    
    // Check basal rate
    if (basalRateSpinBox->value() <= 0.0) {
        QMessageBox::warning(this, "Invalid Profile", "Basal rate must be greater than 0.");
        return false;
    }
    
    // Check carb ratio
    if (carbRatioSpinBox->value() <= 0.0) {
        QMessageBox::warning(this, "Invalid Profile", "Carb ratio must be greater than 0.");
        return false;
    }
    
    // Check correction factor
    if (correctionFactorSpinBox->value() <= 0.0) {
        QMessageBox::warning(this, "Invalid Profile", "Correction factor must be greater than 0.");
        return false;
    }
    
    // Check target glucose
    if (targetGlucoseSpinBox->value() < 3.0 || targetGlucoseSpinBox->value() > 10.0) {
        QMessageBox::warning(this, "Invalid Profile", "Target glucose must be between 3.0 and 10.0 mmol/L.");
        return false;
    }
    
    return true;
}

void ProfileScreen::on_backButton_clicked()
{
    emit backButtonClicked();
}

void ProfileScreen::on_newProfileButton_clicked()
{
    // Setup form for new profile
    editMode = false;
    editingProfileName = "";
    
    // Set default values
    Profile defaultProfile;
    defaultProfile.name = "";
    defaultProfile.basalRate = 1.0;
    defaultProfile.carbRatio = 10.0;
    defaultProfile.correctionFactor = 2.0;
    defaultProfile.targetGlucose = 5.5;
    
    populateEditForm(defaultProfile);
    showEditForm(true);
}

void ProfileScreen::on_editProfileButton_clicked()
{
    int currentRow = profilesList->currentRow();
    if (currentRow < 0 || currentRow >= profiles.size()) {
        return;
    }
    
    // Setup form for editing
    editMode = true;
    editingProfileName = profiles[currentRow].name;
    
    populateEditForm(profiles[currentRow]);
    showEditForm(true);
}

void ProfileScreen::on_deleteProfileButton_clicked()
{
    int currentRow = profilesList->currentRow();
    if (currentRow < 0 || currentRow >= profiles.size()) {
        return;
    }
    
    QString profileName = profiles[currentRow].name;
    
    // Cannot delete Default profile
    if (profileName == "Default") {
        QMessageBox::warning(this, "Cannot Delete", "The Default profile cannot be deleted.");
        return;
    }
    
    // Check if this is the active profile
    if (profileName == pumpController->getActiveProfileName()) {
        QMessageBox::warning(this, "Cannot Delete", "Cannot delete the active profile. Please activate another profile first.");
        return;
    }
    
    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Confirm Deletion", 
        QString("Are you sure you want to delete the profile '%1'?").arg(profileName),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        emit profileDeleted(profileName);
        
        // Reload profiles
        loadProfiles(pumpController);
    }
}

void ProfileScreen::on_activateProfileButton_clicked()
{
    int currentRow = profilesList->currentRow();
    if (currentRow < 0 || currentRow >= profiles.size()) {
        return;
    }
    
    QString profileName = profiles[currentRow].name;
    
    // Check if already active
    if (profileName == pumpController->getActiveProfileName()) {
        return;
    }
    
    // Confirm activation
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Activate Profile", 
        QString("Activate profile '%1'? This will change your insulin delivery settings.").arg(profileName),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        emit profileActivated(profileName);
        
        // Update UI
        updateProfilesList();
    }
}

void ProfileScreen::on_profilesList_currentRowChanged(int currentRow)
{
    bool validSelection = (currentRow >= 0 && currentRow < profiles.size());
    
    // Update button states
    editProfileButton->setEnabled(validSelection);
    deleteProfileButton->setEnabled(validSelection);
    activateProfileButton->setEnabled(validSelection);
    
    // Disable delete button for Default profile
    if (validSelection && profiles[currentRow].name == "Default") {
        deleteProfileButton->setEnabled(false);
    }
    
    // Disable activate button if profile is already active
    if (validSelection && pumpController && profiles[currentRow].name == pumpController->getActiveProfileName()) {
        activateProfileButton->setEnabled(false);
    }
}

void ProfileScreen::on_saveProfileButton_clicked()
{
    if (!validateProfileForm()) {
        return;
    }
    
    Profile profile = getProfileFromForm();
    
    if (editMode) {
        emit profileUpdated(editingProfileName, profile);
    } else {
        emit profileCreated(profile);
    }
    
    // Return to profile list
    showEditForm(false);
    
    // Reload profiles
    loadProfiles(pumpController);
}

void ProfileScreen::on_cancelEditButton_clicked()
{
    // Return to profile list without saving
    showEditForm(false);
}

// Paint event for custom styling
void ProfileScreen::paintEvent(QPaintEvent * /* event */)
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
void ProfileScreen::on_homeButton_clicked()
{
    emit homeButtonClicked();
}
