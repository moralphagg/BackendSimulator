#include "GameController.h"
#include "../core/SkillSystem.h"
#include "../core/ProjectQueue.h"
#include "../core/CheatEngine.h"
#include "../core/SaveManager.h"
#include <QRandomGenerator>
#include <QDateTime>
#include "../core/CareerData.h"

GameController::GameController(QObject *parent)
    : QObject(parent)
{
    if (!SaveManager::load(m_state))
    {
        m_state.initSkills();
        ProjectQueue::refill(m_state);
    }

    if (!SaveManager::load(m_state))
    {
        m_isNewGame = true;
        m_state.initSkills();
        ProjectQueue::refill(m_state);
    }

    m_state.currentJob     = CareerData::jobInfo(JobTitle::Freelancer);
    m_state.currentCompany = CareerData::companyInfo(CompanyType::None);
    m_state.lifeStage   = LifeStage::Student;
    m_state.housingType = HousingType::Dorm;
    m_state.rentCost    = 3000;
    m_state.foodCost    = 50;
    m_state.equipment   = CareerData::equipmentInfo(EquipmentTier::OldLaptop);
    m_state.maxEnergy   = m_state.equipment.maxEnergy;

    m_state.currentJob     = CareerData::jobInfo(JobTitle::Unemployed);
    m_state.currentCompany = CareerData::companyInfo(CompanyType::None);

    m_state.parentAllowance = 2000;
    m_state.transportCost   = 40;

    m_gameTimer = new QTimer(this);

    connect(
        m_gameTimer,
        &QTimer::timeout,
        this,
        &GameController::onGameTick
        );

    m_gameTimer->start(1000);
}

void GameController::sendWelcome()
{
    if (!m_isNewGame) return;

    emit_msg("Добро пожаловать! Ты студент 1-го курса.", "highlight");
    emit_msg("Живёшь в общаге. Есть стипендия 2500/мес.", "text");
    emit_msg("Цель: окончи универ и найди первую работу.", "text");
}

void GameController::advanceTime(int minutes)
{
    m_state.gameMinutes += minutes;

    while (m_state.gameMinutes >= 60)
    {
        m_state.gameMinutes -= 60;
        m_state.gameHours++;

        checkTimeEvents();

        if (m_state.gameHours >= 24)
        {
            m_state.gameHours = 0;
            m_state.gameDay++;
            onNewDay();
        }
    }
}


void GameController::emit_msg(const QString &text, const QString &type) {
    emit messageAdded(text, type);
}

void GameController::executeCommand(
    const QString &rawCmd)
{
    QString cmd =
        rawCmd.trimmed().toLower();

    bool wasGuiUnlocked =
        m_state.guiUnlocked;

    QString cheat =
        CheatEngine::check(
            m_state,
            cmd
            );

    if (!cheat.isEmpty())
    {
        emit_msg(
            cheat,
            "highlight"
            );

        SaveManager::save(m_state);

        if (!wasGuiUnlocked &&
            m_state.guiUnlocked)
        {
            emit guiUnlocked();
        }

        emit stateChanged();
        return;
    }

    postCommand(cmd);
}

void GameController::postCommand(const QString &cmd) {
    QPair<QString,QString> result = {"", "text"};

    if      (cmd == "start")          result = cmdStart();
    else if (cmd == "work")           result = cmdWork();
    else if (cmd == "bugs")           result = cmdBugs();
    else if (cmd == "learn")          result = cmdLearn();
    else if (cmd == "rest")           result = cmdRest();
    else if (cmd == "sleep")          result = cmdSleep();
    else if (cmd.startsWith("buy "))  result = cmdBuy(cmd.mid(4).trimmed());
    else if (cmd == "laundry")        result = cmdLaundry();
    else if (cmd == "walk")           result = cmdWalk();
    else if (cmd == "clean")          result = cmdClean();
    else if (cmd == "deploy")         result = cmdDeploy();
    else if (cmd == "optimize")       result = cmdOptimize();
    else if (cmd == "meeting")        result = cmdMeeting();
    else if (cmd == "research")       result = cmdResearch();
    else if (cmd == "mentor")         result = cmdMentor();
    else if (cmd == "homework")       result = cmdHomework("");
    else if (cmd == "homework skip")  result = cmdHomework("skip");
    else if (cmd == "homework order") result = cmdHomework("order");
    else if (cmd == "coursework")     result = cmdCoursework();
    else if (cmd == "tutor")          result = cmdTutor();
    else if (cmd == "parents")        result = cmdParents();
    else if (cmd == "grant")          result = cmdGrant();
    else if (cmd == "freelance")      result = cmdFreelance();
    else if (cmd == "refactor")       result = cmdRefactor();
    else if (cmd == "document")       result = cmdDocument();
    else if (cmd == "analyze")        result = cmdAnalyze();
    else if (cmd == "scale")          result = cmdScale();
    else if (cmd == "migrate")        result = cmdMigrate();
    else if (cmd == "audit")          result = cmdAudit();
    else if (cmd == "schedule")       result = cmdSchedule();
    else if (cmd == "status")         result = cmdStatus();
    else if (cmd == "submit")
    {
        if (!m_state.pendingLab)
        {
            emit_msg("Нет активной лабы для сдачи.", "warning");
            return;
        }
        if (m_state.energy < 20)
        {
            emit_msg("Слишком устал чтобы сдать лабу. Отдохни.", "warning");
            return;
        }
        m_state.energy -= 20;
        m_state.labsCompleted++;
        m_state.pendingLab = false;
        m_state.studyScore = qMin(100, m_state.studyScore + 5);
        emit_msg(
            QString("Сдал лабу по \"%1\"! +5 успеваемости. "
                    "Сдано лаб: %2/%3")
                .arg(m_state.pendingLabSubject)
                .arg(m_state.labsCompleted)
                .arg(m_state.labsRequired),
            "success"
            );
    }
    else if (cmd == "review fix")     result = cmdReview("fix");
    else if (cmd == "review skip")    result = cmdReview("skip");
    else if (cmd == "jobs")              result = cmdJobs();
    else if (cmd == "quit")              result = cmdQuit();
    else if (cmd.startsWith("apply "))
    {
        bool ok;
        int idx = cmd.mid(6).trimmed().toInt(&ok);
        result  = ok ? cmdApply(idx)
                    : qMakePair(QString("Укажи номер: apply 1"), QString("warning"));
    }
    else if (cmd == "clear")          { emit_msg("__clear__"); return; }
    else if (cmd == "save")           result = cmdSave();
    else if (cmd == "load")           result = cmdLoad();
    else if (cmd.isEmpty())      return;
    else result = {"Неизвестная команда: " + cmd, "error"};

    if (!result.first.isEmpty())
        emit_msg(result.first, result.second);

    checkLevelUp();
    checkPromotion();
    updateProjectQueue();
    handleRandomEvent();

    if (m_state.pendingIncident)
    {
        if (cmd == "fix")
        {
            m_state.pendingIncident = false;

            int energyCost = 40;
            int timeSpent  = QRandomGenerator::global()->bounded(30, 91);

            m_state.energy    = qMax(0, m_state.energy - energyCost);
            m_state.reputation = qMin(200, m_state.reputation + 5);
            updateBurnout(+8);
            advanceTime(timeSpent);

            m_state.skills["debugging"] =
                qMin(10.0, m_state.skills["debugging"] + 0.1);

            emit_msg(
                QString("Починил прод за %1 мин. "
                        "+5 репутации, -40 энергии, debugging +0.1")
                    .arg(timeSpent),
                "success"
                );
        }
        else if (cmd == "ignore")
        {
            m_state.pendingIncident = false;
            m_state.prodIgnored++;

            int repLoss = 10 + m_state.prodIgnored * 5;
            m_state.reputation = qMax(0, m_state.reputation - repLoss);

            emit_msg(
                QString("Проигнорировал инцидент. "
                        "-%1 репутации. Игнорировано всего: %2")
                    .arg(repLoss)
                    .arg(m_state.prodIgnored),
                "error"
                );

            if (m_state.prodIgnored >= 3)
                emit_msg(
                    "Ты часто игноришь инциденты. "
                    "Это скажется на карьере.",
                    "error"
                    );
        }
        else
        {
            emit_msg(
                "Прод горит! Сначала введи fix или ignore.",
                "error"
                );
        }

        SaveManager::save(m_state);
        emit stateChanged();
        return;
    }
}

QPair<QString,QString> GameController::cmdStart() {
    if (m_state.currentProjects.size() >= m_state.maxConcurrentProjects)
        return {QString("Максимум %1 активных проектов!").arg(m_state.maxConcurrentProjects), "warning"};

    if (m_state.projectQueue.isEmpty())
        return {"Нет доступных проектов!", "warning"};

    if (m_state.energy < 25)
        return {"Слишком устал!", "warning"};

    Project p = m_state.projectQueue.dequeue();

    p.deadlineDay = m_state.gameDay + p.deadlineDay;
    m_state.currentProjects.enqueue(p);

    m_state.energy -= 25;

    advanceTime(30);
    updateBurnout(-1);

    return {
        QString("Начал проект: %1 (-25 энергии)\nДедлайн: день %2 (осталось %3 дн.)")
            .arg(p.name)
            .arg(p.deadlineDay)
            .arg(p.deadlineDay - m_state.gameDay),
        "success"
    };
}

QPair<QString,QString> GameController::cmdWork()
{
    if (m_state.currentProjects.isEmpty())
        return {"Нет активного проекта", "warning"};

    if (m_state.energy < 15)
        return {"Слишком устал для работы. Отдохни.", "warning"};

    int energyCost = 20;
    bool isLateNight = (m_state.gameHours >= 22 || m_state.gameHours < 4);
    if (isLateNight)
    {
        energyCost = 30;
        emit_msg("Работаешь ночью — энергия тратится быстрее.", "warning");
    }

    Project &p = m_state.currentProjects.head();

    double burnoutPenalty = 1.0 - (m_state.burnout / 150.0);

    double moodBonus = 0.6 + (m_state.mood / 250.0);

    int stressExtraBugs = 0;
    if (m_state.stress >= 80)      stressExtraBugs = 3;
    else if (m_state.stress >= 50) stressExtraBugs = 1;

    double sleepPenalty = 1.0 - (m_state.sleepDebt * 0.05);
    sleepPenalty = qMax(0.5, sleepPenalty);

    double speedBonus = m_state.currentJob.workSpeedBonus;

    double equipBonus  = m_state.equipment.speedBonus;

    double skillBonus = 1.0;
    if (!p.category.isEmpty() && m_state.skills.contains(p.category))
    {
        double skillLevel = m_state.skills[p.category];
        skillBonus = 1.0 + (skillLevel - 1.0) * 0.1;
    }

    double bugReduction = m_state.equipment.bugReduction;

    int progressGain = QRandomGenerator::global()->bounded(5, 15);
    progressGain = qMax(1, int(progressGain
                               * speedBonus
                               * burnoutPenalty
                               * equipBonus
                               * skillBonus
                               * sleepPenalty
                               * moodBonus) - p.difficulty);
    p.progress = qMin(100, p.progress + progressGain);
    m_state.energy -= energyCost;
    advanceTime(120);
    updateBurnout(+5);

    int bugChance = 25;
    if (m_state.burnout >= 70) bugChance = 55;
    else if (m_state.burnout >= 40) bugChance = 35;
    bugChance = qMax(5, int(bugChance * (1.0 - bugReduction)));

    if (QRandomGenerator::global()->bounded(100) < bugChance)
        p.bugs += QRandomGenerator::global()->bounded(1, 4);

    if (stressExtraBugs > 0 &&
        QRandomGenerator::global()->bounded(100) < 40)
    {
        p.bugs += stressExtraBugs;
        emit_msg(
            QString("Из-за стресса допустил дополнительных багов: +%1")
                .arg(stressExtraBugs),
            "warning"
            );
    }
    if (p.bugs > 5)
        updateTechDebt(+2);

    if (m_state.techDebt >= 50)
    {
        emit_msg(
            QString("Задеплоил с высоким техдолгом (%1/100). "
                    ).arg(m_state.techDebt),
            "warning"
            );
        updateStress(+10);
        updateTechDebt(+5);
    }

    return {
        QString("%1\nПрогресс: %2% (+%3) | Багов: %4%5%6")
            .arg(p.name)
            .arg(p.progress)
            .arg(progressGain)
            .arg(p.bugs)
            .arg(skillBonus > 1.05
                     ? QString(" | Навык x%1").arg(skillBonus, 0, 'f', 2)
                     : QString())
            .arg(p.deadlineDay > 0
                     ? QString(" | Дедлайн: день %1 (осталось %2 дн.)")
                           .arg(p.deadlineDay)
                           .arg(p.deadlineDay - m_state.gameDay)
                     : QString()),
        "success"
    };
}

QPair<QString,QString> GameController::cmdBugs()
{
    if (m_state.energy < 15)
        return {
            "Слишком устал!", "warning"};

    if (m_state.currentProjects.isEmpty())
        return {
            "Нет активных проектов!", "warning"};

    Project &project =
        m_state.currentProjects.head();

    if (project.bugs <= 0)
        return {"В проекте нет багов!", "warning"};

    int fixed =
        qMin(
            project.bugs,
            QRandomGenerator::global()->bounded(1, 4)
            );

    project.bugs -= fixed;

    int reward =
        int(
            fixed * 50 *
            SkillSystem::reputationBonus(m_state)
            );

    int xp = fixed * 5;

    m_state.bugsFixed += fixed;
    m_state.money += reward;
    m_state.xp += xp;
    m_state.energy -= 15;

    advanceTime(60);
    updateBurnout(+10);

    m_state.skills["debugging"] +=
        0.1 *
        SkillSystem::skillBonusMultiplier(m_state);

    return {
        QString(
            "Исправлено %1 багов. Осталось: %2. +%3 денег, +%4 опыта"
            )
            .arg(fixed)
            .arg(project.bugs)
            .arg(reward)
            .arg(xp),
        "success"
    };
}

QPair<QString,QString> GameController::cmdLearn() {
    int cost = SkillSystem::learnCost(m_state);
    if (m_state.money  < cost)  return {QString("Недостаточно денег! (%1)").arg(cost), "warning"};
    if (m_state.energy < 20)    return {"Слишком устал!", "warning"};
    QString info = SkillSystem::improveRandom(m_state, m_state.skills.keys(), 0.2, 0.5);
    m_state.money  -= cost;
    m_state.energy -= 20;
    advanceTime(180);
    updateBurnout(+10);
    return {QString("Прокачал %1 (-%2 денег)").arg(info).arg(cost), "success"};
}

QPair<QString,QString> GameController::cmdRest()
{
    int gain = QRandomGenerator::global()->bounded(10, 21);
    m_state.energy = qMin(m_state.maxEnergy, m_state.energy + gain);
    advanceTime(30);
    updateBurnout(-2);
    updateMood(+5);
    updateStress(-3);

    return {
        QString("Сделал перерыв: +%1 энергии. "
                "Для сна используй sleep.")
            .arg(gain),
        "success"
    };
}

QPair<QString,QString> GameController::cmdSleep()
{
    bool canSleep = (m_state.gameHours >= 22 || m_state.gameHours < 7);
    if (!canSleep)
    {
        return {
            QString("Сейчас %1:%2. Ложиться спать можно с 22:00.")
                .arg(m_state.gameHours, 2, 10, QChar('0'))
                .arg(m_state.gameMinutes, 2, 10, QChar('0')),
            "warning"
        };
    }

    if (m_state.sleptTonight)
    {
        return {
            "Ты уже спал этой ночью. "
            "Хватит валяться — займись делом.",
            "warning"
        };
    }

    int hoursUntilMorning;
    if (m_state.gameHours >= 22)
        hoursUntilMorning = 24 - m_state.gameHours + 7;
    else
        hoursUntilMorning = 7 - m_state.gameHours;

    int sleepHours = qMin(hoursUntilMorning,
                          QRandomGenerator::global()->bounded(7, 9));

    int energyGain = int(m_state.maxEnergy * 0.6)
                     + QRandomGenerator::global()->bounded(0, 21);
    m_state.energy = qMin(m_state.maxEnergy, m_state.energy + energyGain);

    int burnoutGain = -(sleepHours * 3);
    updateBurnout(burnoutGain);

    if (sleepHours < 7)
    {
        int debt = 7 - sleepHours;
        m_state.sleepDebt += debt;
        emit_msg(
            QString("Поспал только %1 ч. Недосып накапливается (%2 ч. долга).")
                .arg(sleepHours)
                .arg(m_state.sleepDebt),
            "warning"
            );
    }
    else if (m_state.sleepDebt > 0)
    {
        m_state.sleepDebt = qMax(0, m_state.sleepDebt - 1);
        emit_msg("Хорошо выспался. Долг сна снижается.", "success");
    }

    m_state.sleptTonight  = true;
    m_state.lastSleepDay  = m_state.gameDay;
    m_state.lastRestDay   = m_state.gameDay;
    m_state.daysWithoutRest = 0;

    advanceTime(sleepHours * 60);

    int moodGain   = sleepHours >= 7 ? 15 : 5;
    int stressLoss = sleepHours >= 7 ? 10 : 3;
    updateMood(+moodGain);
    updateStress(-stressLoss);

    return {
        QString("Поспал %1 ч. +%2 энергии, выгорание %3.")
            .arg(sleepHours)
            .arg(energyGain)
            .arg(burnoutGain),
        "success"
    };
}

QPair<QString,QString> GameController::cmdDeploy() {

    if (m_state.currentProjects.isEmpty())
    {
        return {
            "Нет активных проектов!",
            "warning"
        };
    }

    Project &p =
        m_state.currentProjects.head();

    QString projectName =
        p.name;

    if (p.progress < 100)
    {
        return {
            QString(
                "Проект готов только на %1%"
                )
                .arg(p.progress),
            "warning"
        };
    }

    if (p.bugs > 0)
    {
        return {
            QString(
                "Осталось исправить %1 багов"
                )
                .arg(p.bugs),
            "warning"
        };
    }

    double repBonus =
        SkillSystem::reputationBonus(m_state);

    int reward =
        int(p.reward * repBonus);

    m_state.money += reward;
    m_state.xp += p.difficulty * 10;
    m_state.reputation += p.difficulty;

    m_state.categoryReputation[p.category]
        += p.difficulty;

    m_state.projectsCompleted++;

    if (p.deadlineDay > 0 && m_state.gameDay <= p.deadlineDay)
    {
        m_state.deadlinesMet++;
        int bonus = int(p.reward * 0.2);
        m_state.money += bonus;
        emit_msg(
            QString("Сдано в срок! Бонус +%1 денег").arg(bonus),
            "success"
            );
    }

    if (p.category == "web")
    {
        m_state.skills["api_design"] += 0.3;
    }

    else if (p.category == "database")
    {
        m_state.skills["sql"] += 0.3;
    }

    else if (p.category == "microservice")
    {
        m_state.skills["microservices"] += 0.3;
    }

    else if (p.category == "devops")
    {
        m_state.skills["docker"] += 0.3;
        m_state.skills["ci_cd"] += 0.3;
    }

    else if (p.category == "cloud")
    {
        m_state.skills["aws"] += 0.3;
        m_state.skills["serverless"] += 0.2;
    }

    else if (p.category == "ai")
    {
        m_state.skills["ml"] += 0.4;
    }

    else if (p.category == "security")
    {
        m_state.skills["security"] += 0.4;
    }

    else if (p.category == "distributed")
    {
        m_state.skills["kafka"] += 0.2;
        m_state.skills["microservices"] += 0.2;
    }

    advanceTime(
        p.difficulty * 30
        );
    updateBurnout(-1);
    updateMood(+15);
    updateStress(-10);

    triggerCodeReview(projectName, p.difficulty, p.bugs);

    m_state.currentProjects.dequeue();

    return {
        QString(
            "Проект '%1' задеплоен! +%2 денег, +%3 опыта"
            )
            .arg(projectName)
            .arg(reward)
            .arg(p.difficulty * 10),
        "success"
    };
}

QPair<QString,QString> GameController::cmdOptimize() {
    if (m_state.energy < 18) return {"Слишком устал для оптимизации!", "warning"};
    QString info = SkillSystem::improveRandom(m_state, {"python","debugging"}, 0.1, 0.3);
    m_state.xp     += 8;
    m_state.energy -= 18;
    updateBurnout(+3);

    advanceTime(60);

    return {QString("Код оптимизирован! %1, +8 опыта").arg(info), "success"};
}

QPair<QString,QString> GameController::cmdMeeting() {
    if (m_state.energy < 12) return {"Слишком устал для митинга!", "warning"};
    int eLoss = QRandomGenerator::global()->bounded(10, 21);
    int xp    = QRandomGenerator::global()->bounded(5, 11);
    m_state.energy -= eLoss;
    advanceTime(90);
    updateBurnout(+3);
    updateStress(+8);
    updateMood(-5);
    m_state.xp     += xp;
    m_state.skills["api_design"] += 0.1 * SkillSystem::skillBonusMultiplier(m_state);
    return {QString("Митинг проведён. -%1 энергии, +%2 опыта").arg(eLoss).arg(xp), "text"};
}

QPair<QString,QString> GameController::cmdResearch() {
    if (m_state.energy < 25) return {"Слишком устал для исследований!", "warning"};
    int cost = SkillSystem::researchCost(m_state);
    if (m_state.money < cost) return {QString("Недостаточно денег! (%1)").arg(cost), "warning"};
    QString info = SkillSystem::improveRandom(m_state,
        {"docker","aws","ml","security","kubernetes","redis","graphql","microservices","serverless"},
        0.2, 0.4);
    m_state.money  -= cost;
    m_state.energy -= 45;
    advanceTime(240);
    updateBurnout(+6);
    m_state.xp     += 36;
    return {QString("Исследование завершено! %1, -%2 денег").arg(info).arg(cost), "success"};
}

QPair<QString,QString> GameController::cmdMentor() {
    if (m_state.energy < 15) return {"Слишком устал для менторства!", "warning"};
    int cost = SkillSystem::mentorCost(m_state);
    if (m_state.money < cost) return {QString("Недостаточно денег! (%1)").arg(cost), "warning"};
    int xp  = QRandomGenerator::global()->bounded(10, 21);
    int rep = QRandomGenerator::global()->bounded(1, 4);
    m_state.xp         += xp;
    m_state.reputation += rep;
    m_state.money      -= cost;
    m_state.energy     -= 35;
    advanceTime(120);
    updateBurnout(+8);
    return {QString("Менторинг завершён! +%1 опыта, +%2 репутации (-%3 денег)").arg(xp).arg(rep).arg(cost), "success"};
}

QPair<QString,QString> GameController::cmdHomework(const QString &action)
{
    if (m_state.lifeStage != LifeStage::Student)
        return {"Ты уже не студент.", "warning"};

    if (!m_state.pendingHomework)
        return {"Нет активного домашнего задания.", "text"};

    if (action == "skip")
    {
        int caught = QRandomGenerator::global()->bounded(100);
        m_state.pendingHomework = false;

        if (caught < 30)
        {
            m_state.studyScore = qMax(0, m_state.studyScore - 15);
            updateStress(+10);
            return {
                "Поймали на списывании! Успеваемость -15.",
                "error"
            };
        }
        return {"Списал. Пронесло.", "text"};
    }

    if (action == "order")
    {
        int cost = QRandomGenerator::global()->bounded(500, 1501);
        if (m_state.money < cost)
            return {
                QString("Нет денег на заказ. Нужно %1.").arg(cost),
                "error"
            };

        m_state.money -= cost;
        m_state.pendingHomework = false;
        m_state.studyScore = qMin(100, m_state.studyScore + 3);
        return {
            QString("Заказал выполнение. -%1. Успеваемость +3.")
                .arg(cost),
            "success"
        };
    }

    if (m_state.energy < 15)
        return {"Слишком устал. Отдохни перед заданием.", "warning"};

    int energyCost = QRandomGenerator::global()->bounded(15, 31);
    m_state.energy = qMax(0, m_state.energy - energyCost);
    advanceTime(90);

    m_state.pendingHomework = false;
    m_state.studyScore = qMin(100, m_state.studyScore + 5);

    auto subjects = CareerData::csSubjects();
    if (subjects.contains(m_state.homeworkSubject))
    {
        QMap<QString,QString> subjectToSkill = {
                                                 {"Алгоритмы и структуры данных", "debugging"},
                                                 {"Базы данных",                  "sql"},
                                                 {"Операционные системы",         "deployment"},
                                                 {"Сети и протоколы",             "api_design"},
                                                 {"Веб-разработка",               "python"},
                                                 {"Docker",                       "docker"},
                                                 {"Архитектура ПО",               "microservices"},
                                                 };

        QString skill = subjectToSkill.value(
            m_state.homeworkSubject, "");
        if (!skill.isEmpty() && m_state.skills.contains(skill))
        {
            m_state.skills[skill] =
                qMin(10.0, m_state.skills[skill] + 0.1);
        }
    }

    updateMood(+5);

    return {
        QString("Сделал домашнее по \"%1\". "
                "-%2 энергии, успеваемость +5.")
            .arg(m_state.homeworkSubject)
            .arg(energyCost),
        "success"
    };
}

QPair<QString,QString> GameController::cmdCoursework()
{
    if (m_state.lifeStage != LifeStage::Student)
        return {"Ты уже не студент.", "warning"};

    if (!m_state.courseWorkActive)
        return {"Нет активной курсовой работы.", "text"};

    if (m_state.courseWorkSubmitted)
        return {"Курсовая уже сдана.", "text"};

    if (m_state.energy < 20)
        return {"Слишком устал. Нужно минимум 20 энергии.", "warning"};

    int energyCost = QRandomGenerator::global()->bounded(20, 36);
    m_state.energy = qMax(0, m_state.energy - energyCost);
    advanceTime(120);
    updateStress(+5);

    double skillAvg = 0;
    for (auto v : m_state.skills) skillAvg += v;
    skillAvg /= qMax(1, m_state.skills.size());

    int gain = int(5 + skillAvg * 2 + (m_state.mood / 20));
    m_state.courseWorkProgress =
        qMin(100, m_state.courseWorkProgress + gain);

    if (m_state.courseWorkProgress >= 100)
    {
        m_state.courseWorkSubmitted = true;
        m_state.courseWorkActive   = false;
        m_state.studyScore = qMin(100, m_state.studyScore + 15);
        m_state.xp += 100;
        updateMood(+20);
        updateStress(-15);

        return {
            QString("Курсовая \"%1\" сдана! "
                    "+15 успеваемости, +100 XP.")
                .arg(m_state.courseWorkName),
            "success"
        };
    }

    return {
        QString("Курсовая \"%1\": %2% (+%3). "
                "Дедлайн: день %4. -%5 энергии.")
            .arg(m_state.courseWorkName)
            .arg(m_state.courseWorkProgress)
            .arg(gain)
            .arg(m_state.courseWorkDeadline)
            .arg(energyCost),
        "success"
    };
}

QPair<QString,QString> GameController::cmdTutor()
{
    if (m_state.lifeStage != LifeStage::Student)
        return {"Репетиторство доступно только студентам.", "warning"};

    if (m_state.energy < 20)
        return {"Нет сил преподавать.", "warning"};

    if (m_state.gameHours < 14 || m_state.gameHours > 20)
        return {"Репетиторство — с 14:00 до 20:00.", "warning"};

    int earnings = QRandomGenerator::global()->bounded(500, 1201);
    m_state.money += earnings;
    m_state.energy = qMax(0, m_state.energy - 20);
    m_state.tutoringSessions++;
    advanceTime(60);
    updateStress(+3);

    m_state.skills["api_design"] =
        qMin(10.0, m_state.skills["api_design"] + 0.05);

    return {
        QString("Провёл урок репетиторства. +%1, -20 энергии. "
                "Всего сессий: %2.")
            .arg(earnings)
            .arg(m_state.tutoringSessions),
        "success"
    };
}

QPair<QString,QString> GameController::cmdParents()
{
    if (m_state.lifeStage != LifeStage::Student)
        return {"Это работает только для студентов.", "warning"};

    static int lastParentsDay = 0;
    if (m_state.gameDay - lastParentsDay < 30)
        return {
            QString("Родители уже помогали. "
                    "Следующий раз через %1 дней.")
                .arg(30 - (m_state.gameDay - lastParentsDay)),
            "warning"
        };

    int amount = QRandomGenerator::global()->bounded(1000, 3001);
    m_state.money  += amount;
    lastParentsDay  = m_state.gameDay;
    m_state.parentAllowance = amount;
    updateMood(+5);

    return {
        QString("Родители помогли: +%1.").arg(amount),
        "success"
    };
}

QPair<QString,QString> GameController::cmdGrant()
{
    if (m_state.lifeStage != LifeStage::Student)
        return {"Гранты доступны только студентам.", "warning"};

    if (m_state.studyScore < 70)
        return {
            "Для получения гранта нужна успеваемость 70+. "
            "Сейчас: " + QString::number(m_state.studyScore),
            "warning"
        };

    if (m_state.grantAmount > 0)
        return {
            QString("Грант уже активен: %1/90 дней.")
                .arg(m_state.gameDay % 90),
            "text"
        };

    int chance = m_state.studyScore - 60;
    if (QRandomGenerator::global()->bounded(100) < chance)
    {
        m_state.grantAmount = QRandomGenerator::global()
        ->bounded(3000, 8001);
        updateMood(+15);
        return {
            QString("Грант получен! +%1 каждые 90 дней.")
                .arg(m_state.grantAmount),
            "success"
        };
    }

    return {
        "Заявка на грант отклонена. Попробуй позже.",
        "warning"
    };
}

QPair<QString,QString> GameController::cmdFreelance()
{
    if (m_state.energy < 15)
        return {"Слишком устал для фриланса!", "warning"};

    int reward =
        QRandomGenerator::global()
            ->bounded(300, 801);

    int xp =
        QRandomGenerator::global()
            ->bounded(5, 16);

    m_state.money += reward;
    m_state.xp += xp;
    m_state.energy -= 15;

    advanceTime(180);
    updateBurnout(+3);
    updateMood(+10);
    updateStress(-5);

    return {
        QString(
            "Фриланс-заказ выполнен! +%1 денег, +%2 опыта"
            )
            .arg(reward)
            .arg(xp),
        "success"
    };
}

QPair<QString,QString> GameController::cmdReview(const QString &action)
{
    if (!m_state.pendingReview)
        return {"Нет активного code review.", "warning"};

    if (action == "fix")
    {
        int energyCost = m_state.reviewBugsFound * 8;
        if (m_state.energy < energyCost)
            return {
                QString("Не хватает энергии. Нужно %1.").arg(energyCost),
                "error"
            };

        m_state.energy -= energyCost;
        m_state.pendingReview = false;

        m_state.skills["debugging"] =
            qMin(10.0, m_state.skills["debugging"] + 0.2);

        int xpGain = m_state.reviewBugsFound * 15;
        m_state.xp += xpGain;
        m_state.reputation += 1;
        advanceTime(m_state.reviewBugsFound * 20);

        return {
            QString("Исправил %1 багов из ревью. "
                    "+%2 XP, +1 репутация, debugging +0.2")
                .arg(m_state.reviewBugsFound)
                .arg(xpGain),
            "success"
        };
    }
    else
    {
        m_state.pendingReview = false;
        m_state.reputation    = qMax(0, m_state.reputation - 3);
        updateBurnout(+3);
        updateTechDebt(+8);

        return {
            QString("Проигнорировал замечания. "
                    "-3 репутации. Технический долг растёт."),
            "warning"
        };
    }
}

QPair<QString,QString> GameController::cmdRefactor() {
    if (m_state.energy < 20) return {"Слишком устал для рефакторинга!", "warning"};
    QString info = SkillSystem::improveRandom(m_state, {"python","debugging","api_design"}, 0.2, 0.4);
    m_state.xp     += 10;
    m_state.energy -= 20;
    advanceTime(150);
    updateBurnout(+3);
    updateTechDebt(-15);
    updateMood(+5);
    return {QString("Рефакторинг завершён! %1, +10 опыта").arg(info), "success"};
}

QPair<QString,QString> GameController::cmdDocument() {
    if (m_state.energy < 12) return {"Слишком устал для документации!", "warning"};
    int money = int(QRandomGenerator::global()->bounded(50, 101) * SkillSystem::reputationBonus(m_state));
    int xp    = QRandomGenerator::global()->bounded(5, 11);
    m_state.money  += money;
    m_state.xp     += xp;
    m_state.energy -= 12;
    advanceTime(90);
    updateBurnout(+2);
    updateTechDebt(-5);
    return {QString("Документация написана! +%1 денег, +%2 опыта").arg(money).arg(xp), "success"};
}

QPair<QString,QString> GameController::cmdAnalyze() {
    if (m_state.energy < 15) return {"Слишком устал для анализа!", "warning"};
    QString info = SkillSystem::improveRandom(m_state, {"python","debugging","deployment"}, 0.1, 0.2);
    m_state.xp     += 7;
    m_state.energy -= 35;
    advanceTime(120);
    updateBurnout(+3);
    return {QString("Анализ производительности завершён! %1, +7 опыта").arg(info), "success"};
}

QPair<QString,QString> GameController::cmdScale() {
    if (m_state.energy < 25) return {"Слишком устал для масштабирования!", "warning"};
    int cost = SkillSystem::scaleCost(m_state);
    if (m_state.money < cost) return {QString("Недостаточно денег! (%1)").arg(cost), "warning"};
    QString info = SkillSystem::improveRandom(m_state, {"kubernetes","aws","microservices"}, 0.3, 0.5);
    m_state.money  -= cost;
    m_state.xp     += 20;
    m_state.energy -= 55;
    advanceTime(270);
    updateBurnout(+4);
    return {QString("Система масштабирована! %1, +20 опыта (-%2 денег)").arg(info).arg(cost), "success"};
}

QPair<QString,QString> GameController::cmdMigrate() {
    if (m_state.energy < 22) return {"Слишком устал для миграции!", "warning"};
    int cost = SkillSystem::migrateCost(m_state);
    if (m_state.money < cost) return {QString("Недостаточно денег! (%1)").arg(cost), "warning"};
    if (QRandomGenerator::global()->generateDouble() < 0.7) {
        QString info = SkillSystem::improveRandom(m_state, {"sql"}, 0.2, 0.4);
        m_state.money  -= cost;
        m_state.xp     += 18;
        m_state.energy -= 22;
        advanceTime(180);
        updateBurnout(+5);
        return {QString("Миграция базы успешна! %1, +18 опыта (-%2 денег)").arg(info).arg(cost), "success"};
    }
    m_state.energy -= 18;
    advanceTime(180);
    updateBurnout(+7);
    return {"Миграция провалилась! Потеря данных. (-18 энергии)", "error"};
}

QPair<QString,QString> GameController::cmdAudit() {
    if (m_state.energy < 20) return {"Слишком устал для аудита!", "warning"};
    int cost = SkillSystem::auditCost(m_state);
    if (m_state.money < cost) return {QString("Недостаточно денег! (%1)").arg(cost), "warning"};
    QString info = SkillSystem::improveRandom(m_state, {"security"}, 0.3, 0.6);
    m_state.money  -= cost;
    m_state.xp     += 25;
    m_state.energy -= 20;
    advanceTime(180);
    updateBurnout(+4);
    return {QString("Аудит безопасности завершён! %1, +25 опыта (-%2 денег)").arg(info).arg(cost), "success"};
}

QPair<QString,QString> GameController::cmdSchedule()
{
    if (m_state.lifeStage != LifeStage::Student)
        return {"Ты уже не студент.", "warning"};

    if (m_state.schedule.isEmpty())
        return {"Расписание пусто.", "warning"};

    QString out = QString("── Расписание (успеваемость: %1/100) ──\n")
                      .arg(m_state.studyScore);

    for (const auto &e : m_state.schedule)
    {
        if (e.attended) continue;

        QString typeName;
        switch (e.type) {
        case StudyEventType::Lecture:  typeName = "Лекция";   break;
        case StudyEventType::Practice: typeName = "Практика"; break;
        case StudyEventType::Lab:      typeName = "Лаба";  break;
        case StudyEventType::Exam:     typeName = "ЭКЗАМЕН"; break;
        }

        out += QString("День %1 %2:00 — %3: %4\n")
                   .arg(e.day)
                   .arg(e.hour, 2, 10, QChar('0'))
                   .arg(typeName)
                   .arg(e.subject);
    }

    return {out.trimmed(), "text"};
}

QPair<QString,QString> GameController::cmdStatus()
{
    QString out;

    out += QString("── Статус ─────────────────────\n");
    out += QString("День %1 | %2:%3\n")
               .arg(m_state.gameDay)
               .arg(m_state.gameHours, 2, 10, QChar('0'))
               .arg(m_state.gameMinutes, 2, 10, QChar('0'));
    out += QString("Должность: %1\n")
               .arg(m_state.currentJob.displayName);
    out += QString("Энергия: %1/%2 | Выгорание: %3/100\n")
               .arg(m_state.energy)
               .arg(m_state.maxEnergy)
               .arg(m_state.burnout);
    out += QString("Деньги: %1 | Долг: %2\n")
               .arg(m_state.money)
               .arg(m_state.debt);
    out += QString("Еда: %1 дн. | Беспорядок: %2/10\n")
               .arg(m_state.foodStock)
               .arg(m_state.roomMessLevel);

    if (m_state.pendingIncident)
        out += "Прод горит! Введи fix или ignore.\n";
    if (m_state.pendingReview)
        out += QString("Ожидает code review: %1 багов. "
                       "Введи review fix или review skip.\n")
                   .arg(m_state.reviewBugsFound);
    if (m_state.pendingLab)
        out += QString("Активная лаба: %1. "
                       "Введи submit.\n")
                   .arg(m_state.pendingLabSubject);

    if (m_state.lifeStage == LifeStage::Student)
        out += QString("Успеваемость: %1/100 | Лаб сдано: %2/%3\n")
                   .arg(m_state.studyScore)
                   .arg(m_state.labsCompleted)
                   .arg(m_state.labsRequired);

    return {out.trimmed(), "text"};
}

QPair<QString,QString> GameController::cmdBuy(const QString &item)
{
    auto withNDS = [&](int price) {
        return int(price * (1.0f + m_state.ndsRate));
    };

    if (item == "food")
    {
        int cost = withNDS(800);
        if (m_state.money < cost)
            return {QString("Нет денег. Нужно %1").arg(cost), "error"};

        m_state.money -= cost;
        m_state.foodStock       = qMin(7, m_state.foodStock + 3);
        m_state.daysWithoutFood = 0;
        m_state.energy = qMin(m_state.maxEnergy, m_state.energy + 20);

        return {
            QString("Купил продукты (+3 дня еды, +20 энергии, -%1 с НДС)."
                    " Запас: %2 дн.")
                .arg(cost)
                .arg(m_state.foodStock),
            "success"
        };
    }

    if (item == "laptop")
    {
        auto eq = CareerData::equipmentInfo(EquipmentTier::NormalLaptop);
        int cost = withNDS(eq.price);
        if (m_state.money < cost) return {QString("Нет денег. Нужно %1").arg(cost), "error"};
        m_state.money    -= cost;
        m_state.equipment = eq;
        m_state.maxEnergy = eq.maxEnergy;
        return {QString("Куплен %1 (+%2 к макс. энергии)")
                    .arg(eq.name).arg(eq.maxEnergy - 200), "success"};
    }

    if (item == "gaming_pc")
    {
        auto eq = CareerData::equipmentInfo(EquipmentTier::GamingPC);
        int cost = withNDS(eq.price);
        if (m_state.money < cost) return {QString("Нет денег. Нужно %1").arg(cost), "error"};
        m_state.money    -= cost;
        m_state.equipment = eq;
        m_state.maxEnergy = eq.maxEnergy;
        return {QString("Куплен %1").arg(eq.name), "success"};
    }

    if (item == "workstation")
    {
        auto eq = CareerData::equipmentInfo(EquipmentTier::WorkStation);
        int cost = withNDS(eq.price);
        if (m_state.money < cost) return {QString("Нет денег. Нужно %1").arg(cost), "error"};
        m_state.money    -= cost;
        m_state.equipment = eq;
        m_state.maxEnergy = eq.maxEnergy;
        return {QString("Куплена %1 — теперь работаешь быстрее.").arg(eq.name), "success"};
    }

    return {"Неизвестный товар. Доступно: food, laptop, gaming_pc, workstation", "warning"};
}

QPair<QString,QString> GameController::cmdLaundry()
{
    if (!m_state.laundryPending && m_state.laundryDays < 3)
        return {"Стирать пока не нужно.", "text"};

    int cost = 100;
    if (m_state.money < cost)
        return {QString("Нет денег на стирку (%1).").arg(cost), "error"};

    m_state.money      -= cost;
    m_state.laundryDays = 0;
    m_state.laundryPending = false;
    advanceTime(60);

    return {"Постирал вещи. Чувствуешь себя лучше. -60 мин.", "success"};
}

QPair<QString,QString> GameController::cmdWalk()
{
    if (m_state.gameHours < 8 || m_state.gameHours > 21)
        return {"Поздновато для прогулки.", "warning"};

    m_state.daysIndoor = 0;
    m_state.energy = qMin(m_state.maxEnergy, m_state.energy + 15);
    updateBurnout(-5);
    advanceTime(45);
    updateMood(+10);
    updateStress(-8);

    return {"Вышел на улицу. +15 энергии, выгорание -5.", "success"};
}

QPair<QString,QString> GameController::cmdClean()
{
    if (m_state.roomMessLevel < 3)
        return {"В комнате и так чисто.", "text"};

    int cleaned = qMin(m_state.roomMessLevel, 5);
    m_state.roomMessLevel -= cleaned;
    m_state.energy = qMax(0, m_state.energy - 10);
    advanceTime(30);

    return {
        QString("Убрался в комнате. Уровень беспорядка: %1/10.")
            .arg(m_state.roomMessLevel),
        "success"
    };
}

QPair<QString,QString> GameController::cmdJobs()
{
    if (m_state.jobMarket.isEmpty() ||
        m_state.gameDay >= m_state.jobMarketRefreshDay + 7)
    {
        m_state.jobMarket =
            CareerData::generateJobMarket(m_state);
        m_state.jobMarketRefreshDay = m_state.gameDay;
    }

    m_state.jobMarket.removeIf([&](const JobOffer &o){
        return o.expiresDay < m_state.gameDay;
    });

    if (m_state.jobMarket.isEmpty())
        return {"Нет доступных вакансий. Подожди обновления рынка.", "warning"};

    QString out = "── Рынок труда ─────────────────\n";

    for (int i = 0; i < m_state.jobMarket.size(); i++)
    {
        const auto &o = m_state.jobMarket[i];
        CompanyInfo ci = CareerData::companyInfo(o.companyType);
        JobInfo     ji = CareerData::jobInfo(o.title);

        double skillAvg = 0;
        for (auto v : m_state.skills) skillAvg += v;
        skillAvg /= qMax(1, m_state.skills.size());

        bool available =
            m_state.level    >= o.levelRequired &&
            m_state.reputation >= o.repRequired &&
            skillAvg         >= o.skillRequired;

        QString status = available ? "✓" : "✗";

        out += QString("[%1] %2 %3 @ %4\n"
                       "    Зарплата: %5/день | "
                       "Требования: lvl%6 rep%7 skill%.1f\n"
                       "    %8\n"
                       "    Истекает: день %9\n")
                   .arg(i + 1)
                   .arg(status)
                   .arg(ji.displayName)
                   .arg(ci.name)
                   .arg(o.salaryPerDay)
                   .arg(o.levelRequired)
                   .arg(o.repRequired)
                   .arg(o.skillRequired)
                   .arg(o.description)
                   .arg(o.expiresDay);
    }

    out += "\nДля подачи заявки: apply <номер>";

    return {out.trimmed(), "text"};
}

QPair<QString,QString> GameController::cmdApply(int index)
{
    if (m_state.jobMarket.isEmpty())
        return {"Сначала посмотри вакансии: jobs", "warning"};

    if (index < 1 || index > m_state.jobMarket.size())
        return {QString("Нет вакансии с номером %1").arg(index), "error"};

    if (m_state.interviewsCooldown > m_state.gameDay)
        return {
            QString("Следующее собеседование можно пройти на день %1.")
                .arg(m_state.interviewsCooldown),
            "warning"
        };

    const JobOffer &offer = m_state.jobMarket[index - 1];

    double skillAvg = 0;
    for (auto v : m_state.skills) skillAvg += v;
    skillAvg /= qMax(1, m_state.skills.size());

    if (m_state.level < offer.levelRequired)
        return {
            QString("Недостаточный уровень. Нужно: %1, у тебя: %2")
                .arg(offer.levelRequired).arg(m_state.level),
            "error"
        };

    if (m_state.reputation < offer.repRequired)
        return {
            QString("Недостаточная репутация. Нужно: %1, у тебя: %2")
                .arg(offer.repRequired).arg(m_state.reputation),
            "error"
        };

    advanceTime(120);
    m_state.energy = qMax(0, m_state.energy - 25);
    m_state.interviewsCooldown = m_state.gameDay + 2;

    emit_msg("── Собеседование ───────────────", "dim");
    emit_msg(
        QString("Подал заявку: %1 @ %2")
            .arg(CareerData::jobInfo(offer.title).displayName)
            .arg(CareerData::companyInfo(offer.companyType).name),
        "text"
        );

    int passChance = 40
                     + int((skillAvg - offer.skillRequired) * 10)
                     + (m_state.reputation - offer.repRequired) / 2
                     - int(m_state.burnout * 0.2)
                     - m_state.sleepDebt * 5
                     + (m_state.mood - 50) / 5
                     - m_state.stress / 10
                     + (m_state.consecutiveRefusals > 2 ? -10 : 0);

    passChance = qBound(5, passChance, 90);

    int roll = QRandomGenerator::global()->bounded(100);

    QStringList questions = {
        "Расскажи о REST vs GraphQL.",
        "Что такое SOLID?",
        "Как работает индекс в БД?",
        "Объясни CAP-теорему.",
        "Что такое Docker?",
        "Как работает TCP/IP?",
        "Что такое микросервисы?"
    };
    QString question = questions[
        QRandomGenerator::global()->bounded(questions.size())];

    emit_msg(QString("Вопрос: %1").arg(question), "text");

    if (roll < passChance)
    {
        m_state.consecutiveRefusals = 0;

        CompanyInfo newCompany = CareerData::companyInfo(offer.companyType);
        JobInfo     newJob     = CareerData::jobInfo(offer.title);

        if (m_state.lifeStage == LifeStage::Employed)
        {
            emit_msg(
                QString("Уволился из %1.")
                    .arg(m_state.currentCompany.name),
                "text"
                );
        }

        m_state.lifeStage      = LifeStage::Employed;
        m_state.currentJob     = newJob;
        m_state.currentCompany = newCompany;

        if (m_state.housingType == HousingType::Dorm)
        {
            m_state.housingType = HousingType::Rent;
            m_state.rentCost    = 15000;
            emit_msg(
                "Теперь снимаешь квартиру. Аренда: 15 000/мес.",
                "text"
                );
        }

        m_state.jobMarket.removeAt(index - 1);

        return {
            QString("Принят! %1 @ %2. Зарплата: %3/день.")
                .arg(newJob.displayName)
                .arg(newCompany.name)
                .arg(newCompany.salaryPerDay + newJob.salaryBonus),
            "success"
        };
    }
    else
    {
        m_state.consecutiveRefusals++;
        updateBurnout(+5);

        QString reason;
        if (skillAvg < offer.skillRequired)
            reason = "Недостаточный уровень навыков.";
        else if (m_state.burnout > 60)
            reason = "Выглядел уставшим и растерянным.";
        else if (m_state.sleepDebt > 3)
            reason = "Был не в форме — чувствовалось по ответам.";
        else
            reason = "Не прошёл техническую часть.";

        return {
            QString("Отказ. %1 Отказов подряд: %2.")
                .arg(reason)
                .arg(m_state.consecutiveRefusals),
            "warning"
        };
    }
}

QPair<QString,QString> GameController::cmdQuit()
{
    if (m_state.lifeStage != LifeStage::Employed)
        return {"Ты нигде не работаешь.", "warning"};

    QString company = m_state.currentCompany.name;

    m_state.lifeStage      = LifeStage::Freelancer;
    m_state.currentJob     = CareerData::jobInfo(JobTitle::Freelancer);
    m_state.currentCompany = CareerData::companyInfo(CompanyType::None);

    m_state.reputation = qMax(0, m_state.reputation - 5);

    return {
        QString("Уволился из %1. -5 репутации. "
                "Теперь ты фрилансер.")
            .arg(company),
        "text"
    };
}

QPair<QString,QString> GameController::cmdSave()
{
    if (SaveManager::save(m_state))
        return {"Игра сохранена", "success"};

    return {"Ошибка сохранения", "error"};
}

QPair<QString,QString> GameController::cmdLoad()
{
    if (SaveManager::load(m_state))
    {
        if (m_state.guiUnlocked)
            emit guiUnlocked();

        emit stateChanged();

        return {"Игра загружена", "success"};
    }

    return {"Файл сохранения не найден", "error"};

}

void GameController::checkLevelUp() {
    if (m_state.xp < m_state.level * 100) return;
    int old = m_state.level;
    m_state.level++;
    m_state.xp = 0;
    QString bonus;
    if (m_state.level % 5 == 0) {
        m_state.maxConcurrentProjects++;
        m_state.maxQueueSize += 2;
        m_state.maxEnergy    += 20;
        bonus = ", +1 к макс. проектам, +2 к очереди, +20 энергии";
    } else {
        m_state.maxEnergy += 10;
        bonus = ", +10 энергии";
    }
    m_state.energy = m_state.maxEnergy;
    if (m_state.level > m_state.maxLevel) {
        m_state.level = m_state.maxLevel;
        emit_msg(QString("Достигнут максимальный уровень %1!").arg(m_state.maxLevel), "highlight");
    } else {
        emit_msg(QString("Уровень повышен! %1 → %2%3").arg(old).arg(m_state.level).arg(bonus), "highlight");
        emit levelUp(m_state.level);
    }
}

void GameController::updateProjectQueue() {
    double chance = 0.3 + (m_state.reputation / 1000.0);
    if (QRandomGenerator::global()->generateDouble() < chance
        && m_state.projectQueue.size() < m_state.maxQueueSize) {
        Project p = ProjectQueue::generateProject(m_state.level);
        m_state.projectQueue.enqueue(p);
        emit_msg(QString("Новый проект в очереди: %1").arg(p.name));
    }
}

void GameController::handleRandomEvent() {
    double meetingChance = 0.05 - qMin(0.03, m_state.reputation / 2000.0);
    double r = QRandomGenerator::global()->generateDouble();

    if (r < meetingChance) {
        int loss = QRandomGenerator::global()->bounded(15, 26);
        m_state.energy = qMax(0, m_state.energy - loss);
        emit_msg(QString("Внезапный митинг! -%1 энергии").arg(loss), "warning");
    } else if (r < meetingChance + 0.04) {
        if (!m_state.currentProjects.isEmpty()) {
            Project p = m_state.currentProjects.dequeue();
            m_state.projectQueue.prepend(p);
            emit_msg("Срочный баг-репорт! Проект отложен.", "warning");
        }
    } else if (r < meetingChance + 0.04 + 0.03 + m_state.reputation / 3000.0) {
        int bonus = int(QRandomGenerator::global()->bounded(150, 601) * SkillSystem::reputationBonus(m_state));
        m_state.money += bonus;
        emit_msg(QString("Неожиданный бонус! +%1 денег").arg(bonus), "success");
    } else if (r < meetingChance + 0.09 + m_state.reputation / 3000.0) {
        if (!m_state.currentProjects.isEmpty()) {
            int loss = QRandomGenerator::global()->bounded(20, 31);
            m_state.energy = qMax(0, m_state.energy - loss);
            emit_msg(QString("Срочная проблема в проекте! -%1 энергии").arg(loss), "error");
        }
    }
}

void GameController::onGameTick()
{
    advanceTime(1);

    if (m_state.gameMinutes % 5 == 0)
    {
        SaveManager::save(m_state);
    }

    emit stateChanged();
}

void GameController::checkPromotion()
{
    JobTitle best = CareerData::nextTitle(m_state);

    if (best > m_state.currentJob.title)
    {
        JobInfo newJob = CareerData::jobInfo(best);
        emit_msg(
            QString("Повышение! %1 → %2 (+%3 к зарплате/день)")
                .arg(m_state.currentJob.displayName)
                .arg(newJob.displayName)
                .arg(newJob.salaryBonus - m_state.currentJob.salaryBonus),
            "highlight"
            );
        m_state.currentJob = newJob;
    }
}

void GameController::updateBurnout(int delta)
{
    m_state.burnout = qBound(0, m_state.burnout + delta, m_state.maxBurnout);

    if (delta > 0 && m_state.burnout >= 70)
        emit_msg("Ты сильно устал. Производительность падает.", "warning");
    else if (delta > 0 && m_state.burnout >= 40)
        emit_msg("Чувствуешь усталость.", "warning");
}

void GameController::onNewDay()
{
    applyCharacterInteractions();

    m_state.sleptTonight = false;

    if (!m_state.sleptTonight && m_state.gameDay > 1)
    {
        m_state.sleepDebt += 7;
        updateBurnout(+10);
        emit_msg(
            QString("Не спал всю ночь. "
                    "Долг сна +7 ч., выгорание +10. "
                    "Сейчас долг: %1 ч.")
                .arg(m_state.sleepDebt),
            "error"
            );
    }

    for (auto &p : m_state.currentProjects)
    {
        if (p.deadlineDay <= 0) continue;

        int daysLeft = p.deadlineDay - m_state.gameDay;

        if (daysLeft == 3)
        {
            emit_msg(
                QString("Дедлайн через 3 дня: %1").arg(p.name),
                "warning"
                );
            updateStress(+5);
        }
        else if (daysLeft == 1)
        {
            emit_msg(
                QString("Завтра дедлайн: %1! Торопись.").arg(p.name),
                "warning"
                );
            updateStress(+10);
        }
        else if (daysLeft <= 0)
        {
            int repLoss   = p.difficulty * 5;
            int moneyLoss = p.reward / 2;

            m_state.reputation  = qMax(0, m_state.reputation - repLoss);
            m_state.money       = qMax(0, m_state.money - moneyLoss);
            m_state.deadlinesMissed++;
            m_state.burnout = qMin(m_state.maxBurnout, m_state.burnout + 10);

            emit_msg(
                QString("Дедлайн провален: %1\n"
                        "  -%2 репутации, -%3 денег, выгорание +10")
                    .arg(p.name)
                    .arg(repLoss)
                    .arg(moneyLoss),
                "error"
                );
            updateStress(+20);
            updateMood(-15);
            p.deadlineDay = 0;
        }
    }

    int salary = m_state.currentJob.salaryBonus;

    salary += m_state.currentCompany.salaryPerDay;

    salary = int(salary * m_state.currentCompany.reputationMultiplier);

    if (salary > 0)
    {
        int ndfl = int(salary * m_state.ndflRate);
        m_state.money -= ndfl;
        emit_msg(QString("НДФЛ: -%1 (13%)").arg(ndfl), "text");
    }

    m_state.money -= m_state.foodCost;
    if (m_state.money < 0)
        m_state.debt += -m_state.money;

    emit_msg(QString("Еда: -%1").arg(m_state.foodCost), "text");

    if (m_state.gameDay % 30 == 0)
    {
        if (m_state.money >= m_state.rentCost)
        {
            m_state.money -= m_state.rentCost;
            m_state.missedRentCount = 0;
            emit_msg(
                QString("Аренда оплачена: -%1").arg(m_state.rentCost),
                "text"
                );
        }
        else
        {
            m_state.missedRentCount++;
            emit_msg(
                QString("Нет денег на аренду! Долг растёт. (%1/3)")
                    .arg(m_state.missedRentCount),
                "error"
                );
        }
    }

    if (m_state.lifeStage == LifeStage::Student && m_state.gameDay % 30 == 0)
    {
        int stipend = 2500;
        m_state.money += stipend;
        emit_msg(QString("Стипендия: +%1").arg(stipend), "success");
    }

    if (m_state.housingType == HousingType::Dorm)
    {
        m_state.energy = qMax(0, m_state.energy - 5);
        emit_msg("Общага шумная. Плохо поспал. (-5 энергии)", "text");
    }


    if (salary > 0)
    {
        m_state.money += salary;
        emit_msg(
            QString("День %1: зарплата +%2 денег")
                .arg(m_state.gameDay)
                .arg(salary),
            "success"
            );
    }
    else
    {
        emit_msg(
            QString("День %1 начался. Найди работу — ты фрилансер.")
                .arg(m_state.gameDay),
            "text"
            );
    }

    int rest = QRandomGenerator::global()->bounded(10, 21);
    m_state.energy = qMin(m_state.maxEnergy, m_state.energy + rest);

    if (m_state.lastRestDay < m_state.gameDay - 1)
        m_state.daysWithoutRest++;
    else
        m_state.daysWithoutRest = 0;

    if (m_state.daysWithoutRest >= 7)
    {
        int burnoutPenalty = 15;
        int energyPenalty  = 30;
        m_state.burnout = qMin(m_state.maxBurnout,
                               m_state.burnout + burnoutPenalty);
        m_state.energy  = qMax(0, m_state.energy - energyPenalty);
        emit_msg(
            QString("Ты не отдыхал %1 дней. Выгорание +%2, энергия -%3. ")
                .arg(m_state.daysWithoutRest)
                .arg(burnoutPenalty)
                .arg(energyPenalty),
            "error"
            );
    }
    else if (m_state.daysWithoutRest >= 3)
    {
        int burnoutPenalty = 7;
        m_state.burnout = qMin(m_state.maxBurnout,
                               m_state.burnout + burnoutPenalty);
        emit_msg(
            QString("День %1 без отдыха. Усталость накапливается. "
                    "Выгорание +%2.")
                .arg(m_state.daysWithoutRest)
                .arg(burnoutPenalty),
            "warning"
            );
    }

    if (m_state.burnout >= 70)
    {
        m_state.highBurnoutDays++;

        if (m_state.highBurnoutDays == 3)
        {
            emit_msg(
                "Уже 3 дня на износе. Тяжело сосредоточиться.",
                "warning"
                );
        }
        else if (m_state.highBurnoutDays == 7)
        {
            int xpLoss = m_state.xp / 4;
            m_state.xp = qMax(0, m_state.xp - xpLoss);
            m_state.energy = qMax(0, m_state.energy - 40);
            emit_msg(
                QString("7 дней с высоким выгоранием. "
                        "Моральная усталость. -%1 XP, -40 энергии.")
                    .arg(xpLoss),
                "error"
                );
        }
        else if (m_state.highBurnoutDays == 14)
        {
            if (!m_state.currentProjects.isEmpty())
            {
                Project p = m_state.currentProjects.dequeue();
                m_state.reputation = qMax(0, m_state.reputation - 20);
                emit_msg(
                    QString("Ничего не хочется делать. "
                            "Бросил проект \"%1\". -20 репутации.")
                        .arg(p.name),
                    "error"
                    );
            }
            m_state.maxEnergy = qMax(100, m_state.maxEnergy - 20);
            emit_msg(
                "Максимальная энергия снизилась на 20. "
                "Нужен серьёзный отдых.",
                "error"
                );
        }
        else if (m_state.highBurnoutDays >= 21)
        {
            emit_msg(
                "Три недели на пределе. "
                "Ты не можешь больше работать.",
                "error"
                );
            m_state.burnout = m_state.maxBurnout;
        }
    }
    else
    {
        if (m_state.highBurnoutDays > 0)
        {
            emit_msg("Выгорание снижается. Становится легче.", "success");
            m_state.highBurnoutDays = 0;
        }
    }

    if (m_state.gameDay % 7 == 1)
        generateWeekSchedule();

    if (m_state.lifeStage == LifeStage::Student)
    {
        m_state.academicDay++;

        if ((m_state.academicDay == 135 || m_state.academicDay == 270)
            && !m_state.sessionActive)
        {
            m_state.sessionActive = true;
            emit_msg(
                "Началась сессия! Сдай все экзамены.",
                "error"
                );

            if (m_state.labsCompleted < m_state.labsRequired)
            {
                emit_msg(
                    QString("Не допущен к сессии! "
                            "Сдано лаб: %1/%2. Угроза отчисления.")
                        .arg(m_state.labsCompleted)
                        .arg(m_state.labsRequired),
                    "error"
                    );
                m_state.failedExams += 2;
                checkExpulsion();
            }
        }
    }

    checkGameOver();

    if (m_state.foodStock > 0)
    {
        m_state.foodStock--;
    }
    else
    {
        m_state.daysWithoutFood++;
        int penalty = m_state.daysWithoutFood * 10;
        m_state.energy = qMax(0, m_state.energy - penalty);
        updateBurnout(+3);
        emit_msg(
            QString("Нечего есть уже %1 дн. -%2 энергии. Купи еду!")
                .arg(m_state.daysWithoutFood)
                .arg(penalty),
            "error"
            );
    }

    m_state.roomMessLevel = qMin(10, m_state.roomMessLevel + 1);

    m_state.daysIndoor++;

    m_state.laundryDays++;

    generateDayEvents();

    if (m_state.lifeStage == LifeStage::Student)
    {
        if (!m_state.pendingHomework &&
            m_state.gameDay % QRandomGenerator::global()->bounded(3, 6) == 0)
        {
            auto subjects = CareerData::csSubjects();
            m_state.homeworkSubject = subjects[
                QRandomGenerator::global()->bounded(subjects.size())];
            m_state.homeworkDeadline = m_state.gameDay + 3;
            m_state.pendingHomework  = true;

            emit_msg(
                QString("Домашнее задание: %1. "
                        "Сдать до дня %2. Введи homework.")
                    .arg(m_state.homeworkSubject)
                    .arg(m_state.homeworkDeadline),
                "warning"
                );
        }

        if (m_state.pendingHomework &&
            m_state.gameDay > m_state.homeworkDeadline)
        {
            m_state.pendingHomework = false;
            m_state.homeworksMissed++;
            m_state.studyScore = qMax(0, m_state.studyScore - 8);
            updateStress(+5);

            emit_msg(
                QString("Просрочил домашнее по \"%1\". "
                        "Успеваемость -8. Пропущено: %2")
                    .arg(m_state.homeworkSubject)
                    .arg(m_state.homeworksMissed),
                "error"
                );

            if (m_state.homeworksMissed >= 5)
            {
                emit_msg(
                    "Слишком много пропущенных заданий. "
                    "Угроза отчисления.",
                    "error"
                    );
                m_state.failedExams++;
                checkExpulsion();
            }
        }

        int weekDay = m_state.gameDay % 7;
        if (weekDay >= 1 && weekDay <= 5)
        {
            m_state.money = qMax(0, m_state.money - m_state.transportCost);
        }

        if (m_state.hostingCost > 0 && m_state.gameDay % 30 == 0)
        {
            if (m_state.money >= m_state.hostingCost)
            {
                m_state.money -= m_state.hostingCost;
                emit_msg(
                    QString("Хостинг: -%1").arg(m_state.hostingCost),
                    "text"
                    );
            }
            else
            {
                emit_msg(
                    "Нет денег на хостинг! Проект недоступен.",
                    "error"
                    );
                updateStress(+5);
            }
        }

        if (m_state.parentAllowance > 0 && m_state.gameDay % 30 == 0)
        {
            m_state.money += m_state.parentAllowance;
            emit_msg(
                QString("Родители перевели: +%1")
                    .arg(m_state.parentAllowance),
                "success"
                );
        }

        if (m_state.grantAmount > 0 && m_state.gameDay % 90 == 0)
        {
            m_state.money += m_state.grantAmount;
            emit_msg(
                QString("Грант зачислен: +%1").arg(m_state.grantAmount),
                "success"
                );
        }

        if (!m_state.courseWorkActive &&
            !m_state.courseWorkSubmitted &&
            m_state.academicDay == 30)
        {
            QStringList projects = {
                "Backend интернет-магазина",
                "CRM система",
                "REST API для мобильного приложения",
                "Система управления задачами",
                "Чат-сервер на WebSocket"
            };
            m_state.courseWorkName     = projects[
                QRandomGenerator::global()->bounded(projects.size())];
            m_state.courseWorkProgress = 0;
            m_state.courseWorkDeadline = m_state.gameDay + 60;
            m_state.courseWorkActive   = true;

            emit_msg(
                QString("Курсовая работа: \"%1\". "
                        "Сдать до дня %2. Делай частями: coursework.")
                    .arg(m_state.courseWorkName)
                    .arg(m_state.courseWorkDeadline),
                "highlight"
                );
        }

        if (m_state.courseWorkActive &&
            !m_state.courseWorkSubmitted &&
            m_state.gameDay > m_state.courseWorkDeadline)
        {
            m_state.courseWorkActive = false;
            m_state.studyScore = qMax(0, m_state.studyScore - 25);
            m_state.failedExams++;
            updateStress(+20);
            updateMood(-15);

            emit_msg(
                "Не сдал курсовую в срок! "
                "Успеваемость -25. Угроза отчисления.",
                "error"
                );
            checkExpulsion();
        }
    }
}

void GameController::checkGameOver()
{
    QString reason;

    if (m_state.expelled)
        reason = "Тебя отчислили из университета.";
    else if (m_state.missedRentCount >= 3)
        reason = "Тебя выселили за долги по аренде.";
    else if (m_state.burnout >= 100 && m_state.deadlinesMissed >= 5)
        reason = "Полное выгорание + 5 просроченных дедлайнов.\nТы сломался.";
    else if (m_state.debt > 50000)
        reason = "Долг превысил 50 000. Банкротство.";
    else if (m_state.highBurnoutDays >= 21)
        reason = "Три недели хронического выгорания.\n"
                 "Ты выгорел полностью. Ничего не хочется.\n"
                 "Нужно всё начать заново.";

    if (!reason.isEmpty())
    {
        emit_msg("=== GAME OVER ===", "error");
        emit_msg(reason, "error");
        emit gameOver(reason);
    }
}

void GameController::generateDayEvents()
{
    int day  = m_state.gameDay;
    auto rnd = [](int a, int b){
        return QRandomGenerator::global()->bounded(a, b);
    };

    m_eventQueue.append({
        EventType::MorningCoffee, day, 8, false,
        "Утро. Выпил кофе. +10 энергии, мысли прояснились."
    });

    m_eventQueue.append({
        EventType::AfternoonSlump, day, 14, false,
        "После обеда клонит в сон. -15 энергии."
    });

    if (rnd(0, 100) < 40)
    {
        m_eventQueue.append({
            EventType::ClientCall, day, rnd(10, 18), false,
            "Звонок клиента. Обсуждение требований. -30 мин, -10 энергии."
        });
    }

    if (rnd(0, 100) < 30)
    {
        m_eventQueue.append({
            EventType::BugReport, day, rnd(9, 20), false,
            "Пришёл баг-репорт. Добавлен баг в текущий проект."
        });
    }

    if (rnd(0, 100) < 50)
    {
        m_eventQueue.append({
            EventType::Distraction, day, rnd(11, 22), false,
            "Залип на YouTube/Reddit. Потерял 45 минут."
        });
    }

    if (rnd(0, 100) < 15)
    {
        m_eventQueue.append({
            EventType::ServerDown, day, rnd(0, 5), false,
            "ALERT: Сервер упал. Нужно чинить прямо сейчас.",
            true
        });
    }

    if (day % 30 == 15)
    {
        m_eventQueue.append({
            EventType::InternetBill, day, 10, false,
            "Пришёл счёт за интернет. -500 денег."
        });
    }

    m_eventQueue.append({
        EventType::HungryAlert, day, 13, false,
        "Давно не ел. -10 энергии. Купи еду командой buy food."
    });
    m_eventQueue.append({
        EventType::HungryAlert, day, 19, false,
        "Снова голоден. Поешь нормально."
    });

    if (m_state.foodStock <= 1)
    {
        m_eventQueue.append({
            EventType::GroceryShopping, day, 10, false,
            "Еда заканчивается! Купи продукты: buy food"
        });
    }

    if (m_state.laundryDays >= 5)
    {
        m_eventQueue.append({
            EventType::LaundryReminder, day, rnd(10, 14), false,
            "Пора постирать вещи. Введи laundry."
        });
    }

    if (m_state.roomMessLevel >= 7)
    {
        m_eventQueue.append({
            EventType::RoomMess, day, rnd(8, 12), false,
            "В комнате полный хаос. Тяжело сосредоточиться. -10 энергии."
        });
    }

    if (m_state.daysIndoor >= 5)
    {
        m_eventQueue.append({
            EventType::HealthCheck, day, 15, false,
            "Ты не выходил на улицу 5 дней. Витамин D на нуле. -15 энергии."
        });
    }

    if (day % 30 == 20)
    {
        m_eventQueue.append({
            EventType::PhoneBill, day, 11, false,
            "Пришёл счёт за телефон. -300 денег."
        });
    }

    if ((m_state.gameDay + 1) % 30 == 0)
    {
        m_eventQueue.append({
            EventType::RentReminder, day, 9, false,
            QString("Завтра аренда! Нужно %1 денег. На счету: %2")
                .arg(m_state.rentCost)
                .arg(m_state.money)
        });
    }

    if (m_state.money < m_state.rentCost + 500)
    {
        m_eventQueue.append({
            EventType::LowMoneyAlert, day, 8, false,
            QString("Денег мало: %1. Скоро аренда %2.")
                .arg(m_state.money)
                .arg(m_state.rentCost)
        });
    }

    if (m_state.housingType == HousingType::Dorm && rnd(0, 100) < 30)
    {
        m_eventQueue.append({
            EventType::DormNoise, day, rnd(23, 24), false,
            "Соседи шумят ночью. Плохо заснул. -10 энергии, сон долга +1."
        });
    }

    if (m_state.housingType == HousingType::Dorm && rnd(0, 100) < 10)
    {
        m_eventQueue.append({
            EventType::RoommateIssue, day, rnd(18, 22), false,
            "Конфликт с соседом. Выгорание +5, потеря 30 минут."
        });
    }
}

void GameController::checkTimeEvents()
{
    for (auto &e : m_eventQueue)
    {
        if (e.triggered) continue;
        if (e.triggerDay  != m_state.gameDay)  continue;
        if (e.triggerHour != m_state.gameHours) continue;

        e.triggered = true;
        triggerEvent(e);
    }

    checkStudyEvents();

    m_eventQueue.removeIf([&](const GameEvent &e){
        return e.triggered ||
               e.triggerDay < m_state.gameDay;
    });
}

void GameController::triggerEvent(const GameEvent &e)
{
    emit_msg("── Событие ──────────────────", "dim");
    emit_msg(e.description, "warning");

    switch (e.type)
    {
    case EventType::MorningCoffee:
        m_state.energy = qMin(m_state.maxEnergy, m_state.energy + 10);
        break;

    case EventType::AfternoonSlump:
        m_state.energy = qMax(0, m_state.energy - 15);
        break;

    case EventType::ClientCall:
        m_state.energy = qMax(0, m_state.energy - 10);
        advanceTime(30);
        break;

    case EventType::BugReport:
        if (!m_state.currentProjects.isEmpty())
            m_state.currentProjects.head().bugs +=
                QRandomGenerator::global()->bounded(1, 4);
        break;

    case EventType::Distraction:
        advanceTime(45);
        updateBurnout(+2);
        break;

    case EventType::ServerDown:
    {
        m_state.prodIncidents++;

        bool isNight = (m_state.gameHours >= 0 && m_state.gameHours < 6);

        if (isNight)
        {
            emit_msg(
                "ALERT [02:34]: Прод упал. "
                "Пользователи не могут зайти.",
                "error"
                );
            emit_msg(
                "Тебя разбудили. Вводи fix или ignore.",
                "error"
                );
            m_state.sleepDebt++;
            m_state.sleptTonight = false;
        }
        else
        {
            emit_msg(
                "ALERT: Сервер недоступен в рабочее время.",
                "error"
                );
            emit_msg("Введи fix или ignore.", "warning");
        }

        m_state.pendingIncident = true;
        break;
    }

    case EventType::InternetBill:
        if (m_state.money >= 500)
        {
            m_state.money -= 500;
            emit_msg("Оплачено: -500 за интернет.", "text");
        }
        else
        {
            emit_msg("Нет денег на интернет! Работаешь медленнее.", "error");
            m_state.sleepDebt += 2;
        }
        break;

    case EventType::HungryAlert:
        m_state.energy = qMax(0, m_state.energy - 10);
        break;

    case EventType::GroceryShopping:
        emit_msg("Купи продукты командой buy food.", "warning");
        break;

    case EventType::LaundryReminder:
        emit_msg("Не забудь постирать — введи laundry.", "warning");
        break;

    case EventType::RoomMess:
        m_state.energy = qMax(0, m_state.energy - 10);
        emit_msg("Беспорядок мешает работать.", "error");
        break;

    case EventType::HealthCheck:
        m_state.energy = qMax(0, m_state.energy - 15);
        updateBurnout(+5);
        emit_msg("Выйди на улицу — введи walk.", "warning");
        break;

    case EventType::PhoneBill:
        if (m_state.money >= 300)
        {
            m_state.money -= 300;
            emit_msg("Оплачен телефон: -300.", "text");
        }
        else
        {
            emit_msg("Нет денег на телефон! Связь отключена.", "error");
            updateBurnout(+3);
        }
        break;

    case EventType::RentReminder:

        break;

    case EventType::LowMoneyAlert:

        break;

    case EventType::DormNoise:
        m_state.energy  = qMax(0, m_state.energy - 10);
        m_state.sleepDebt++;
        break;

    case EventType::RoommateIssue:
        advanceTime(30);
        updateBurnout(+5);
        break;

    default:
        break;
    }



    emit stateChanged();
}

void GameController::generateWeekSchedule()
{
    if (m_state.lifeStage != LifeStage::Student) return;

    m_state.schedule.clear();
    auto subjects = CareerData::csSubjects();
    auto rnd = [](int a, int b){
        return QRandomGenerator::global()->bounded(a, b);
    };

    int today = m_state.gameDay;

    for (int d = 0; d < 5; d++)
    {
        int dayOffset = today + d;
        int weekDay   = dayOffset % 7;

        if (weekDay == 5 || weekDay == 6) continue;

        QString subj1 = subjects[rnd(0, subjects.size())];
        QString subj2 = subjects[rnd(0, subjects.size())];

        m_state.schedule.append({
            StudyEventType::Lecture, dayOffset, 9, subj1
        });

        bool isLab = (rnd(0, 100) < 35);
        m_state.schedule.append({
            isLab ? StudyEventType::Lab : StudyEventType::Practice,
            dayOffset, 11, subj2
        });
    }

    emit_msg(
        QString("Новое расписание на неделю сгенерировано. "
                "Введи schedule чтобы посмотреть."),
        "text"
        );
}

void GameController::checkStudyEvents()
{
    if (m_state.lifeStage != LifeStage::Student) return;

    for (auto &e : m_state.schedule)
    {
        if (e.attended)   continue;
        if (e.day  != m_state.gameDay)  continue;
        if (e.hour != m_state.gameHours) continue;

        triggerStudyEvent(e);
    }
}

void GameController::handleExam(const StudyEvent &e)
{
    double skillAvg = 0;
    for (auto v : m_state.skills) skillAvg += v;
    skillAvg /= qMax(1, m_state.skills.size());

    int passChance = m_state.studyScore
                     + int(skillAvg * 5)
                     - m_state.sleepDebt * 3
                     - int(m_state.burnout * 0.3);

    passChance = qBound(5, passChance, 95);

    int roll = QRandomGenerator::global()->bounded(100);

    if (roll < passChance)
    {
        m_state.failedExams = 0;
        m_state.xp += 50;
        emit_msg(
            QString("Сдал экзамен по \"%1\"! +50 XP").arg(e.subject),
            "success"
            );
    }
    else
    {
        m_state.failedExams++;
        m_state.studyScore = qMax(0, m_state.studyScore - 20);
        emit_msg(
            QString("Завалил экзамен по \"%1\". "
                    "Успеваемость -20. Провалов подряд: %2")
                .arg(e.subject)
                .arg(m_state.failedExams),
            "error"
            );
        checkExpulsion();
    }
}

void GameController::checkExpulsion()
{
    if (m_state.failedExams >= 3 || m_state.studyScore <= 20)
    {
        m_state.expelled = true;
        emit_msg(
            "Тебя отчислили за неуспеваемость. GAME OVER.",
            "error"
            );
        checkGameOver();
    }
}

void GameController::triggerStudyEvent(StudyEvent &e)
{
    QString typeName;
    switch (e.type) {
    case StudyEventType::Lecture:  typeName = "Лекция";   break;
    case StudyEventType::Practice: typeName = "Практика"; break;
    case StudyEventType::Lab:      typeName = "Лаба";     break;
    case StudyEventType::Exam:     typeName = "Экзамен";  break;
    }

    emit_msg("── Учёба ────────────────────", "dim");
    emit_msg(
        QString("%1: %2").arg(typeName).arg(e.subject),
        "highlight"
        );

    bool isWorking = !m_state.currentProjects.isEmpty()
                     && m_state.gameHours >= 9
                     && m_state.gameHours <= 16;

    if (isWorking)
    {
        int penalty = 0;
        switch (e.type) {
        case StudyEventType::Lecture:  penalty = 5;  break;
        case StudyEventType::Practice: penalty = 8;  break;
        case StudyEventType::Lab:      penalty = 12; break;
        case StudyEventType::Exam:     penalty = 25; break;
        }
        m_state.studyScore = qMax(0, m_state.studyScore - penalty);
        emit_msg(
            QString("Пропустил %1. Успеваемость -%2 (%3/100)")
                .arg(typeName).arg(penalty).arg(m_state.studyScore),
            "error"
            );
    }
    else
    {
        int duration = (e.type == StudyEventType::Lab) ? 180 : 120;
        int energyCost = (e.type == StudyEventType::Exam) ? 30 : 15;

        advanceTime(duration);
        m_state.energy = qMax(0, m_state.energy - energyCost);

        if (e.type == StudyEventType::Lab)
        {
            m_state.pendingLab = true;
            m_state.pendingLabSubject = e.subject;
            emit_msg(
                QString("Лаба по \"%1\" начата. "
                        "Введи submit чтобы сдать (-20 энергии).")
                    .arg(e.subject),
                "warning"
                );
        }
        else if (e.type == StudyEventType::Exam)
        {
            handleExam(e);
        }
        else
        {
            m_state.studyScore = qMin(100, m_state.studyScore + 2);
            emit_msg(
                QString("Посетил %1 по \"%2\". "
                        "-%3 энергии, успеваемость: %4/100")
                    .arg(typeName).arg(e.subject)
                    .arg(energyCost).arg(m_state.studyScore),
                "success"
                );
        }
    }

    e.attended = true;
    emit stateChanged();
}

void GameController::triggerCodeReview(
    const QString &projectName,
    int difficulty,
    int bugsWasFixed)
{
    auto rnd = [](int a, int b){
        return QRandomGenerator::global()->bounded(a, b);
    };

    double skillAvg = 0;
    for (auto v : m_state.skills) skillAvg += v;
    skillAvg /= qMax(1, m_state.skills.size());

    int quality = int(skillAvg * 10)
                  - int(m_state.burnout * 0.3)
                  - m_state.sleepDebt * 3
                  + rnd(-10, 11);

    quality = qBound(0, quality, 100);

    int hiddenBugs = 0;
    if (quality < 30)       hiddenBugs = rnd(3, 7);
    else if (quality < 60)  hiddenBugs = rnd(1, 4);
    else if (quality < 80)  hiddenBugs = rnd(0, 2);

    m_state.pendingReview     = true;
    m_state.reviewProjectName = projectName;
    m_state.reviewBugsFound   = hiddenBugs;
    m_state.reviewQuality     = quality;

    emit_msg("── Code Review ──────────────", "dim");

    if (quality >= 80)
    {
        emit_msg(
            QString("LGTM! Качество кода: %1/100. "
                    "Отличная работа.").arg(quality),
            "success"
            );
    }
    else if (quality >= 60)
    {
        emit_msg(
            QString("Код приемлем. Качество: %1/100. "
                    "Есть замечания.").arg(quality),
            "text"
            );
    }
    else if (quality >= 40)
    {
        emit_msg(
            QString("Код требует доработки. "
                    "Качество: %1/100.").arg(quality),
            "warning"
            );
    }
    else
    {
        emit_msg(
            QString("Код отвратительный. "
                    "Качество: %1/100. Нужен рефакторинг.").arg(quality),
            "error"
            );
    }

    QStringList comments;
    if (quality < 70)
        comments << "— плохой нейминг переменных";
    if (quality < 60)
        comments << "— слишком большие классы";
    if (quality < 50)
        comments << "— нет тестов";
    if (quality < 40)
        comments << "— нет документации";
    if (quality < 30)
        comments << "— дублирование кода";
    if (m_state.burnout > 60)
        comments << "— хаотичная структура (видно что делал на износе)";

    for (const auto &c : comments)
        emit_msg(c, "text");

    if (hiddenBugs > 0)
    {
        emit_msg(
            QString("Найдено скрытых багов: %1. "
                    "Введи review fix — исправить, "
                    "review skip — пропустить.")
                .arg(hiddenBugs),
            "error"
            );
    }
    else
    {
        emit_msg("Скрытых багов не найдено.", "success");
        m_state.pendingReview = false;

        if (quality >= 80)
        {
            m_state.reputation += 2;
            emit_msg("+2 репутации за чистый код.", "success");
        }
    }
}

void GameController::updateMood(int delta)
{
    m_state.mood = qBound(0, m_state.mood + delta, m_state.maxMood);
}

void GameController::updateStress(int delta)
{
    m_state.stress = qBound(0, m_state.stress + delta, m_state.maxStress);
}

void GameController::updateTechDebt(int delta)
{
    m_state.techDebt = qBound(0, m_state.techDebt + delta, 100);
}

void GameController::applyCharacterInteractions()
{
    if (m_state.stress >= 80)
    {
        updateMood(-3);
        if (m_state.stress == 80)
            emit_msg("Стресс зашкаливает. Настроение падает.", "warning");
    }
    else if (m_state.stress >= 50)
    {
        updateMood(-1);
    }

    if (m_state.burnout >= 80)
        updateMood(-2);
    else if (m_state.burnout >= 60)
        updateMood(-1);

    if (m_state.mood <= 20)
    {
        updateStress(+3);
        if (m_state.mood == 20)
            emit_msg("Совсем нет желания работать.", "error");
    }
    else if (m_state.mood <= 40)
        updateStress(+1);

    if (m_state.sleepDebt >= 3)
    {
        updateMood(-2);
        updateStress(+2);
        updateBurnout(+1);
    }

    if (m_state.techDebt >= 70)
    {
        updateStress(+3);
        emit_msg(
            QString("Технический долг %1/100. "
                    "Код разваливается.").arg(m_state.techDebt),
            "error"
            );
    }
    else if (m_state.techDebt >= 40)
        updateStress(+1);

    if (m_state.mood >= 80 && m_state.stress > 0)
        updateStress(-1);

    if (m_state.stress >= 90 &&
        QRandomGenerator::global()->bounded(100) < 20)
    {
        int energyLoss = QRandomGenerator::global()->bounded(20, 41);
        m_state.energy = qMax(0, m_state.energy - energyLoss);
        updateBurnout(+5);
        emit_msg(
            QString("Паническая атака. -%1 энергии, выгорание +5.")
                .arg(energyLoss),
            "error"
            );
    }
}