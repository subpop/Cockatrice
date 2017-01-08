#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QSet>
#include <QWizard>
#include "gamecreator.h"
#include "tab_room.h"
#include "settingscache.h"

#include "pending_command.h"
#include "pb/serverinfo_game.pb.h"

void GameCreator::sharedCtor() {
    rememberGameSettings = new QCheckBox(tr("Re&member"));
    descriptionLabel = new QLabel(tr("&Description:"));
    descriptionEdit = new QLineEdit;
    descriptionLabel->setBuddy(descriptionEdit);
    descriptionEdit->setMaxLength(60);

    maxPlayersLabel = new QLabel(tr("P&layers:"));
    maxPlayersEdit = new QSpinBox();
    maxPlayersEdit->setMinimum(1);
    maxPlayersEdit->setMaximum(100);
    maxPlayersEdit->setValue(2);
    maxPlayersLabel->setBuddy(maxPlayersEdit);

    QGridLayout *generalGrid = new QGridLayout;
    generalGrid->addWidget(descriptionLabel, 0, 0);
    generalGrid->addWidget(descriptionEdit, 0, 1);
    generalGrid->addWidget(maxPlayersLabel, 1, 0);
    generalGrid->addWidget(maxPlayersEdit, 1, 1);
    generalGrid->addWidget(rememberGameSettings, 2, 0);

    QGroupBox *generalGroupBox = new QGroupBox(tr("General"));
    generalGroupBox->setLayout(generalGrid);

    QGridLayout *gameTypeLayout = new QGridLayout;
    QMapIterator<int, QString> gameTypeIterator(gameTypes);

    // split the games into 2 columns to save vertical space
    // first find out half of the types
    int half = 0;
    if (gameTypes.size() % 2 == 1) {
        half = (gameTypes.size() + 1) / 2;
    } else {
        half = gameTypes.size() / 2;
    }

    int yPos = 0;
    int xPos = 0;
    while (gameTypeIterator.hasNext()) {
        gameTypeIterator.next();
        QRadioButton *gameTypeRadioButton = new QRadioButton(gameTypeIterator.value(), this);
        gameTypeLayout->addWidget(gameTypeRadioButton, yPos++, xPos);
        gameTypeCheckBoxes.insert(gameTypeIterator.key(), gameTypeRadioButton);
        bool isChecked = settingsCache->getGameTypes().contains(gameTypeIterator.value() + ", ");
        gameTypeCheckBoxes[gameTypeIterator.key()]->setChecked(isChecked);

        // start a new column when we have placed half
        if (yPos == half) {
            xPos += 1;
            yPos = 0;
        }
    }
    QGroupBox *gameTypeGroupBox = new QGroupBox(tr("Game type"));
    gameTypeGroupBox->setLayout(gameTypeLayout);

    passwordLabel = new QLabel(tr("&Password:"));
    passwordEdit = new QLineEdit;
    passwordLabel->setBuddy(passwordEdit);

    onlyBuddiesCheckBox = new QCheckBox(tr("Only &buddies"));
    onlyRegisteredCheckBox = new QCheckBox(tr("Only &registered users"));
    if (room && room->getUserInfo()->user_level() & ServerInfo_User::IsRegistered) {
        onlyRegisteredCheckBox->setChecked(true);
    } else {
        onlyBuddiesCheckBox->setEnabled(false);
        onlyRegisteredCheckBox->setEnabled(false);
    }

    QGridLayout *joinRestrictionsLayout = new QGridLayout;
    joinRestrictionsLayout->addWidget(passwordLabel, 0, 0);
    joinRestrictionsLayout->addWidget(passwordEdit, 0, 1);
    joinRestrictionsLayout->addWidget(onlyBuddiesCheckBox, 1, 0, 1, 2);
    joinRestrictionsLayout->addWidget(onlyRegisteredCheckBox, 2, 0, 1, 2);

    QGroupBox *joinRestrictionsGroupBox = new QGroupBox(tr("Joining restrictions"));
    joinRestrictionsGroupBox->setLayout(joinRestrictionsLayout);

    spectatorsAllowedCheckBox = new QCheckBox(tr("&Can watch"));
    spectatorsAllowedCheckBox->setChecked(true);
    connect(spectatorsAllowedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(spectatorsAllowedChanged(int)));
    spectatorsNeedPasswordCheckBox = new QCheckBox(tr("Require password"));
    spectatorsCanTalkCheckBox = new QCheckBox(tr("Can &chat"));
    spectatorsSeeEverythingCheckBox = new QCheckBox(tr("Can see &hands"));
    QVBoxLayout *spectatorsLayout = new QVBoxLayout;
    spectatorsLayout->addWidget(spectatorsAllowedCheckBox);
    spectatorsLayout->addWidget(spectatorsNeedPasswordCheckBox);
    spectatorsLayout->addWidget(spectatorsCanTalkCheckBox);
    spectatorsLayout->addWidget(spectatorsSeeEverythingCheckBox);
    spectatorsGroupBox = new QGroupBox(tr("Spectators"));
    spectatorsGroupBox->setLayout(spectatorsLayout);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(generalGroupBox, 0, 0);
    grid->addWidget(spectatorsGroupBox, 1, 0);
    grid->addWidget(joinRestrictionsGroupBox, 0, 1);
    grid->addWidget(gameTypeGroupBox, 1, 1);

    createButton = new QPushButton(tr("&Create"));
    connect(createButton, SIGNAL(clicked()), this, SLOT(actOK()));
    clearButton = new QPushButton(tr("&Reset"));
    connect(clearButton, SIGNAL(clicked()), this, SLOT(actReset()));
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(createButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    setFixedHeight(sizeHint().height());
}

GameCreator::GameCreator(TabRoom *_room, const QMap<int, QString> &_gameTypes, QWidget *parent)
        : QGroupBox(parent), room(_room), gameTypes(_gameTypes) {
    sharedCtor();

    rememberGameSettings->setChecked(settingsCache->getRememberGameSettings());
    descriptionEdit->setText(settingsCache->getGameDescription());
    maxPlayersEdit->setValue(settingsCache->getMaxPlayers());
    if (room && room->getUserInfo()->user_level() & ServerInfo_User::IsRegistered) {
        onlyBuddiesCheckBox->setChecked(settingsCache->getOnlyBuddies());
        onlyRegisteredCheckBox->setChecked(settingsCache->getOnlyRegistered());
    } else {
        onlyBuddiesCheckBox->setEnabled(false);
        onlyRegisteredCheckBox->setEnabled(false);
    }
    spectatorsAllowedCheckBox->setChecked(settingsCache->getSpectatorsAllowed());
    spectatorsNeedPasswordCheckBox->setChecked(settingsCache->getSpectatorsNeedPassword());
    spectatorsCanTalkCheckBox->setChecked(settingsCache->getSpectatorsCanTalk());
    spectatorsSeeEverythingCheckBox->setChecked(settingsCache->getSpectatorsCanSeeEverything());

    if (!rememberGameSettings->isChecked()) {
        actReset();
    }
}

GameCreator::GameCreator(const ServerInfo_Game &gameInfo, const QMap<int, QString> &_gameTypes, QWidget *parent)
        : QGroupBox(parent), room(0), gameTypes(_gameTypes) {
    sharedCtor();

    rememberGameSettings->setEnabled(false);
    descriptionEdit->setEnabled(false);
    maxPlayersEdit->setEnabled(false);
    passwordEdit->setEnabled(false);
    onlyBuddiesCheckBox->setEnabled(false);
    onlyRegisteredCheckBox->setEnabled(false);
    spectatorsAllowedCheckBox->setEnabled(false);
    spectatorsNeedPasswordCheckBox->setEnabled(false);
    spectatorsCanTalkCheckBox->setEnabled(false);
    spectatorsSeeEverythingCheckBox->setEnabled(false);

    descriptionEdit->setText(QString::fromStdString(gameInfo.description()));
    maxPlayersEdit->setValue(gameInfo.max_players());
    onlyBuddiesCheckBox->setChecked(gameInfo.only_buddies());
    onlyRegisteredCheckBox->setChecked(gameInfo.only_registered());
    spectatorsAllowedCheckBox->setChecked(gameInfo.spectators_allowed());
    spectatorsNeedPasswordCheckBox->setChecked(gameInfo.spectators_need_password());
    spectatorsCanTalkCheckBox->setChecked(gameInfo.spectators_can_chat());
    spectatorsSeeEverythingCheckBox->setChecked(gameInfo.spectators_omniscient());

    QSet<int> types;
    for (int i = 0; i < gameInfo.game_types_size(); ++i)
        types.insert(gameInfo.game_types(i));

    QMapIterator<int, QString> gameTypeIterator(gameTypes);
    while (gameTypeIterator.hasNext()) {
        gameTypeIterator.next();

        QRadioButton *gameTypeCheckBox = gameTypeCheckBoxes.value(gameTypeIterator.key());
        gameTypeCheckBox->setEnabled(false);
        gameTypeCheckBox->setChecked(types.contains(gameTypeIterator.key()));
    }
}

void GameCreator::actReset() {
    descriptionEdit->setText("");
    maxPlayersEdit->setValue(2);

    passwordEdit->setText("");
    onlyBuddiesCheckBox->setChecked(false);
    onlyRegisteredCheckBox->setChecked(room && room->getUserInfo()->user_level() & ServerInfo_User::IsRegistered);

    spectatorsAllowedCheckBox->setChecked(true);
    spectatorsNeedPasswordCheckBox->setChecked(false);
    spectatorsCanTalkCheckBox->setChecked(false);
    spectatorsSeeEverythingCheckBox->setChecked(false);

    QMapIterator<int, QRadioButton *> gameTypeCheckBoxIterator(gameTypeCheckBoxes);
    while (gameTypeCheckBoxIterator.hasNext()) {
        gameTypeCheckBoxIterator.next();
        // must set auto enclusive to false to be able to set the check to false
        gameTypeCheckBoxIterator.value()->setAutoExclusive(false);
        gameTypeCheckBoxIterator.value()->setChecked(false);
        gameTypeCheckBoxIterator.value()->setAutoExclusive(true);
    }

    descriptionEdit->setFocus();
}


void GameCreator::actOK() {
    Command_CreateGame cmd;
    cmd.set_description(descriptionEdit->text().simplified().toStdString());
    cmd.set_password(passwordEdit->text().toStdString());
    cmd.set_max_players(maxPlayersEdit->value());
    cmd.set_only_buddies(onlyBuddiesCheckBox->isChecked());
    cmd.set_only_registered(onlyRegisteredCheckBox->isChecked());
    cmd.set_spectators_allowed(spectatorsAllowedCheckBox->isChecked());
    cmd.set_spectators_need_password(spectatorsNeedPasswordCheckBox->isChecked());
    cmd.set_spectators_can_talk(spectatorsCanTalkCheckBox->isChecked());
    cmd.set_spectators_see_everything(spectatorsSeeEverythingCheckBox->isChecked());

    QString gameTypes = QString();
    QMapIterator<int, QRadioButton *> gameTypeCheckBoxIterator(gameTypeCheckBoxes);
    while (gameTypeCheckBoxIterator.hasNext()) {
        gameTypeCheckBoxIterator.next();
        if (gameTypeCheckBoxIterator.value()->isChecked()) {
            cmd.add_game_type_ids(gameTypeCheckBoxIterator.key());
            gameTypes += gameTypeCheckBoxIterator.value()->text() + ", ";
        }
    }

    settingsCache->setRememberGameSettings(rememberGameSettings->isChecked());
    if (rememberGameSettings->isChecked()) {
        settingsCache->setGameDescription(descriptionEdit->text());
        settingsCache->setMaxPlayers(maxPlayersEdit->value());
        settingsCache->setOnlyBuddies(onlyBuddiesCheckBox->isChecked());
        settingsCache->setOnlyRegistered(onlyRegisteredCheckBox->isChecked());
        settingsCache->setSpectatorsAllowed(spectatorsAllowedCheckBox->isChecked());
        settingsCache->setSpectatorsNeedPassword(spectatorsNeedPasswordCheckBox->isChecked());
        settingsCache->setSpectatorsCanTalk(spectatorsCanTalkCheckBox->isChecked());
        settingsCache->setSpectatorsCanSeeEverything(spectatorsSeeEverythingCheckBox->isChecked());
        settingsCache->setGameTypes(gameTypes);
    }
    PendingCommand *pend = room->prepareRoomCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(checkResponse(Response)));
    room->sendRoomCommand(pend);
}

void GameCreator::retranslateUi() {
    setTitle(tr("Create Game"));
    createButton->setText(tr("Create"));
    clearButton->setText(tr("Reset"));
}


void GameCreator::spectatorsAllowedChanged(int state) {
    spectatorsNeedPasswordCheckBox->setEnabled(state);
    spectatorsCanTalkCheckBox->setEnabled(state);
    spectatorsSeeEverythingCheckBox->setEnabled(state);
}

