#pragma once
#include <QString>

enum class CompanyType {
    None,
    Startup,
    Outsource,
    Corporation,
    Government,
    FAANG
};

enum class JobTitle {
    Unemployed,
    Freelancer,
    Intern,
    Junior,
    Middle,
    Senior,
    Lead,
    Architect,
    CTO
};

struct JobInfo {
    JobTitle title          = JobTitle::Unemployed;
    int      salaryBonus    = 0;
    int      repRequired    = 0;
    int      levelRequired  = 0;
    double   workSpeedBonus = 1.0;
    QString  displayName;
};

struct CompanyInfo {
    CompanyType type                 = CompanyType::None;
    QString     name;
    int         salaryPerDay         = 0;
    double      deadlineMultiplier   = 1.0;
    double      reputationMultiplier = 1.0;
    QString     description;
};