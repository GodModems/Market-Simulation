#pragma once
#include "Initialization.h"  // Provides SimulationWorld, Factory, Market, Commodity, Equipment
#include <vector>

class AIController {
public:
    // Updated function: update an individual AI factory using the full simulation world.
    void updateFactory(SimulationWorld &world, Factory &factory);
};
