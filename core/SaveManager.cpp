#include "SaveManager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include "CareerData.h"
#include "CareerTypes.h"

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

    root["mood"]     = state.mood;
    root["stress"]   = state.stress;
    root["techDebt"] = state.techDebt;

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
    root["daysWithoutRest"]         = state.daysWithoutRest;
    root["lastRestDay"]             = state.lastRestDay;
    root["lifeStage"]               = static_cast<int>(state.lifeStage);
    root["housingType"]             = static_cast<int>(state.housingType);
    root["equipmentTier"]           = static_cast<int>(state.equipment.tier);
    root["rentCost"]                = state.rentCost;
    root["foodCost"]                = state.foodCost;
    root["debt"]                    = state.debt;
    root["semesterDay"]             = state.semesterDay;
    root["studyScore"]              = state.studyScore;
    root["expelled"]                = state.expelled;
    root["missedRentCount"]         = state.missedRentCount;
    root["highBurnoutDays"]         = state.highBurnoutDays;
    root["sleepDebt"]               = state.sleepDebt;
    root["lastSleepDay"]            = state.lastSleepDay;
    root["sleptTonight"]            = state.sleptTonight;
    root["semester"]                = state.semester;
    root["academicDay"]             = state.academicDay;
    root["labsCompleted"]           = state.labsCompleted;
    root["labsRequired"]            = state.labsRequired;
    root["sessionActive"]           = state.sessionActive;
    root["sessionPassed"]           = state.sessionPassed;
    root["failedExams"]             = state.failedExams;
    root["pendingLab"]              = state.pendingLab;
    root["pendingLabSubject"]       = state.pendingLabSubject;
    QJsonArray scheduleArr;
    for (const auto &e : state.schedule)
    {
        QJsonObject obj;
        obj["type"]      = static_cast<int>(e.type);
        obj["day"]       = e.day;
        obj["hour"]      = e.hour;
        obj["subject"]   = e.subject;
        obj["attended"]  = e.attended;
        obj["completed"] = e.completed;
        scheduleArr.append(obj);
    }
    root["schedule"] = scheduleArr;
    root["foodStock"]       = state.foodStock;
    root["daysWithoutFood"] = state.daysWithoutFood;
    root["laundryPending"]  = state.laundryPending;
    root["laundryDays"]     = state.laundryDays;
    root["roomMessLevel"]   = state.roomMessLevel;
    root["daysIndoor"]      = state.daysIndoor;
    root["pendingReview"]     = state.pendingReview;
    root["reviewProjectName"] = state.reviewProjectName;
    root["reviewBugsFound"]   = state.reviewBugsFound;
    root["reviewQuality"]     = state.reviewQuality;
    root["prodIncidents"]     = state.prodIncidents;
    root["prodIgnored"]       = state.prodIgnored;
    root["jobMarketRefreshDay"]  = state.jobMarketRefreshDay;
    root["interviewsCooldown"]   = state.interviewsCooldown;
    root["consecutiveRefusals"]  = state.consecutiveRefusals;

    root["pendingHomework"]     = state.pendingHomework;
    root["homeworkSubject"]     = state.homeworkSubject;
    root["homeworkDeadline"]    = state.homeworkDeadline;
    root["homeworksMissed"]     = state.homeworksMissed;
    root["courseWorkActive"]    = state.courseWorkActive;
    root["courseWorkName"]      = state.courseWorkName;
    root["courseWorkProgress"]  = state.courseWorkProgress;
    root["courseWorkDeadline"]  = state.courseWorkDeadline;
    root["courseWorkSubmitted"] = state.courseWorkSubmitted;
    root["transportCost"]       = state.transportCost;
    root["hostingCost"]         = state.hostingCost;
    root["parentAllowance"]     = state.parentAllowance;
    root["grantAmount"]         = state.grantAmount;
    root["tutoringSessions"]    = state.tutoringSessions;

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

    state.level =
        root["level"].toInt();

    state.xp =
        root["xp"].toInt();

    state.money =
        root["money"].toInt();

    state.energy =
        root["energy"].toInt();

    state.reputation =
        root["reputation"].toInt();

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

    state.mood     = root["mood"].toInt(80);
    state.stress   = root["stress"].toInt(0);
    state.techDebt = root["techDebt"].toInt(0);

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

    state.daysWithoutRest =
        root["daysWithoutRest"].toInt(0);

    state.lastRestDay     =
        root["lastRestDay"].toInt(0);

    state.highBurnoutDays =
        root["highBurnoutDays"].toInt(0);

    state.sleepDebt    =
        root["sleepDebt"].toInt(0);

    state.lastSleepDay =
        root["lastSleepDay"].toInt(0);

    state.sleptTonight =
        root["sleptTonight"].toBool(false);

    state.lifeStage     =
        static_cast<LifeStage>(root["lifeStage"].toInt(0));
    state.housingType   =
        static_cast<HousingType>(root["housingType"].toInt(0));
    state.equipment     = CareerData::equipmentInfo(
        static_cast<EquipmentTier>(root["equipmentTier"].toInt(0)));
    state.rentCost      =
        root["rentCost"].toInt(3000);
    state.foodCost      =
        root["foodCost"].toInt(50);
    state.debt          =
        root["debt"].toInt(0);
    state.semesterDay   =
        root["semesterDay"].toInt(1);
    state.studyScore    =
        root["studyScore"].toInt(100);
    state.expelled      =
        root["expelled"].toBool(false);
    state.missedRentCount =
        root["missedRentCount"].toInt(0);

    {
        auto title = static_cast<JobTitle>(root["jobTitle"].toInt(1));
        state.currentJob = CareerData::jobInfo(title);
    }

    {
        auto type = static_cast<CompanyType>(root["companyType"].toInt(0));
        state.currentCompany = CareerData::companyInfo(type);
    }

    state.semester      =
        root["semester"].toInt(1);
    state.academicDay   =
        root["academicDay"].toInt(1);
    state.labsCompleted =
        root["labsCompleted"].toInt(0);
    state.labsRequired  =
        root["labsRequired"].toInt(3);
    state.sessionActive =
        root["sessionActive"].toBool(false);
    state.sessionPassed =
        root["sessionPassed"].toBool(false);
    state.failedExams   =
        root["failedExams"].toInt(0);
    state.pendingLab =
        root["pendingLab"].toBool(false);
    state.pendingLabSubject =
        root["pendingLabSubject"].toString();
    state.schedule.clear();
    QJsonArray scheduleArr = root["schedule"].toArray();
    for (const auto &val : scheduleArr)
    {
        QJsonObject obj = val.toObject();
        StudyEvent e;
        e.type      = static_cast<StudyEventType>(obj["type"].toInt());
        e.day       = obj["day"].toInt();
        e.hour      = obj["hour"].toInt();
        e.subject   = obj["subject"].toString();
        e.attended  = obj["attended"].toBool(false);
        e.completed = obj["completed"].toBool(false);
        state.schedule.append(e);
    }
    state.foodStock         = root["foodStock"].toInt(3);
    state.daysWithoutFood   = root["daysWithoutFood"].toInt(0);
    state.laundryPending    = root["laundryPending"].toBool(false);
    state.laundryDays       = root["laundryDays"].toInt(0);
    state.roomMessLevel     = root["roomMessLevel"].toInt(0);
    state.daysIndoor        = root["daysIndoor"].toInt(0);
    state.pendingReview     = root["pendingReview"].toBool(false);
    state.reviewProjectName = root["reviewProjectName"].toString();
    state.reviewBugsFound   = root["reviewBugsFound"].toInt(0);
    state.reviewQuality     = root["reviewQuality"].toInt(0);
    state.prodIncidents     = root["prodIncidents"].toInt(0);
    state.prodIgnored       = root["prodIgnored"].toInt(0);
    state.jobMarketRefreshDay = root["jobMarketRefreshDay"].toInt(0);
    state.interviewsCooldown  = root["interviewsCooldown"].toInt(0);
    state.consecutiveRefusals = root["consecutiveRefusals"].toInt(0);

    state.pendingHomework    = root["pendingHomework"].toBool(false);
    state.homeworkSubject    = root["homeworkSubject"].toString();
    state.homeworkDeadline   = root["homeworkDeadline"].toInt(0);
    state.homeworksMissed    = root["homeworksMissed"].toInt(0);
    state.courseWorkActive   = root["courseWorkActive"].toBool(false);
    state.courseWorkName     = root["courseWorkName"].toString();
    state.courseWorkProgress = root["courseWorkProgress"].toInt(0);
    state.courseWorkDeadline = root["courseWorkDeadline"].toInt(0);
    state.courseWorkSubmitted= root["courseWorkSubmitted"].toBool(false);
    state.transportCost      = root["transportCost"].toInt(40);
    state.hostingCost        = root["hostingCost"].toInt(0);
    state.parentAllowance    = root["parentAllowance"].toInt(0);
    state.grantAmount        = root["grantAmount"].toInt(0);
    state.tutoringSessions   = root["tutoringSessions"].toInt(0);

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