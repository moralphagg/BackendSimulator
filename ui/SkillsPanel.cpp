#include "SkillsPanel.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>

SkillsPanel::SkillsPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    layout->addWidget(
        new QLabel("Навыки")
        );

    skillsList = new QListWidget(this);

    layout->addWidget(skillsList);
}

void SkillsPanel::clear()
{
    skillsList->clear();
}

void SkillsPanel::addSkill(
    const QString &name,
    double value
    )
{
    skillsList->addItem(
        QString("%1 : %2")
            .arg(name)
            .arg(value, 0, 'f', 1)
        );
}