#pragma once

#include <QWidget>

class QPushButton;

class CommandsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CommandsPanel(QWidget *parent = nullptr);

signals:
    void commandClicked(const QString &cmd);
};