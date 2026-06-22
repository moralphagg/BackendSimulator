#include "ProjectsPanel.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>

ProjectsPanel::ProjectsPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Активные проекты"));

    activeProjects = new QListWidget(this);
    layout->addWidget(activeProjects);

    layout->addWidget(new QLabel("Очередь проектов"));

    queuedProjects = new QListWidget(this);
    layout->addWidget(queuedProjects);
}

void ProjectsPanel::clear()
{
    activeProjects->clear();
    queuedProjects->clear();
}

void ProjectsPanel::addActiveProject(
    const QString &name,
    int difficulty,
    int reward,
    int progress,
    int bugs)
{
    activeProjects->addItem(
        QString("%1\nГотов на %2% | Багов: %3\nСложность: %4 | $%5")
            .arg(name)
            .arg(progress)
            .arg(bugs)
            .arg(difficulty)
            .arg(reward)
        );
}

void ProjectsPanel::addQueuedProject(
    const QString &name,
    int difficulty,
    int reward
    )
{
    queuedProjects->addItem(
        QString("%1\n"
                "Сложность: %2\n"
                "Награда: %3")
            .arg(name)
            .arg(difficulty)
            .arg(reward)
        );
}