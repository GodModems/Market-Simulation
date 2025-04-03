#pragma once
#include <vector>
#include "Market.h"
#include "Factory.h"
#include "Commodity.h"

// Updated SimulationWorld with catalogs for resources, products, and equipment.
struct SimulationWorld {
    Market market;
    Factory playerFactory;
    std::vector<Factory> aiFactories;
    std::vector<Commodity> productCatalog;   // Manufacturable products.
    std::vector<Commodity> resourceCatalog;  // Raw resources.
    std::vector<Equipment> equipmentCatalog; // Equipment types.
};

SimulationWorld initializeSimulation();
