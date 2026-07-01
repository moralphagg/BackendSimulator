#include "CareerData.h"
#include "GameState.h"
#include <QRandomGenerator>

JobInfo CareerData::jobInfo(JobTitle title)
{
    switch (title) {
    case JobTitle::Unemployed:
        return {title, 0,    0,  0,  0.7,  "Безработный"};
    case JobTitle::Vibecoder:
        return {title, 0,    0,  0, 0.85,      "Vibecoder"};
    case JobTitle::Freelancer:
        return {title, 0,    0,  0,  0.9,  "Фрилансер"};
    case JobTitle::Intern:
        return {title, 100,  0,  1,  1.0,  "Стажёр"};
    case JobTitle::Junior:
        return {title, 250,  10, 3,  1.1,  "Junior"};
    case JobTitle::Middle:
        return {title, 500,  30, 7,  1.25, "Middle"};
    case JobTitle::Senior:
        return {title, 900,  60, 12, 1.4,  "Senior"};
    case JobTitle::Lead:
        return {title, 1400, 100,17, 1.55, "Lead"};
    case JobTitle::Architect:
        return {title, 2000, 150,22, 1.7,  "Architect"};
    case JobTitle::CTO:
        return {title, 3000, 200,27, 2.0,  "CTO"};
    case JobTitle::Founder:
        return {title, 5000, 250, 30, 2.5, "Founder"};
    default:
        return {title, 0, 0, 0, 1.0, "???"};
    }
}

CompanyInfo CareerData::companyInfo(CompanyType type)
{
    switch (type) {
    case CompanyType::Startup:
        return {type, "Стартап",      800,  1.3, 1.2,
                "Быстрый рост, жёсткие дедлайны, высокая репутация"};
    case CompanyType::Outsource:
        return {type, "Аутсорс",      600,  1.0, 0.9,
                "Стабильный поток задач, средние условия"};
    case CompanyType::Corporation:
        return {type, "Корпорация",   1200, 0.7, 1.0,
                "Высокая зарплата, мягкие дедлайны, медленный рост"};
    case CompanyType::Government:
        return {type, "Госкомпания",  700,  0.5, 0.8,
                "Очень мягкие дедлайны, низкая репутация"};
    case CompanyType::FAANG:
        return {type, "FAANG",        2500, 1.8, 1.5,
                "Огромная зарплата, жесточайшие дедлайны"};
    default:
        return {type, "Нет",          0,    1.0, 1.0, ""};
    }
}

JobTitle CareerData::nextTitle(const GameState &s)
{
    if (s.lifeStage == LifeStage::Student)
        return s.currentJob.title;

    struct Req { JobTitle t; int rep; int lvl; };
    static const QList<Req> reqs = {
                                    {JobTitle::Founder,   250, 30},
                                    {JobTitle::CTO,       200, 27},
                                    {JobTitle::Architect, 150, 22},
                                    {JobTitle::Lead,      100, 17},
                                    {JobTitle::Senior,    60,  12},
                                    {JobTitle::Middle,    30,  7 },
                                    {JobTitle::Junior,    10,  3 },
                                    {JobTitle::Intern,    0,   1 },
                                    {JobTitle::Vibecoder, 0,   0 },
                                    {JobTitle::Freelancer,0,   0 },
                                    };

    for (const auto &r : reqs) {
        if (s.reputation >= r.rep && s.level >= r.lvl)
            return r.t;
    }
    return JobTitle::Unemployed;
}

bool CareerData::canJoin(const GameState &s, CompanyType type)
{
    switch (type) {
    case CompanyType::Startup:
        return s.level >= 3;
    case CompanyType::Outsource:
        return s.level >= 2;
    case CompanyType::Corporation:
        return s.level >= 8  && s.reputation >= 50;
    case CompanyType::Government:
        return s.level >= 2;
    case CompanyType::FAANG:
        return s.level >= 20 && s.reputation >= 150;
    default:
        return false;
    }
}


Equipment CareerData::equipmentInfo(EquipmentTier tier)
{
    switch (tier) {
    case EquipmentTier::OldLaptop:
        return {tier, "Старый ноутбук",   0,      1.0,  200, 0.00};
    case EquipmentTier::NormalLaptop:
        return {tier, "Ноутбук",          15000,  1.2,  220, 0.05};
    case EquipmentTier::GamingPC:
        return {tier, "Игровой ПК",       45000,  1.45, 240, 0.10};
    case EquipmentTier::WorkStation:
        return {tier, "Рабочая станция",  120000, 1.8,  260, 0.20};
    default:
        return {tier, "???", 0, 1.0, 200, 0.0};
    }
}

QStringList CareerData::csSubjects()
{
    return {
        "Алгоритмы и структуры данных",
        "Базы данных",
        "Операционные системы",
        "Сети и протоколы",
        "Веб-разработка",
        "Математический анализ",
        "Линейная алгебра",
        "Теория вероятностей",
        "Архитектура ПО",
        "Английский язык"
    };
}
QList<JobOffer> CareerData::generateJobMarket(const GameState &s)
{
    QList<JobOffer> offers;
    auto rnd = [](int a, int b){
        return QRandomGenerator::global()->bounded(a, b);
    };

    offers.append({
        CompanyType::Outsource,
        JobTitle::Junior,
        650, 5, 2, 2.0,
        "Аутсорс: Junior-разработчик. "
        "Стабильно, скучновато.",
        s.gameDay + 7
    });

    if (s.level >= 3)
    {
        offers.append({
            CompanyType::Startup,
            JobTitle::Junior,
            900, 10, 3, 2.5,
            "Стартап: Junior. Быстрый рост, "
            "жёсткие дедлайны, equity.",
            s.gameDay + 5
        });
    }

    if (s.level >= 5)
    {
        offers.append({
            CompanyType::Outsource,
            JobTitle::Middle,
            1100, 25, 5, 3.5,
            "Аутсорс: Middle. "
            "Разнообразные проекты.",
            s.gameDay + 7
        });
    }

    if (s.level >= 8 && s.reputation >= 50)
    {
        offers.append({
            CompanyType::Corporation,
            JobTitle::Middle,
            1400, 50, 8, 4.0,
            "Корпорация: Middle. "
            "Высокая зарплата, медленный рост.",
            s.gameDay + 10
        });
    }

    if (s.level >= 8)
    {
        offers.append({
            CompanyType::Government,
            JobTitle::Middle,
            900, 30, 6, 3.0,
            "Госкомпания: Middle. "
            "Стабильность, мягкие дедлайны.",
            s.gameDay + 14
        });
    }

    if (s.level >= 12 && s.reputation >= 60)
    {
        offers.append({
            CompanyType::Startup,
            JobTitle::Senior,
            1800, 60, 12, 5.0,
            "Стартап: Senior. "
            "Высокая ответственность, большой рост.",
            s.gameDay + 5
        });
    }

    if (s.level >= 20 && s.reputation >= 150)
    {
        offers.append({
            CompanyType::FAANG,
            JobTitle::Senior,
            3000, 150, 20, 7.0,
            "FAANG: Senior. "
            "Мечта. Жесточайший отбор.",
            s.gameDay + 7
        });
    }

    return offers;
}