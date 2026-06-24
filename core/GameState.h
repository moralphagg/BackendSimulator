#pragma once
#include <QString>
#include <QMap>
#include <QQueue>
#include <QDateTime>
#include "CareerTypes.h"

struct Project
{
    QString name;

    QString category;

    int difficulty;
    int reward;
    int time;

    int progress = 0;
    int maxProgress = 100;

    int bugs = 0;

    bool readyForDeploy = false;

    int deadlineDay = 0;
};

struct GameState {
    int energy      = 200;
    int maxEnergy   = 200;
    int money       = 1500;

    int reputation  = 0;
    QMap<QString, int> categoryReputation;

    int level       = 1;
    int maxLevel    = 30;
    int xp          = 0;

    int gameDay = 1;
    int gameHours = 8;
    int gameMinutes = 0;

    int projectsCompleted = 0;
    int bugsFixed         = 0;

    int maxConcurrentProjects = 1;
    int maxQueueSize          = 5;

    QQueue<Project> currentProjects;
    QQueue<Project> projectQueue;

    QMap<QString, double> skills;

    bool guiUnlocked = false;

    int burnout    = 0;
    int maxBurnout = 100;

    JobInfo     currentJob;
    CompanyInfo currentCompany;

    int deadlinesMissed  = 0;
    int deadlinesMet     = 0;

    void initSkills() {
        const QStringList names = {
            "python","sql","api_design","debugging","testing",
            "deployment","docker","aws","ml","security",
            "kubernetes","redis","nginx","ci_cd","graphql",
            "microservices","serverless","elasticsearch","kafka","grpc"
        };

        categoryReputation["web"] = 0;
        categoryReputation["database"] = 0;
        categoryReputation["microservice"] = 0;
        categoryReputation["devops"] = 0;
        categoryReputation["cloud"] = 0;
        categoryReputation["ai"] = 0;
        categoryReputation["security"] = 0;
        categoryReputation["distributed"] = 0;

        for (const QString &n : names)
            skills[n] = 1.0;
    }
};