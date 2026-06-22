#pragma once

#include "GameState.h"

class SaveManager
{
public:
    static bool save(const GameState& state);
    static bool load(GameState& state);
};