#include "PlayerController.h"
#include "Initialization.h"  // For SimulationWorld, Factory, Market, Commodity, and Equipment
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <set>
#include <cctype>

// Helper function: View Product Market Overview
static void viewProductMarket(const Market& market, const std::vector<Commodity>& productCatalog) {
    std::cout << "\n--- Product Market Overview ---\n";
    std::cout << std::left << std::setw(15) << "Product"
        << " | " << std::right << std::setw(12) << "Best BUY"
        << " | " << std::setw(12) << "Best SELL" << "\n";
    std::cout << std::string(50, '-') << "\n";

    // For each product, calculate best buy and best sell.
    for (const auto& prod : productCatalog) {
        float bestBuyPrice = 0.0f;
        int bestBuyQty = 0;
        // For BUY orders: highest price wins.
        for (const auto& order : market.orders) {
            if (order.productId == prod.id && order.type == OrderType::BUY) {
                if (order.price > bestBuyPrice) {
                    bestBuyPrice = order.price;
                    bestBuyQty = order.amount;
                }
                else if (order.price == bestBuyPrice) {
                    bestBuyQty += order.amount;
                }
            }
        }

        float bestSellPrice = 0.0f;
        int bestSellQty = 0;
        bool foundSell = false;
        // For SELL orders: lowest price wins.
        for (const auto& order : market.orders) {
            if (order.productId == prod.id && order.type == OrderType::SELL) {
                if (!foundSell || order.price < bestSellPrice) {
                    bestSellPrice = order.price;
                    bestSellQty = order.amount;
                    foundSell = true;
                }
                else if (order.price == bestSellPrice) {
                    bestSellQty += order.amount;
                }
            }
        }
        if (!foundSell) {
            bestSellPrice = 0.0f;
            bestSellQty = 0;
        }

        std::cout << std::left << std::setw(15) << prod.name
            << " | " << std::right << std::setw(6) << bestBuyPrice << " (" << std::setw(3) << bestBuyQty << ")"
            << " | " << std::setw(6) << bestSellPrice << " (" << std::setw(3) << bestSellQty << ")\n";
    }
}

// Helper function: View Resource Market Overview
static void viewResourceMarket(const Market& market, const std::vector<Commodity>& resourceCatalog) {
    std::cout << "\n--- Resource Market Overview ---\n";
    std::cout << std::left << std::setw(15) << "Resource"
        << " | " << std::right << std::setw(12) << "Best SELL" << "\n";
    std::cout << std::string(30, '-') << "\n";

    // For each resource, find the best (lowest) SELL order.
    for (const auto& res : resourceCatalog) {
        float bestSellPrice = 0.0f;
        int bestSellQty = 0;
        bool foundSell = false;
        for (const auto& order : market.orders) {
            if (order.productId == res.id && order.type == OrderType::SELL) {
                if (!foundSell || order.price < bestSellPrice) {
                    bestSellPrice = order.price;
                    bestSellQty = order.amount;
                    foundSell = true;
                }
                else if (order.price == bestSellPrice) {
                    bestSellQty += order.amount;
                }
            }
        }
        if (!foundSell) {
            bestSellPrice = 0.0f;
            bestSellQty = 0;
        }
        std::cout << std::left << std::setw(15) << res.name
            << " | " << std::right << std::setw(6) << bestSellPrice
            << " (" << std::setw(3) << bestSellQty << ")\n";
    }
}


// Helper function: View Equipment Catalog
static void viewEquipmentCatalog(const std::vector<Equipment>& equipCatalog) {
    std::cout << "\n--- Equipment Catalog ---\n";
    std::cout << std::left << std::setw(12) << "Equip. ID"
        << " | " << std::setw(12) << "Price"
        << " | " << std::setw(12) << "Output Rate"
        << " | " << std::setw(18) << "Operational Cost" << "\n";
    std::cout << std::string(60, '-') << "\n";
    for (const auto& equip : equipCatalog) {
        std::cout << std::left << std::setw(12) << equip.id
            << " | " << std::right << std::setw(10) << equip.price << "  "
            << " | " << std::setw(12) << equip.output_rate
            << " | " << std::setw(16) << equip.operational_cost << "\n";
    }
}

// Helper function: View full Product Catalog (with recipes and equipment requirements)
static void viewProductCatalog(const std::vector<Commodity>& productCatalog) {
    std::cout << "\n--- Product Catalog ---\n";
    for (const auto& prod : productCatalog) {
        std::cout << "Product: " << prod.name
            << " (ID: " << prod.id
            << ", Price: " << prod.price << ")\n";
        std::cout << "  Resources:\n";
        if (prod.recipe.empty()) {
            std::cout << "    None\n";
        }
        else {
            for (const auto& req : prod.recipe) {
                std::cout << "    Resource " << std::setw(3) << req.first
                    << "  x " << std::setw(2) << req.second << "\n";
            }
        }
        std::cout << "  Equipment:\n";
        if (prod.requiredEquipment.empty()) {
            std::cout << "    None\n";
        }
        else {
            for (const auto& equip : prod.requiredEquipment) {
                std::cout << "    Equipment " << std::setw(3) << equip.first
                    << "  x " << std::setw(2) << equip.second << "\n";
            }
        }
        std::cout << "\n";
    }
}

// Helper function: View all orders for a specific commodity.
static void viewMarketOrdersForCommodity(const Market& market, int commodityId) {
    std::cout << "\n--- Market Orders for Commodity " << commodityId << " ---\n";
    for (const auto& order : market.orders) {
        if (order.productId == commodityId) {
            std::cout << "Order ID " << order.id
                << ", " << (order.type == OrderType::BUY ? "BUY" : "SELL")
                << ", Price: " << order.price
                << ", Amount: " << order.amount << "\n";
        }
    }
}

// Helper function: Handle buying a commodity.
static void buyCommodity(Factory& player, Market& market) {
    int commodityId, amount;
    float maxPrice;
    char fullPurchase;

    std::cout << "Enter commodity ID, desired amount, and maximum price: ";
    std::cin >> commodityId >> amount >> maxPrice;
    std::cout << "Full purchase only? (y/n): ";
    std::cin >> fullPurchase;

    // If full purchase is required, check that enough supply is available.
    if (std::toupper(fullPurchase) == 'Y') {
        int available = 0;
        for (const auto& order : market.orders) {
            if (order.productId == commodityId && order.type == OrderType::SELL && order.price <= maxPrice)
                available += order.amount;
        }
        if (available < amount) {
            std::cout << "Not enough available supply at or below your max price. Order not placed.\n";
            return;
        }
    }

    // Place the buy order.
    market.placeBuyOrder(commodityId, amount, maxPrice, player.id);

    // For this simulation, assume instant full execution of the buy order.
    // Update the player's inventory by adding exactly 'amount' units.
    bool found = false;
    for (auto& item : player.inventory) {
        if (item.first.id == commodityId) {
            item.second += amount;
            found = true;
            break;
        }
    }
    if (!found) {
        // If the commodity isn't already in inventory, create a new entry.
        // (In a full implementation, you'd look up the commodity details from a catalog.)
        Commodity commodity;
        commodity.id = commodityId;
        commodity.name = "Commodity " + std::to_string(commodityId);
        // Assume it's a Resource (or set type appropriately).
        commodity.type = CommodityType::Resource;
        player.inventory.push_back({ commodity, amount });
    }
    std::cout << "Purchased " << amount << " units of commodity " << commodityId << ".\n";
}

// Helper function: Handle selling a commodity.
static void sellCommodity(Factory& player, Market& market) {
    int commodityId, amount;
    float price;

    std::cout << "Enter commodity ID, amount, and price: ";
    std::cin >> commodityId >> amount >> price;

    // Check if the player has enough of the commodity.
    for (auto& item : player.inventory) {
        if (item.first.id == commodityId) {
            if (item.second < amount) {
                std::cout << "Insufficient quantity in inventory. Order not placed.\n";
                return;
            }
            // Remove the entire 'amount' from inventory immediately.
            item.second -= amount;
            break;
        }
    }

    // Place the sell order.
    market.placeSellOrder(commodityId, amount, price, player.id);
    std::cout << "Placed SELL order for " << amount << " units of commodity " << commodityId << ".\n";
}


// Helper function: Purchase equipment from the catalog.
static void purchaseEquipment(Factory& player, const std::vector<Equipment>& equipCatalog) {
    viewEquipmentCatalog(equipCatalog);
    int equipId, qty;
    std::cout << "Enter Equipment ID to purchase: ";
    std::cin >> equipId;
    std::cout << "Enter quantity to purchase: ";
    std::cin >> qty;

    // Look up the equipment in the catalog.
    const Equipment* selectedEquip = nullptr;
    for (const auto& equip : equipCatalog) {
        if (equip.id == equipId) {
            selectedEquip = &equip;
            break;
        }
    }
    if (!selectedEquip) {
        std::cout << "Equipment ID not found.\n";
        return;
    }

    float totalCost = selectedEquip->price * qty;
    if (player.balance < totalCost) {
        std::cout << "Insufficient balance to purchase " << qty << " units of Equipment " << equipId << ".\n";
        return;
    }

    player.balance -= totalCost;
    // Add the equipment units to the player's inventory.
    for (int i = 0; i < qty; i++) {
        player.equipment.push_back(*selectedEquip);
    }
    std::cout << "Purchased " << qty << " units of Equipment " << equipId << " for a total of " << totalCost << ".\n";
}

// Helper function: View the player's inventory.
static void viewInventory(const Factory& factory) {
    std::cout << "\n--- Inventory for Factory " << factory.id << " ---\n";

    // Resources Section
    std::cout << "\nResources:\n";
    bool hasResources = false;
    for (const auto& item : factory.inventory) {
        if (item.first.type == CommodityType::Resource) {
            std::cout << item.first.id
                << " (" << item.first.name << ") - Quantity: "
                << item.second << "\n";
            hasResources = true;
        }
    }
    if (!hasResources) {
        std::cout << "  None\n";
    }

    // Products Section
    std::cout << "\nProducts:\n";
    bool hasProducts = false;
    for (const auto& item : factory.inventory) {
        if (item.first.type == CommodityType::Product) {
            std::cout << item.first.id
                << " (" << item.first.name << ") - Quantity: "
                << item.second << "\n";
            hasProducts = true;
        }
    }
    if (!hasProducts) {
        std::cout << "  None\n";
    }

    // Equipment Section
    std::cout << "\nEquipment Owned (" << factory.equipment.size() << "):\n";
    if (factory.equipment.empty()) {
        std::cout << "  None\n";
    }
    else {
        for (size_t i = 0; i < factory.equipment.size(); i++) {
            const auto& equip = factory.equipment[i];
            std::cout << equip.id
                << " | Output Rate: " << std::setw(3) << equip.output_rate
                << " | Operational Cost: " << std::setw(6) << equip.operational_cost
                << " | Price: " << std::setw(6) << equip.price << "\n";
        }
    }
}

// Helper function: Handle production of a product.
static void produceProduct(Factory& player, const std::vector<Commodity>& productCatalog) {
    int productId;
    std::cout << "Enter the product ID you want to produce: ";
    std::cin >> productId;

    const Commodity* chosenProduct = nullptr;
    for (const auto& prod : productCatalog) {
        if (prod.id == productId) {
            chosenProduct = &prod;
            break;
        }
    }
    if (!chosenProduct) {
        std::cout << "Product not found.\n";
        return;
    }
    if (chosenProduct->recipe.empty()) {
        std::cout << "This product is not manufacturable (no recipe defined).\n";
        return;
    }

    int requestedAmount;
    std::cout << "Enter desired production amount: ";
    std::cin >> requestedAmount;

    // Determine maximum production possible based on available resources.
    int maxProductionByResources = requestedAmount;
    for (const auto& req : chosenProduct->recipe) {
        int resourceId = req.first;
        int requiredPerUnit = req.second;
        int available = 0;
        for (const auto& item : player.inventory) {
            if (item.first.id == resourceId && item.first.type == CommodityType::Resource) {
                available = item.second;
                break;
            }
        }
        int possibleForThisResource = available / requiredPerUnit;
        if (possibleForThisResource < maxProductionByResources)
            maxProductionByResources = possibleForThisResource;
    }

    if (maxProductionByResources <= 0) {
        std::cout << "Insufficient resources for production.\n";
        return;
    }

    // Equipment capacity: sum of output rates of all owned equipment.
    int equipmentCapacity = 0;
    float totalOperationalCost = 0.0f;
    for (const auto& equip : player.equipment) {
        equipmentCapacity += equip.output_rate;
        totalOperationalCost += equip.operational_cost;
    }
    if (equipmentCapacity <= 0) {
        std::cout << "No equipment available for production.\n";
        return;
    }

    int producibleAmount = std::min({ requestedAmount, maxProductionByResources, equipmentCapacity });
    if (player.balance < totalOperationalCost) {
        std::cout << "Insufficient balance to cover equipment operating costs.\n";
        return;
    }

    // Deduct required resources based on the product recipe.
    for (const auto& req : chosenProduct->recipe) {
        int resourceId = req.first;
        int totalRequired = producibleAmount * req.second;
        for (auto& item : player.inventory) {
            if (item.first.id == resourceId && item.first.type == CommodityType::Resource) {
                item.second -= totalRequired;
                break;
            }
        }
    }

    // Deduct equipment operating cost.
    player.balance -= totalOperationalCost;

    // Add produced product to inventory.
    bool found = false;
    for (auto& item : player.inventory) {
        if (item.first.id == chosenProduct->id && item.first.type == CommodityType::Product) {
            item.second += producibleAmount;
            found = true;
            break;
        }
    }
    if (!found) {
        Commodity product = *chosenProduct; // Copy product definition.
        player.inventory.push_back({ product, producibleAmount });
    }

    std::cout << "Produced " << producibleAmount << " units of " << chosenProduct->name << ".\n";
}

// The player's turn function now takes the entire SimulationWorld.
void PlayerController::takeTurn(SimulationWorld& world) {
    // Extract components from the world.
    Factory& player = world.playerFactory;
    Market& market = world.market;
    const std::vector<Commodity>& productCatalog = world.productCatalog;
    const std::vector<Commodity>& resourceCatalog = world.resourceCatalog;
    const std::vector<Equipment>& equipCatalog = world.equipmentCatalog;

    bool turnOver = false;
    while (!turnOver) {
        std::cout << "\n--- Player Turn (Factory " << player.id << ") ---\n";
        std::cout << "Balance: " << player.balance << "\n";
        std::cout << "Select an action:\n";
        std::cout << "  1. View Product Market\n";
        std::cout << "  2. View Resource Market\n";
        std::cout << "  3. View Orders for Specific Commodity\n";
        std::cout << "  4. Buy Commodity\n";
        std::cout << "  5. Sell Commodity\n";
        std::cout << "  6. View Inventory\n";
        std::cout << "  7. Buy Equipment\n";
        std::cout << "  8. View Product Catalog\n";
        std::cout << "  9. Produce Product\n";
        std::cout << " 10. End Turn\n";
        std::cout << "Enter command number: ";
        int choice;
        std::cin >> choice;

        switch (choice) {
        case 1:
            viewProductMarket(market, productCatalog);
            break;
        case 2:
            viewResourceMarket(market, resourceCatalog);
            break;
        case 3: {
            int commodityId;
            std::cout << "Enter commodity ID to view orders: ";
            std::cin >> commodityId;
            viewMarketOrdersForCommodity(market, commodityId);
            break;
        }
        case 4:
            buyCommodity(player, market);
            break;
        case 5:
            sellCommodity(player, market);
            break;
        case 6:
            viewInventory(player);
            break;
        case 7:
            purchaseEquipment(player, equipCatalog);
            break;
        case 8:
            viewProductCatalog(productCatalog);
            break;
        case 9:
            produceProduct(player, productCatalog);
            break;
        case 10:
            turnOver = true;
            break;
        default:
            std::cout << "Invalid command. Try again.\n";
        }
    }
}
