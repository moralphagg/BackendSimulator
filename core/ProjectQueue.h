#pragma once
#include "GameState.h"

class ProjectQueue {
public:
    static Project generateProject(int level);
    static void refill(GameState &state);
};