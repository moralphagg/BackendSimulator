#pragma once
#include "GameState.h"

class SkillSystem {
public:
    static double  successChance(const GameState &s);
    static double  reputationBonus(const GameState &s);
    static double  skillBonusMultiplier(const GameState &s);

    static int learnCost(const GameState &s);
    static int researchCost(const GameState &s);
    static int mentorCost(const GameState &s);
    static int deployCost(const GameState &s);
    static int scaleCost(const GameState &s);
    static int migrateCost(const GameState &s);
    static int auditCost(const GameState &s);

    static QString improveRandom(GameState &s, const QStringList &candidates, double min, double max);
};