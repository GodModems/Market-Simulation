#pragma once
#include <vector>
#include <utility>
#include "Commodity.h"
#include "Market.h"

struct Factory {
    int id;
    float balance;
    std::vector<Equipment> equipment;
    // Inventory: a pair of Commodity and its quantity.
    std::vector<std::pair<Commodity, int>> inventory;

    // Added member function: returns the maximum number of products that can be produced
    // based on available resources. (Dummy implementation here.)
    int optimizeProduction();

    // Update function (for AI or other use)
    void update(Market& market);
};
