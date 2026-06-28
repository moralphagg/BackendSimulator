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

enum class LifeStage {
    Student,
    Freelancer,
    Employed
};

enum class HousingType {
    Dorm,
    Rent,
    Own
};

enum class EquipmentTier {
    OldLaptop,
    NormalLaptop,
    GamingPC,
    WorkStation
};

enum class StudyEventType {
    Lecture,
    Practice,
    Lab,
    Exam
};

struct StudyEvent {
    StudyEventType type;
    int     day;
    int     hour;
    QString subject;
    bool    attended = false;
    bool    completed = false;
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

struct Equipment {
    EquipmentTier tier        = EquipmentTier::OldLaptop;
    QString       name        = "Старый ноутбук";
    int           price       = 0;
    double        speedBonus  = 1.0;
    int           maxEnergy   = 200;
    double        bugReduction = 0.0;
};