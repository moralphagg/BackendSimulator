#pragma once

#include <QMainWindow>
#include <QCloseEvent>

class GameController;
class TerminalWidget;

class QTextEdit;
class QLineEdit;
class QListWidget;
class QLabel;
class QPushButton;
class QDockWidget;

class StatusPanel;

class SkillsPanel;

class ProjectsPanel;

class CommandsPanel;

class HelpPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onCommandEntered(const QString& cmd);
    void addMessage(const QString& text, const QString& type);
    void updateUi();
    void unlockGui();

private:
    GameController* m_controller;

    StatusPanel *statusPanel;
    TerminalWidget* terminalWidget;
    ProjectsPanel *projectsPanel;
    SkillsPanel *skillsPanel;
    CommandsPanel *commandsPanel;
    HelpPanel *helpPanel;

    void buildUi();
    void applyDarkTheme();
    void connectSignals();
};

