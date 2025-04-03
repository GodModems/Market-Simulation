#include "AIController.h"
#include "SimplexAlgorithm.h"  // Declares the Simplex class.
#include <iostream>
#include <climits>
#include <algorithm>
#include <unordered_map>
#include <vector>

void AIController::updateFactory(SimulationWorld& world, Factory& factory) {
    // Extract components from the world.
    Market& market = world.market;
    const std::vector<Commodity>& productCatalog = world.productCatalog;
    const std::vector<Commodity>& resourceCatalog = world.resourceCatalog;
    const std::vector<Equipment>& equipCatalog = world.equipmentCatalog;

    std::cout << "\n[AI Factory " << factory.id << " Turn]\n";

    if (productCatalog.empty() || resourceCatalog.empty()) {
        std::cout << "No products or resources available for production.\n";
        return;
    }

    // Build a map of available resource quantities from the factory's inventory.
    std::unordered_map<int, int> resourceAvail;
    for (const auto& res : resourceCatalog) {
        for (const auto& item : factory.inventory) {
            if (item.first.id == res.id && item.first.type == CommodityType::Resource) {
                resourceAvail[res.id] = item.second;
                break;
            }
        }
    }

    // Calculate current equipment capacity: sum of output rates of all owned equipment.
    int equipmentCapacity = 0;
    for (const auto& equip : factory.equipment) {
        equipmentCapacity += equip.output_rate;
    }

    // --- Construct the Linear Program ---
    // Decision variables: production quantities for each product in productCatalog.
    int numProducts = productCatalog.size();

    // Identify which resources are used in at least one product's recipe.
    std::vector<int> resourcesUsed;
    for (const auto& res : resourceCatalog) {
        bool used = false;
        for (const auto& prod : productCatalog) {
            for (const auto& req : prod.recipe) {
                if (req.first == res.id) { used = true; break; }
            }
            if (used) break;
        }
        if (used) {
            resourcesUsed.push_back(res.id);
        }
    }
    int numResourceConstraints = resourcesUsed.size();

    // Total constraints: one for each resource plus one for equipment.
    int numConstraints = numResourceConstraints + 1;

    // Create the tableau: (numConstraints+1) rows and (numProducts+1) columns.
    // Row 0: objective function. Rows 1..numResourceConstraints: resource constraints.
    // Last row: equipment constraint.
    std::vector<std::vector<double>> tableau;
    tableau.resize(numConstraints + 1, std::vector<double>(numProducts + 1, 0.0));

    // Objective row: maximize total profit.
    // For simplicity, assume profit per unit = product.price.
    // Since our simplex code minimizes, we set coefficient = -profit.
    for (int j = 0; j < numProducts; j++) {
        double profit = productCatalog[j].price;
        tableau[0][j] = -profit;
    }
    tableau[0][numProducts] = 0.0;

    // Resource constraints: For each used resource, sum_j (recipe requirement) * x_j <= available.
    for (int i = 0; i < numResourceConstraints; i++) {
        int resId = resourcesUsed[i];
        for (int j = 0; j < numProducts; j++) {
            // Find the requirement of resource resId for product j.
            int reqQuantity = 0;
            for (const auto& req : productCatalog[j].recipe) {
                if (req.first == resId) {
                    reqQuantity = req.second;
                    break;
                }
            }
            tableau[i + 1][j] = reqQuantity;
        }
        // RHS is the available amount for resource resId.
        tableau[i + 1][numProducts] = resourceAvail[resId];
    }

    // Equipment constraint: Sum_j x_j <= equipmentCapacity.
    int equipRow = numConstraints; // Last row.
    for (int j = 0; j < numProducts; j++) {
        tableau[equipRow][j] = 1;
    }
    tableau[equipRow][numProducts] = equipmentCapacity;

    // Solve the LP using the simplex algorithm.
    Simplex simplex(tableau);
    bool solved = simplex.solve();
    if (!solved) {
        std::cout << "Simplex algorithm failed to find an optimal solution.\n";
        return;
    }
    std::vector<double> solution = simplex.getSolution();

    // --- Process the Production Decision ---
    // For each product, the solution gives the production quantity.
    for (int j = 0; j < numProducts; j++) {
        int productionQty = static_cast<int>(solution[j] + 0.001); // Round down.
        if (productionQty > 0) {
            const Commodity& prod = productCatalog[j];
            // Deduct resources from inventory according to the recipe.
            for (const auto& req : prod.recipe) {
                int resId = req.first;
                int totalRequired = productionQty * req.second;
                for (auto& item : factory.inventory) {
                    if (item.first.id == resId && item.first.type == CommodityType::Resource) {
                        item.second -= totalRequired;
                        break;
                    }
                }
            }
            // Add produced product to inventory.
            bool found = false;
            for (auto& item : factory.inventory) {
                if (item.first.id == prod.id && item.first.type == CommodityType::Product) {
                    item.second += productionQty;
                    found = true;
                    break;
                }
            }
            if (!found) {
                Commodity newProd = prod; // Copy product definition.
                factory.inventory.push_back({ newProd, productionQty });
            }
            // Place a sell order for the produced product.
            market.placeSellOrder(prod.id, productionQty, prod.price, factory.id);
            std::cout << "AI Factory " << factory.id << " produced and listed "
                << productionQty << " units of " << prod.name << ".\n";
        }
    }

    // --- Resource Replenishment ---
    // For each resource used in the LP, if current inventory is less than the target (the available amount used in LP),
    // then buy exactly the difference.
    for (const auto& res : resourceCatalog) {
        if (resourceAvail.find(res.id) == resourceAvail.end())
            continue;
        int target = resourceAvail[res.id];
        int current = 0;
        for (const auto& item : factory.inventory) {
            if (item.first.id == res.id && item.first.type == CommodityType::Resource) {
                current = item.second;
                break;
            }
        }
        if (current < target) {
            int amountToBuy = target - current;
            float buyPrice = res.price * 1.05f;
            market.placeBuyOrder(res.id, amountToBuy, buyPrice, factory.id);
            std::cout << "AI Factory " << factory.id << " placed BUY order for resource "
                << res.id << " for quantity " << amountToBuy << ".\n";
        }
    }

    // --- Equipment Upgrade ---
    // If the current equipment capacity is lower than the total production planned,
    // determine how much additional capacity is needed.
    int totalProduction = 0;
    for (int j = 0; j < numProducts; j++) {
        totalProduction += static_cast<int>(solution[j] + 0.001);
    }
    if (equipmentCapacity < totalProduction) {
        // Determine additional capacity needed.
        int neededCapacity = totalProduction - equipmentCapacity;
        // Evaluate each equipment option for additional capacity per cost.
        const Equipment* bestEquip = nullptr;
        double bestRatio = 0.0; // ratio = output_rate / price.
        for (const auto& equip : equipCatalog) {
            double ratio = static_cast<double>(equip.output_rate) / equip.price;
            if (ratio > bestRatio) {
                bestRatio = ratio;
                bestEquip = &equip;
            }
        }
        if (bestEquip && factory.balance >= bestEquip->price) {
            factory.balance -= bestEquip->price;
            factory.equipment.push_back(*bestEquip);
            std::cout << "AI Factory " << factory.id << " purchased Equipment " << bestEquip->id
                << " (Output Rate: " << bestEquip->output_rate << ").\n";
        }
        else {
            std::cout << "AI Factory " << factory.id << " cannot afford additional equipment upgrade.\n";
        }
    }
}
