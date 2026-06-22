#include "SkillSystem.h"
#include <QRandomGenerator>

double SkillSystem::successChance(const GameState &s) {
    double base = 0.3;
    double skillSum = 0;
    for (double v : s.skills) skillSum += v;
    double skillBonus = skillSum / 180.0;
    double repBonus   = qMin(0.2, s.reputation / 500.0);
    return qMin(0.98, base + skillBonus + repBonus);
}

double SkillSystem::reputationBonus(const GameState &s) {
    return 1.0 + (s.reputation / 50) * 0.1;
}

double SkillSystem::skillBonusMultiplier(const GameState &s) {
    return 1.0 + (s.level / 5) * 0.2;
}

int SkillSystem::learnCost(const GameState &s) {
    return qMax(100, 300 - (s.reputation / 10) * 20);
}
int SkillSystem::researchCost(const GameState &s) {
    return qMax(50,  200 - (s.reputation / 15) * 15);
}
int SkillSystem::mentorCost(const GameState &s) {
    return qMax(10,   50 - (s.reputation / 20) * 5);
}
int SkillSystem::deployCost(const GameState &s) {
    return qMax(25,  100 - (s.reputation / 25) * 10);
}
int SkillSystem::scaleCost(const GameState &s) {
    return qMax(40,  150 - (s.reputation / 30) * 15);
}
int SkillSystem::migrateCost(const GameState &s) {
    return qMax(30,  120 - (s.reputation / 35) * 12);
}
int SkillSystem::auditCost(const GameState &s) {
    return qMax(45,  180 - (s.reputation / 40) * 18);
}

QString SkillSystem::improveRandom(GameState &s, const QStringList &candidates,
                                    double minVal, double maxVal)
{
    QString skill = candidates[QRandomGenerator::global()->bounded(candidates.size())];
    double range  = maxVal - minVal;
    double gain   = minVal + QRandomGenerator::global()->generateDouble() * range;
    gain *= skillBonusMultiplier(s);
    s.skills[skill] += gain;
    return QString("%1 +%2").arg(skill).arg(gain, 0, 'f', 1);
}