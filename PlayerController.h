#pragma once
#include "Initialization.h"  // Provides SimulationWorld
#include <vector>

class PlayerController {
public:
    // Runs the player's turn using the complete simulation world.
    void takeTurn(SimulationWorld &world);
};
