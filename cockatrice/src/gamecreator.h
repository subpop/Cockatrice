#ifndef COCKATRICE_GAMECREATOR_H
#define COCKATRICE_GAMECREATOR_H

#include <QGroupBox>
#include <QMap>

class QLabel;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QRadioButton;
class QGroupBox;
class QSpinBox;
class QButtonBox;
class TabRoom;

class Response;
class ServerInfo_Game;

class GameCreator : public QGroupBox {
    Q_OBJECT
public:
    GameCreator(TabRoom *_room, const QMap<int, QString> &_gameTypes, QWidget *parent=0);
    GameCreator(const ServerInfo_Game &game, const QMap<int, QString> &_gameTypes, QWidget *parent=0);
    void retranslateUi();
private slots:
    void actOK();
    void actReset();
    void spectatorsAllowedChanged(int state);
private:
    TabRoom *room;
    QMap<int, QString> gameTypes;
    QMap<int, QRadioButton *> gameTypeCheckBoxes;

    QGroupBox *spectatorsGroupBox;
    QLabel *descriptionLabel, *passwordLabel, *maxPlayersLabel;
    QLineEdit *descriptionEdit, *passwordEdit;
    QSpinBox *maxPlayersEdit;
    QCheckBox *onlyBuddiesCheckBox, *onlyRegisteredCheckBox;
    QCheckBox *spectatorsAllowedCheckBox, *spectatorsNeedPasswordCheckBox, *spectatorsCanTalkCheckBox, *spectatorsSeeEverythingCheckBox;
    QPushButton *clearButton, *createButton;
    QCheckBox *rememberGameSettings;

    void sharedCtor();
};

#endif //COCKATRICE_GAMECREATOR_H
