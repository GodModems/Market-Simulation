#include "ResourceMarket.h"
#include <iostream>
#include <random>

void updateResourcePrices(SimulationWorld &world) {
    // For example, adjust resource prices based on demand vs. a randomly generated supply.
    const int minSupply = 100;
    const int maxSupply = 1000;
    const float alpha = 0.1f; // sensitivity factor

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> supplyDist(minSupply, maxSupply);

    // Update each resource's price
    for (auto& res : world.resourceCatalog) {
        int totalDemand = 0;
        // Sum BUY orders for this resource
        for (const auto& order : world.market.orders) {
            if (order.productId == res.id && order.type == OrderType::BUY)
                totalDemand += order.amount;
        }
        int supply = supplyDist(gen);
        float ratio = (supply > 0) ? static_cast<float>(totalDemand) / supply : 0.0f;
        float newPrice = res.price * (1 + alpha * (ratio - 1));
        if (newPrice < 1.0f) newPrice = 1.0f;
        //std::cout << "Updating " << res.name << " (ID " << res.id
        //    << "): Demand = " << totalDemand
        //    << ", Supply = " << supply
        //    << ", New Price = " << newPrice << "\n";
        res.price = newPrice;
    }
}
