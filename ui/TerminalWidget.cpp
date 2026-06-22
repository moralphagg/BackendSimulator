#include "TerminalWidget.h"

#include <QVBoxLayout>
#include <QScrollBar>

TerminalWidget::TerminalWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);

    m_msgContainer = new QWidget;
    m_msgLayout = new QVBoxLayout(m_msgContainer);

    m_msgLayout->setAlignment(Qt::AlignTop);

    m_scroll->setWidget(m_msgContainer);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText("Введите команду...");

    layout->addWidget(m_scroll);
    layout->addWidget(m_input);

    connect(
        m_input,
        &QLineEdit::returnPressed,
        this,
        &TerminalWidget::onReturnPressed
        );

    connect(
        m_scroll->verticalScrollBar(),
        &QScrollBar::rangeChanged,
        this,
        [this](int, int max){ m_scroll->verticalScrollBar()->setValue(max); }
        );
}

void TerminalWidget::addMessage(
    const QString &text,
    const QString &type)
{
    QString color = "#FFFFFF";

    if (type == "success")
        color = "#00FF88";
    else if (type == "error")
        color = "#FF5555";
    else if (type == "warning")
        color = "#FFCC00";
    else if (type == "highlight")
        color = "#55AAFF";

    appendLabel(text, color);
}

void TerminalWidget::clear()
{
    QLayoutItem *item;

    while ((item = m_msgLayout->takeAt(0)))
    {
        delete item->widget();
        delete item;
    }
}

void TerminalWidget::onReturnPressed()
{
    QString cmd = m_input->text().trimmed();

    if (cmd.isEmpty())
        return;

    emit commandEntered(cmd);

    m_input->clear();
}

void TerminalWidget::appendLabel(
    const QString &text,
    const QString &colorHex)
{
    QLabel *label = new QLabel(text);

    label->setWordWrap(true);

    label->setStyleSheet(
        QString("color:%1;").arg(colorHex)
        );

    m_msgLayout->addWidget(label);

}

void TerminalWidget::scrollToBottom()
{
    QScrollBar *bar = m_scroll->verticalScrollBar();

    bar->setValue(bar->maximum());
}