#include "Factory.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <thread>
#include <chrono>

int Factory::optimizeProduction() {
    // Example dummy optimization:
    // Assume product production requires 1 unit each of resource with id 1 and resource with id 2.
    int available1 = 0, available2 = 0;
    for (auto& item : inventory) {
        if (item.first.type == CommodityType::Resource) {
            if (item.first.id == 1)
                available1 = item.second;
            else if (item.first.id == 2)
                available2 = item.second;
        }
    }
    // The maximum producible units is the minimum of the two available resource amounts.
    return std::min(available1, available2);
}

void Factory::update(Market& market) {
    // --- Selling Logic ---
    // For each product in the inventory, if we have some finished products,
    // we try to place a sell order on the market.
    for (auto& item : inventory) {
        Commodity& commodity = item.first;
        int quantity = item.second;
        if (commodity.type == CommodityType::Product && quantity > 0) {
            // Decide to sell half (or at least 1 unit) of the available quantity.
            int sellAmount = std::max(1, quantity / 2);
            // For simplicity, use the commodity's price as the sell price.
            market.placeSellOrder(commodity.id, sellAmount, commodity.price, id);
            item.second -= sellAmount;
            // Assume an immediate sale in our simulation: update factory balance.
            balance += sellAmount * commodity.price;
            std::cout << "Factory " << id << " sold " << sellAmount
                << " units of product " << commodity.id
                << " at price " << commodity.price << "\n";
        }
    }

    // --- Production Logic ---
    // Example: If the factory has enough resources (say resource IDs 1 and 2),
    // it can produce a product (e.g., product ID 1000).
    int res1Index = -1, res2Index = -1;
    for (size_t i = 0; i < inventory.size(); ++i) {
        // Look for resource with id 1.
        if (inventory[i].first.type == CommodityType::Resource &&
            inventory[i].first.id == 1 && inventory[i].second > 0)
            res1Index = i;
        // And resource with id 2.
        if (inventory[i].first.type == CommodityType::Resource &&
            inventory[i].first.id == 2 && inventory[i].second > 0)
            res2Index = i;
    }

    if (res1Index != -1 && res2Index != -1) {
        // Consume one unit each of resource 1 and resource 2.
        inventory[res1Index].second--;
        inventory[res2Index].second--;
        // Produce one unit of a product (ID 1000).
        Commodity product;
        product.id = 1000;
        product.price = 150.0f;  // This might be computed dynamically.
        product.type = CommodityType::Product;
        // In a full simulation, you might fill in the production recipe.

        bool found = false;
        for (auto& item : inventory) {
            if (item.first.id == product.id && item.first.type == CommodityType::Product) {
                item.second++;
                found = true;
                break;
            }
        }
        if (!found) {
            inventory.push_back({ product, 1 });
        }
        std::cout << "Factory " << id << " produced product " << product.id << "\n";
    }

    // --- Buying Logic ---
    // If the factory has low inventory of a resource (e.g., resource id 1), try to buy more.
    for (auto& item : inventory) {
        Commodity& commodity = item.first;
        if (commodity.type == CommodityType::Resource && commodity.id == 1 && item.second < 5) {
            int buyAmount = 10; // Decide how much to buy.
            // Willing to pay a little above the current price.
            float maxPrice = commodity.price * 1.05f;
            market.placeBuyOrder(commodity.id, buyAmount, maxPrice, id);
            std::cout << "Factory " << id << " placed BUY order for resource " << commodity.id
                << " (amount " << buyAmount << ")\n";
        }
    }

    // Optionally, you could add more complex strategies here such as:
    // - Checking current market trends.
    // - Managing cash flow and risk.
    // - Updating equipment usage and maintenance.
}
