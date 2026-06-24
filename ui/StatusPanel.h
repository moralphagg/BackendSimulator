#pragma once

#include <QWidget>
#include <QProgressBar>

class QLabel;

class StatusPanel : public QWidget
{
    Q_OBJECT

public:
    explicit StatusPanel(QWidget *parent = nullptr);

    void setLevel(int value, int maxLevel);
    void setXp(int value, int xpNeeded);
    void setEnergy(int value, int maxValue);
    void setMoney(int value);
    void setReputation(int value);
    void setProjectsCompleted(int value);
    void setBugsFixed(int value);
    void setActiveProjects(int current, int max);
    void setQueue(int current, int max);
    void setGameTime(
        int day,
        int hour,
        int minute
        );
    void setJob(const QString &title, const QString &company);
    void setBurnout(int value, int max);

private:
    QLabel *levelLabel;
    QLabel *xpLabel;
    QLabel *energyLabel;
    QLabel *moneyLabel;
    QLabel *reputationLabel;
    QLabel *projectsCompletedLabel;
    QLabel *bugsFixedLabel;
    QLabel *activeProjectsLabel;
    QLabel *queueLabel;
    QLabel *timeLabel;
    QLabel *jobLabel;
    QLabel *companyLabel;
    QProgressBar *burnoutBar;
};