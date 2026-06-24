#include "StatusPanel.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>

StatusPanel::StatusPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    levelLabel             = new QLabel("Уровень: 0");
    xpLabel                = new QLabel("XP: 0");
    energyLabel            = new QLabel("Энергия: 0");
    moneyLabel             = new QLabel("Деньги: 0");
    reputationLabel        = new QLabel("Репутация: 0");
    projectsCompletedLabel = new QLabel("Проектов: 0");
    bugsFixedLabel         = new QLabel("Багов: 0");
    activeProjectsLabel    = new QLabel("Активных: 0/1");
    queueLabel             = new QLabel("Очередь: 0/5");
    jobLabel               = new QLabel("Должность: Фрилансер");
    companyLabel           = new QLabel("Компания: —");
    burnoutBar             = new QProgressBar();

    burnoutBar->setRange(0, 100);
    burnoutBar->setValue(0);
    burnoutBar->setTextVisible(false);
    burnoutBar->setFixedHeight(6);
    burnoutBar->setStyleSheet(
        "QProgressBar { background: #333; border-radius: 3px; }"
        "QProgressBar::chunk { background: #f78166; border-radius: 3px; }"
        );

    timeLabel =
        new QLabel(
            "День 1 | 08:00"
            );

    layout->addWidget(timeLabel);
    layout->addWidget(levelLabel);
    layout->addWidget(xpLabel);
    layout->addWidget(energyLabel);
    layout->addWidget(moneyLabel);
    layout->addWidget(reputationLabel);
    layout->addWidget(projectsCompletedLabel);
    layout->addWidget(bugsFixedLabel);
    layout->addWidget(activeProjectsLabel);
    layout->addWidget(queueLabel);
    layout->addStretch();
    layout->addWidget(jobLabel);
    layout->addWidget(companyLabel);
    layout->addWidget(new QLabel("Выгорание:"));
    layout->addWidget(burnoutBar);
}

void StatusPanel::setLevel(int value, int maxLevel) {
    levelLabel->setText(QString("Уровень: %1/%2").arg(value).arg(maxLevel));
}

void StatusPanel::setXp(int value, int xpNeeded) {
    xpLabel->setText(QString("XP: %1/%2").arg(value).arg(xpNeeded));
}

void StatusPanel::setEnergy(int value, int maxValue) {
    energyLabel->setText(QString("Энергия: %1/%2").arg(value).arg(maxValue));
}

void StatusPanel::setMoney(int value)
{
    moneyLabel->setText(QString("Деньги: %1").arg(value));
}

void StatusPanel::setReputation(int value)
{
    reputationLabel->setText(QString("Репутация: %1").arg(value));
}

void StatusPanel::setProjectsCompleted(int value)
{
    projectsCompletedLabel->setText(
        QString("Проектов: %1").arg(value)
        );
}

void StatusPanel::setBugsFixed(int value)
{
    bugsFixedLabel->setText(
        QString("Багов: %1").arg(value)
        );
}
void StatusPanel::setGameTime(
    int day,
    int hour,
    int minute
    )
{
    timeLabel->setText(
        QString(
            "День %1 | %2:%3"
            )
            .arg(day)
            .arg(hour, 2, 10, QChar('0'))
            .arg(minute, 2, 10, QChar('0'))
        );
}

void StatusPanel::setActiveProjects(int current, int max) {
    activeProjectsLabel->setText(QString("Активных: %1/%2").arg(current).arg(max));
}

void StatusPanel::setQueue(int current, int max) {
    queueLabel->setText(QString("Очередь: %1/%2").arg(current).arg(max));
}

void StatusPanel::setJob(const QString &title, const QString &company)
{
    jobLabel->setText("Должность: " + title);
    companyLabel->setText("Компания: " + company);
}

void StatusPanel::setBurnout(int value, int max)
{
    burnoutBar->setMaximum(max);
    burnoutBar->setValue(value);

    QString color;
    if (value < 40)
        color = "#3fb950";
    else if (value < 70)
        color = "#d29922";
    else
        color = "#f78166";

    burnoutBar->setStyleSheet(QString(
                                  "QProgressBar { background: #333; border-radius: 3px; }"
                                  "QProgressBar::chunk { background: %1; border-radius: 3px; }"
                                  ).arg(color));
}