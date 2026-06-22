#pragma once

#include <QWidget>

class QListWidget;

class ProjectsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectsPanel(QWidget *parent = nullptr);

    void clear();
    void addActiveProject(const QString &name, int difficulty, int reward, int progress, int bugs);

    void addQueuedProject(const QString &name, int difficulty, int reward);

private:
    QListWidget *activeProjects;
    QListWidget *queuedProjects;
};