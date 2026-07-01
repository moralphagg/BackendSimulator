#include "MainWindow.h"

#include "../controller/GameController.h"

#include "StatusPanel.h"
#include "SkillsPanel.h"
#include "ProjectsPanel.h"
#include "TerminalWidget.h"
#include "CommandsPanel.h"
#include "HelpPanel.h"
#include "../core/SaveManager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_controller(new GameController(this))
{
    buildUi();
    connectSignals();

    terminalWidget->addMessage(
        "Запуск симулятора бэкенд-разработчика!",
        "highlight"
        );

    terminalWidget->addMessage(
        "Введите команду...",
        "text"
        );

    setWindowTitle("Backend Developer Simulator");
    resize(1600, 900);

    updateUi();
}

MainWindow::~MainWindow()
{
}

void MainWindow::buildUi()
{
    QWidget *central = new QWidget(this);

    auto *layout = new QHBoxLayout(central);

    statusPanel = new StatusPanel(this);
    projectsPanel = new ProjectsPanel(this);
    skillsPanel = new SkillsPanel(this);
    terminalWidget = new TerminalWidget(this);
    commandsPanel = new CommandsPanel(this);
    helpPanel = new HelpPanel(this);

    commandsPanel->hide();

    layout->addWidget(statusPanel);
    layout->addWidget(projectsPanel);
    layout->addWidget(skillsPanel);
    layout->addWidget(commandsPanel);
    layout->addWidget(helpPanel);
    layout->addWidget(terminalWidget, 1);

    setCentralWidget(central);

    applyDarkTheme();
}

void MainWindow::connectSignals()
{
    connect(
        terminalWidget,
        &TerminalWidget::commandEntered,
        this,
        &MainWindow::onCommandEntered
        );

    connect(
        m_controller,
        &GameController::messageAdded,
        this,
        &MainWindow::addMessage
        );

    connect(
        m_controller,
        &GameController::stateChanged,
        this,
        &MainWindow::updateUi
        );

    connect(
        m_controller,
        &GameController::guiUnlocked,
        this,
        &MainWindow::unlockGui
        );
    connect(
        commandsPanel,
        &CommandsPanel::commandClicked,
        this,
        &MainWindow::onCommandEntered
        );

    m_controller->sendWelcome();
}

void MainWindow::onCommandEntered(const QString& cmd)
{
    m_controller->executeCommand(cmd);
}

void MainWindow::addMessage(const QString& text, const QString& type)
{
    if (text == "__clear__")
    {
        terminalWidget->clear();
        return;
    }

    terminalWidget->addMessage(text, type);
}

void MainWindow::updateUi()
{
    const auto &state = m_controller->state();

    statusPanel->setLevel(state.level, state.maxLevel);
    statusPanel->setXp(state.xp, state.level * 100);
    statusPanel->setEnergy(state.energy, state.maxEnergy);
    statusPanel->setMoney(state.money);
    statusPanel->setReputation(state.reputation);
    statusPanel->setProjectsCompleted(
        state.projectsCompleted
        );

    statusPanel->setBugsFixed(
        state.bugsFixed
        );

    statusPanel->setActiveProjects(
        state.currentProjects.size(),
        state.maxConcurrentProjects
        );
    statusPanel->setQueue(
        state.projectQueue.size(),
        state.maxQueueSize
        );

    statusPanel->setGameTime(
        state.gameDay,
        state.gameHours,
        state.gameMinutes
        );

    statusPanel->setJob(
        state.currentJob.displayName,
        state.currentCompany.name.isEmpty() ? "—" : state.currentCompany.name
        );
    statusPanel->setBurnout(state.burnout, state.maxBurnout);

    statusPanel->setMood(state.mood, state.maxMood);
    statusPanel->setStress(state.stress, state.maxStress);
    statusPanel->setTechDebt(state.techDebt);

    projectsPanel->clear();

    for (const auto &project : state.currentProjects)
    {
        projectsPanel->addActiveProject(
            project.name,
            project.difficulty,
            project.reward,
            project.progress,
            project.bugs
            );
    }

    for (const auto &project : state.projectQueue)
    {
        projectsPanel->addQueuedProject(
            project.name,
            project.difficulty,
            project.reward
            );
    }

    skillsPanel->clear();

    for (auto it = state.skills.begin();
         it != state.skills.end();
         ++it)
    {
        skillsPanel->addSkill(
            it.key(),
            it.value()
            );
    }

    if (state.guiUnlocked)
    {
        commandsPanel->show();
    }
    else
    {
        commandsPanel->hide();
    }
}

void MainWindow::unlockGui() {
    terminalWidget->addMessage("GUI разблокирован!", "highlight");
}

void MainWindow::applyDarkTheme()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    SaveManager::save(
        m_controller->state()
        );

    QMainWindow::closeEvent(event);
}