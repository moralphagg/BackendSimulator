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
    else if (cmd == "deploy")         result = cmdDeploy();
    else if (cmd == "optimize")       result = cmdOptimize();
    else if (cmd == "meeting")        result = cmdMeeting();
    else if (cmd == "research")       result = cmdResearch();
    else if (cmd == "mentor")         result = cmdMentor();
    else if (cmd == "freelance")      result = cmdFreelance();
    else if (cmd == "refactor")       result = cmdRefactor();
    else if (cmd == "document")       result = cmdDocument();
    else if (cmd == "analyze")        result = cmdAnalyze();
    else if (cmd == "scale")          result = cmdScale();
    else if (cmd == "migrate")        result = cmdMigrate();
    else if (cmd == "audit")          result = cmdAudit();
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
            m_state.energy = qMax(0, m_state.energy - 40);
            m_state.reputation = qMin(200, m_state.reputation + 5);
            updateBurnout(+8);
            emit_msg("Починил прод. +5 репутации, -40 энергии.", "success");
        }
        else if (cmd == "ignore")
        {
            m_state.pendingIncident = false;
            m_state.reputation = qMax(0, m_state.reputation - 10);
            emit_msg("Проигнорировал. -10 репутации.", "error");
        }
        else
        {
            emit_msg("Прод горит! Сначала введи fix или ignore.", "error");
            return;
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
                               * sleepPenalty) - p.difficulty);
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

    return {
        QString(
            "Фриланс-заказ выполнен! +%1 денег, +%2 опыта"
            )
            .arg(reward)
            .arg(xp),
        "success"
    };
}

QPair<QString,QString> GameController::cmdRefactor() {
    if (m_state.energy < 20) return {"Слишком устал для рефакторинга!", "warning"};
    QString info = SkillSystem::improveRandom(m_state, {"python","debugging","api_design"}, 0.2, 0.4);
    m_state.xp     += 10;
    m_state.energy -= 20;
    advanceTime(150);
    updateBurnout(+3);
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

QPair<QString,QString> GameController::cmdBuy(const QString &item)
{
    auto withNDS = [&](int price) {
        return int(price * (1.0f + m_state.ndsRate));
    };

    if (item == "food")
    {
        int cost = withNDS(500);
        if (m_state.money < cost)
            return {QString("Нет денег. Нужно %1").arg(cost), "error"};
        m_state.money -= cost;
        m_state.energy = qMin(m_state.maxEnergy, m_state.energy + 30);
        return {QString("Купил еды (+30 энергии, -%1 с НДС)").arg(cost), "success"};
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
        }
        else if (daysLeft == 1)
        {
            emit_msg(
                QString("Завтра дедлайн: %1! Торопись.").arg(p.name),
                "warning"
                );
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
                QString("✗ Дедлайн провален: %1\n"
                        "  -%2 репутации, -%3 денег, выгорание +10")
                    .arg(p.name)
                    .arg(repLoss)
                    .arg(moneyLoss),
                "error"
                );
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

    checkGameOver();

    generateDayEvents();
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
        emit_msg("Введи fix — починить (−40 энергии, +репутация)", "warning");
        emit_msg("Введи ignore — игнорировать (−репутация, штраф)", "warning");
        m_state.pendingIncident = true;
        break;

    case EventType::InternetBill:
        if (m_state.money >= 500)
        {
            m_state.money -= 500;
            emit_msg("Оплачено: -500 за интернет.", "text");
        }
        else
        {
            emit_msg("Нет денег на интернет! Работаешь медленнее.", "error");
            m_state.sleepDebt += 2; // штраф — не можешь нормально работать
        }
        break;

    case EventType::HungryAlert:
        m_state.energy = qMax(0, m_state.energy - 10);
        break;

    default:
        break;
    }

    emit stateChanged();
}