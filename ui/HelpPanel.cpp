#include "HelpPanel.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>

HelpPanel::HelpPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    auto *title =
        new QLabel("Команды", this);

    layout->addWidget(title);

    helpText =
        new QTextEdit(this);

    helpText->setReadOnly(true);

    helpText->setText(
        "start\n"
        "Взять проект\n\n"

        "work\n"
        "Работать над проектом\n\n"

        "bugs\n"
        "Исправить баги\n\n"

        "deploy\n"
        "Завершить проект\n\n"

        "learn\n"
        "Обучение\n\n"

        "research\n"
        "Изучение технологий\n\n"

        "optimize\n"
        "Оптимизация\n\n"

        "refactor\n"
        "Рефакторинг\n\n"

        "audit\n"
        "Аудит безопасности\n\n"

        "meeting\n"
        "Митинг\n\n"

        "mentor\n"
        "Менторство\n\n"

        "freelance\n"
        "Фриланс\n\n"

        "rest\n"
        "Отдых"
        );

    layout->addWidget(helpText);

    setMinimumWidth(250);
    setMaximumWidth(300);
}