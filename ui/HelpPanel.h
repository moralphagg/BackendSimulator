#pragma once

#include <QWidget>

class QTextEdit;

class HelpPanel : public QWidget
{
    Q_OBJECT

public:
    explicit HelpPanel(QWidget *parent = nullptr);

private:
    QTextEdit *helpText;
};