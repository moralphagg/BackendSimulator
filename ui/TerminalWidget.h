#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>

class TerminalWidget : public QWidget {
    Q_OBJECT
public:
    explicit TerminalWidget(QWidget *parent = nullptr);
    void addMessage(const QString &text, const QString &type);
    void clear();

signals:
    void commandEntered(const QString &cmd);

private slots:
    void onReturnPressed();

private:
    QScrollArea  *m_scroll;
    QWidget      *m_msgContainer;
    QVBoxLayout  *m_msgLayout;
    QLineEdit    *m_input;

    void appendLabel(const QString &text, const QString &colorHex);
    void scrollToBottom();

};