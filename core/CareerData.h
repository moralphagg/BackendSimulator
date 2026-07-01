#pragma once
#include "CareerTypes.h"

struct GameState;

class CareerData {
public:
    static JobInfo     jobInfo(JobTitle title);
    static CompanyInfo companyInfo(CompanyType type);
    static JobTitle    nextTitle(const GameState &s);
    static bool        canJoin(const GameState &s, CompanyType type);
    static Equipment equipmentInfo(EquipmentTier tier);
    static QStringList csSubjects();
    static QList<JobOffer> generateJobMarket(const GameState &s);
};