#include "SaveManager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include "CareerData.h"

static QJsonObject projectToJson(const Project& p)
{
    QJsonObject obj;
    obj["name"]        = p.name;
    obj["category"]    = p.category;
    obj["difficulty"]  = p.difficulty;
    obj["reward"]      = p.reward;
    obj["time"]        = p.time;
    obj["progress"]    = p.progress;
    obj["maxProgress"] = p.maxProgress;
    obj["bugs"]        = p.bugs;
    obj["deadlineDay"] = p.deadlineDay;
    obj["readyForDeploy"] = p.readyForDeploy;
    return obj;
}

static Project projectFromJson(const QJsonObject& obj)
{
    Project p;
    p.name           = obj["name"].toString();
    p.category       = obj["category"].toString();
    p.difficulty     = obj["difficulty"].toInt();
    p.reward         = obj["reward"].toInt();
    p.time           = obj["time"].toInt();
    p.progress       = obj["progress"].toInt();
    p.maxProgress    = obj["maxProgress"].toInt(100);
    p.bugs           = obj["bugs"].toInt();
    p.deadlineDay    = obj["deadlineDay"].toInt(0);
    p.readyForDeploy = obj["readyForDeploy"].toBool(false);
    return p;
}

bool SaveManager::save(const GameState& state)
{
    QJsonObject root;

    root["level"]                   = state.level;
    root["xp"]                      = state.xp;
    root["money"]                   = state.money;
    root["energy"]                  = state.energy;
    root["reputation"]              = state.reputation;
    root["maxEnergy"]               = state.maxEnergy;
    root["projectsCompleted"]       = state.projectsCompleted;
    root["bugsFixed"]               = state.bugsFixed;
    root["guiUnlocked"]             = state.guiUnlocked;
    root["gameDay"]                 = state.gameDay;
    root["gameHours"]               = state.gameHours;
    root["gameMinutes"]             = state.gameMinutes;
    root["burnout"]                 = state.burnout;
    root["maxBurnout"]              = state.maxBurnout;
    root["deadlinesMissed"]         = state.deadlinesMissed;
    root["deadlinesMet"]            = state.deadlinesMet;
    root["jobTitle"]                = static_cast<int>(state.currentJob.title);
    root["jobSalaryBonus"]          = state.currentJob.salaryBonus;
    root["jobSpeedBonus"]           = state.currentJob.workSpeedBonus;
    root["jobDisplayName"]          = state.currentJob.displayName;
    root["companyType"]             = static_cast<int>(state.currentCompany.type);
    root["companyName"]             = state.currentCompany.name;
    root["companySalaryPerDay"]     = state.currentCompany.salaryPerDay;
    root["companyDeadlineMult"]     = state.currentCompany.deadlineMultiplier;
    root["companyRepMult"]          = state.currentCompany.reputationMultiplier;

    root["maxConcurrentProjects"] =
        state.maxConcurrentProjects;

    root["maxQueueSize"] =
        state.maxQueueSize;

    QJsonObject skillsObj;

    for (auto it = state.skills.begin();
         it != state.skills.end();
         ++it)
    {
        skillsObj[it.key()] = it.value();
    }

    root["skills"] = skillsObj;

    QJsonArray activeProjects;

    for (const auto& p : state.currentProjects)
    {
        activeProjects.append(
            projectToJson(p)
            );
    }

    root["currentProjects"] =
        activeProjects;

    QJsonArray queuedProjects;

    for (const auto& p : state.projectQueue)
    {
        queuedProjects.append(
            projectToJson(p)
            );
    }

    root["projectQueue"] =
        queuedProjects;

    QFile file("save.json");

    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(
        QJsonDocument(root).toJson()
        );

    return true;
}

bool SaveManager::load(GameState& state)
{
    QFile file("save.json");

    if (!file.open(QIODevice::ReadOnly))
        return false;

    auto json =
        QJsonDocument::fromJson(
            file.readAll()
            );

    auto root = json.object();

    state.level = root["level"].toInt();
    state.xp = root["xp"].toInt();
    state.money = root["money"].toInt();
    state.energy = root["energy"].toInt();
    state.reputation = root["reputation"].toInt();

    state.maxEnergy =
        root["maxEnergy"].toInt(200);

    state.projectsCompleted =
        root["projectsCompleted"].toInt();

    state.bugsFixed =
        root["bugsFixed"].toInt();

    state.guiUnlocked =
        root["guiUnlocked"].toBool(false);

    state.gameDay =
        root["gameDay"].toInt(1);

    state.gameHours =
        root["gameHours"].toInt(8);

    state.gameMinutes =
        root["gameMinutes"].toInt(0);

    state.burnout    =
        root["burnout"].toInt(0);

    state.maxBurnout =
        root["maxBurnout"].toInt(100);

    state.deadlinesMissed =
        root["deadlinesMissed"].toInt(0);

    state.deadlinesMet    =
        root["deadlinesMet"].toInt(0);

    state.maxConcurrentProjects =
        root["maxConcurrentProjects"].toInt(1);

    state.maxQueueSize =
        root["maxQueueSize"].toInt(5);

    {
        auto title = static_cast<JobTitle>(root["jobTitle"].toInt(1));
        state.currentJob = CareerData::jobInfo(title);
    }

    {
        auto type = static_cast<CompanyType>(root["companyType"].toInt(0));
        state.currentCompany = CareerData::companyInfo(type);
    }

    state.skills.clear();

    QJsonObject skillsObj =
        root["skills"].toObject();

    for (auto it = skillsObj.begin();
         it != skillsObj.end();
         ++it)
    {
        state.skills[it.key()] =
            it.value().toDouble();
    }

    if (state.skills.isEmpty())
    {
        state.initSkills();
    }

    state.currentProjects.clear();

    QJsonArray activeProjects =
        root["currentProjects"].toArray();

    for (const auto &value : activeProjects)
    {
        state.currentProjects.enqueue(
            projectFromJson(
                value.toObject()
                )
            );
    }

    state.projectQueue.clear();

    QJsonArray queuedProjects =
        root["projectQueue"].toArray();

    for (const auto &value : queuedProjects)
    {
        state.projectQueue.enqueue(
            projectFromJson(
                value.toObject()
                )
            );
    }



    return true;


}