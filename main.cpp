#include <iostream>
#include "Initialization.h"
#include "PlayerController.h"
#include "AIController.h"
#include "ResourceMarket.h"  // For updateResourcePrices()

int main() {
    // Initialize the simulation world.
    SimulationWorld world = initializeSimulation();

    // Create controllers.
    PlayerController playerController;
    AIController aiController;

    int day = 1;
    char cont;
    while (true) {
        std::cout << "\n===== Day " << day << " =====\n";

        // Process the player-controlled factory turn.
        playerController.takeTurn(world);

        // Process each AI-controlled factory turn.
        for (auto& aiFactory : world.aiFactories) {
            aiController.updateFactory(world, aiFactory);
        }

        // Update market prices and (if needed) clear or match orders.
        updateResourcePrices(world);

        // Optionally, clear old orders here or run additional market clearing logic.
        // For example: world.market.clearOrders();

        day++;
        std::cout << "\nProceed to next day? (y/n): ";
        std::cin >> cont;
        if (cont == 'n' || cont == 'N')
            break;
    }

    std::cout << "\nSimulation ended.\n";
    return 0;
}
