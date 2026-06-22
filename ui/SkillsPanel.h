#pragma once

#include <QWidget>

class QListWidget;

class SkillsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SkillsPanel(QWidget *parent = nullptr);

    void clear();
    void addSkill(
        const QString &name,
        double value
        );

private:
    QListWidget *skillsList;
};