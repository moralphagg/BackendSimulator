#include "TerminalWidget.h"

#include <QVBoxLayout>
#include <QKeyEvent>
#include <QHBoxLayout>
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

    m_input->installEventFilter(this);

    m_input->setStyleSheet(
        "QLineEdit {"
        "  background: transparent;"
        "  border: none;"
        "  color: #FFFFFF;"
        "  caret-color: #00FF88;"
        "}"
        );

    QFont termFont("Courier New", 10);
    termFont.setStyleHint(QFont::Monospace);

    m_scroll->setFont(termFont);
    m_msgContainer->setFont(termFont);
    m_input->setFont(termFont);

    QWidget *inputRow = new QWidget(this);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputRow);
    inputLayout->setContentsMargins(4, 2, 4, 2);
    inputLayout->setSpacing(4);

    m_prompt = new QLabel("dev@backend-simulator:~$", this);
    m_prompt->setFont(termFont);
    m_prompt->setStyleSheet("color: #00FF88;");

    inputLayout->addWidget(m_prompt);
    inputLayout->addWidget(m_input, 1);

    layout->addWidget(m_scroll);
    layout->addWidget(inputRow);

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
    else if (type == "prompt")
        color = "#888888";

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

    addMessage("dev@backend-simulator:~$ " + cmd, "prompt");

    m_history.append(cmd);
    if (m_history.size() > 100)
        m_history.removeFirst();
    m_historyIndex = -1;

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

bool TerminalWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_input && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *key = static_cast<QKeyEvent*>(event);

        if (key->key() == Qt::Key_Up)
        {
            if (!m_history.isEmpty())
            {
                if (m_historyIndex == -1)
                    m_historyIndex = m_history.size() - 1;
                else if (m_historyIndex > 0)
                    m_historyIndex--;

                m_input->setText(m_history[m_historyIndex]);
            }
            return true;
        }
        else if (key->key() == Qt::Key_Down)
        {
            if (m_historyIndex != -1)
            {
                m_historyIndex++;
                if (m_historyIndex >= m_history.size())
                {
                    m_historyIndex = -1;
                    m_input->clear();
                }
                else
                {
                    m_input->setText(m_history[m_historyIndex]);
                }
            }
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}