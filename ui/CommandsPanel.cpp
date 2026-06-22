#include "CommandsPanel.h"

#include <QVBoxLayout>
#include <QPushButton>

CommandsPanel::CommandsPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    QStringList commands =
        {
            "start",
            "work",
            "bugs",
            "learn",
            "rest",
            "deploy",
            "optimize",
            "meeting",
            "research",
            "mentor",
            "refactor",
            "document",
            "analyze",
            "scale",
            "migrate",
            "audit",
            "freelance",
            "save",
            "load"
        };

    for (const QString &cmd : commands)
    {
        auto *button = new QPushButton(cmd, this);

        connect(
            button,
            &QPushButton::clicked,
            this,
            [this, cmd]()
            {
                emit commandClicked(cmd);
            });

        layout->addWidget(button);
    }

    layout->addStretch();
}