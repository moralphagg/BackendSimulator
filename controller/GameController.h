#pragma once
#include <QObject>
#include <QTimer>
#include "../core/GameState.h"

class GameController : public QObject {
    Q_OBJECT
public:
    explicit GameController(QObject *parent = nullptr);

    void executeCommand(const QString &cmd);

    const GameState &state() const { return m_state; }

signals:
    void messageAdded(const QString &text, const QString &type);
    void stateChanged();
    void levelUp(int newLevel);
    void guiUnlocked();

private slots:
    void onGameTick();

private:
    GameState m_state;

    QTimer* m_gameTimer;

    void postCommand(const QString &cmd);

    void advanceTime(int minutes);

    void checkPromotion();

    QPair<QString,QString> cmdStart();
    QPair<QString,QString> cmdWork();
    QPair<QString,QString> cmdBugs();
    QPair<QString,QString> cmdLearn();
    QPair<QString,QString> cmdRest();
    QPair<QString,QString> cmdDeploy();
    QPair<QString,QString> cmdOptimize();
    QPair<QString,QString> cmdMeeting();
    QPair<QString,QString> cmdResearch();
    QPair<QString,QString> cmdMentor();
    QPair<QString,QString> cmdRefactor();
    QPair<QString,QString> cmdDocument();
    QPair<QString,QString> cmdAnalyze();
    QPair<QString,QString> cmdScale();
    QPair<QString,QString> cmdMigrate();
    QPair<QString,QString> cmdAudit();
    QPair<QString,QString> cmdSave();
    QPair<QString,QString> cmdLoad();
    QPair<QString,QString> cmdFreelance();

    void checkLevelUp();
    void updateProjectQueue();
    void handleRandomEvent();
    void updateBurnout(int delta);

    void emit_msg(const QString &text, const QString &type = "text");
};