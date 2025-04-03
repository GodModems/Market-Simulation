#include "Initialization.h"
#include <iostream>
#include <random>
#include <sstream>
#include <algorithm>

// You can adjust these constants as needed.
const int NUM_RESOURCES = 20;
const int NUM_PRODUCTS = 10;
const int NUM_EQUIPMENTS = 5;
const int NUM_AI_FACTORIES = 6;

SimulationWorld initializeSimulation() {
    SimulationWorld world;

    // Random engine setup.
    std::random_device rd;
    std::mt19937 gen(rd());

    // --- Generate Resource Catalog ---
    // Resources: commodity type Resource, no recipe.
    std::uniform_real_distribution<float> resourcePriceDist(4.0f, 150.0f);
    for (int i = 0; i < NUM_RESOURCES; i++) {
        Commodity res;
        res.id = i + 1; // IDs 1 .. NUM_RESOURCES.
        std::stringstream ss;
        ss << "Resource " << res.id;
        res.name = ss.str();
        res.price = resourcePriceDist(gen);
        res.type = CommodityType::Resource;
        // No recipe or equipment requirements for raw resources.
        world.resourceCatalog.push_back(res);
    }

    // --- Generate Equipment Catalog ---
    std::uniform_real_distribution<float> equipPriceDist(300.0f, 1000.0f);
    std::uniform_int_distribution<int> outputRateDist(1, 10);
    std::uniform_real_distribution<float> operationalCostDist(10.0f, 50.0f);
    for (int i = 0; i < NUM_EQUIPMENTS; i++) {
        Equipment equip;
        equip.id = i + 1; // Equipment IDs: 1..NUM_EQUIPMENTS.
        equip.price = equipPriceDist(gen);
        equip.output_rate = outputRateDist(gen);
        equip.operational_cost = operationalCostDist(gen);
        world.equipmentCatalog.push_back(equip);
    }

    // --- Generate Product Catalog ---
    // Products: commodity type Product with a recipe and equipment requirements.
    std::uniform_real_distribution<float> productPriceDist(75.0f, 700.0f);
    std::uniform_int_distribution<int> recipeCountDist(1, 7);              // How many resource ingredients.
    std::uniform_int_distribution<int> recipeQtyDist(1, 10);                // Quantity required for each.
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        Commodity prod;
        prod.id = NUM_RESOURCES + i + 1; // Product IDs start after resources.
        std::stringstream ss;
        ss << "Product " << prod.id;
        prod.name = ss.str();
        prod.price = productPriceDist(gen);
        prod.type = CommodityType::Product;

        // Generate a random recipe: randomly select resources from the resourceCatalog.
        int numIngredients = recipeCountDist(gen);
        std::vector<int> used;
        for (int j = 0; j < numIngredients; j++) {
            std::uniform_int_distribution<int> resourceIndexDist(0, world.resourceCatalog.size() - 1);
            int idx = resourceIndexDist(gen);
            int resourceId = world.resourceCatalog[idx].id;
            if (std::find(used.begin(), used.end(), resourceId) == used.end()) {
                int qty = recipeQtyDist(gen);
                prod.recipe.push_back({ resourceId, qty });
                used.push_back(resourceId);
            }
        }

        // Generate random equipment requirements.
        std::uniform_int_distribution<int> equipCountDist(1, NUM_EQUIPMENTS);
        int numEquipReq = equipCountDist(gen);
        std::vector<int> usedEquip;
        for (int j = 0; j < numEquipReq; j++) {
            std::uniform_int_distribution<int> equipIndexDist(0, world.equipmentCatalog.size() - 1);
            int idx = equipIndexDist(gen);
            int equipId = world.equipmentCatalog[idx].id;
            if (std::find(usedEquip.begin(), usedEquip.end(), equipId) == usedEquip.end()) {
                std::uniform_int_distribution<int> equipQtyDist(1, 3);
                int qty = equipQtyDist(gen);
                prod.requiredEquipment.push_back({ equipId, qty });
                usedEquip.push_back(equipId);
            }
        }
        world.productCatalog.push_back(prod);
    }

    // --- Initialize Player Factory ---
    world.playerFactory.id = 1;
    world.playerFactory.balance = 1000.0f;
    // Give player a fixed starting amount of each resource.
    for (const auto& res : world.resourceCatalog) {
        world.playerFactory.inventory.push_back({ res, 10 });
    }

    // --- Generate AI Factories ---
    std::uniform_int_distribution<int> inventoryDist(5, 15);
    for (int i = 0; i < NUM_AI_FACTORIES; i++) {
        Factory aiFactory;
        aiFactory.id = 2 + i;  // IDs: 2, 3, ...
        aiFactory.balance = 1000.0f;
        // Give each AI factory a random amount of each resource.
        for (const auto& res : world.resourceCatalog) {
            int qty = inventoryDist(gen);
            aiFactory.inventory.push_back({ res, qty });
        }
        world.aiFactories.push_back(aiFactory);
    }

    std::cout << "Simulation initialized with:\n"
        << NUM_RESOURCES << " resources,\n"
        << NUM_PRODUCTS << " products,\n"
        << NUM_EQUIPMENTS << " equipment types,\n"
        << NUM_AI_FACTORIES << " AI factories.\n";

    return world;
}
