#include "CheatEngine.h"

struct CheatEntry { QString code; QString(*fn)(GameState&); };

QString CheatEngine::check(GameState &state, const QString &input)
{
    static const QList<CheatEntry> cheats = {
        {"sudo startx -aob",         &CheatEngine::activateGui},
        {"sudo rn use by redbull",   &CheatEngine::cheatEnergy},
        {"sudo -i --m pvldrv",       &CheatEngine::cheatMoney},
        {"sudo rm -rf /*",           &CheatEngine::cheatSkills},
        {"sudo make me a sandwich",  &CheatEngine::cheatReputation},
    };

    QString lower = input.trimmed().toLower();
    for (const CheatEntry &c : cheats)
        if (lower == c.code)
            return c.fn(state);
    return {};
}

QString CheatEngine::activateGui(GameState &s) {
    s.guiUnlocked = true;
    return "Графический интерфейс активирован.";
}
QString CheatEngine::cheatEnergy(GameState &s) {
    s.energy    += 10000;
    s.maxEnergy += 10000;
    return "+10000 энергии";
}
QString CheatEngine::cheatMoney(GameState &s) {
    s.money += 100000;
    return "+100000 денег";
}
QString CheatEngine::cheatSkills(GameState &s) {
    for (auto &v : s.skills) v += 5.0;
    return "все навыки +5.0";
}
QString CheatEngine::cheatReputation(GameState &s) {
    s.reputation += 100;
    return "+100 репутации";
}