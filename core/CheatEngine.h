#pragma once
#include "GameState.h"

class CheatEngine {
public:
    // Возвращает сообщение если чит-код сработал, иначе пустую строку
    static QString check(GameState &state, const QString &input);

private:
    static QString activateGui(GameState &s);
    static QString cheatEnergy(GameState &s);
    static QString cheatMoney(GameState &s);
    static QString cheatSkills(GameState &s);
    static QString cheatReputation(GameState &s);
};