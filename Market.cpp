#include "Market.h"
#include <iostream>
#include <algorithm>
#include <limits>

Market::Market() : nextOrderId(1) {}

void Market::placeBuyOrder(int productId, int amount, float maxPrice, int ownerId) {
    Order order = { nextOrderId++, productId, OrderType::BUY, maxPrice, amount, ownerId };
    orders.push_back(order);
    std::cout << "Placed BUY order: ID " << order.id
        << ", Product " << productId
        << ", Amount " << amount
        << ", Max Price " << maxPrice << "\n";
    matchOrders(productId);
}

void Market::placeSellOrder(int productId, int amount, float price, int ownerId) {
    Order order = { nextOrderId++, productId, OrderType::SELL, price, amount, ownerId };
    orders.push_back(order);
    std::cout << "Placed SELL order: ID " << order.id
        << ", Product " << productId
        << ", Amount " << amount
        << ", Price " << price << "\n";
    // Only immediately match if the order is not a market-generated sell order.
    if (ownerId != 0) {
        matchOrders(productId);
    }
}


bool Market::removeOrder(int orderId, int ownerId) {
    auto it = std::find_if(orders.begin(), orders.end(), [&](const Order& o) {
        return o.id == orderId;
        });
    if (it != orders.end()) {
        if (it->ownerId == ownerId) {
            std::cout << "Removed order ID " << orderId << "\n";
            orders.erase(it);
            return true;
        }
        else {
            std::cout << "Order ID " << orderId
                << " does not belong to owner " << ownerId << "\n";
            return false;
        }
    }
    std::cout << "Order ID " << orderId << " not found\n";
    return false;
}

void Market::matchOrders(int productId) {
    // Create temporary vectors to hold pointers to BUY and SELL orders for the product.
    std::vector<Order*> buyOrders;
    std::vector<Order*> sellOrders;

    for (auto& order : orders) {
        if (order.productId != productId || order.amount <= 0)
            continue;
        if (order.type == OrderType::BUY)
            buyOrders.push_back(&order);
        else if (order.type == OrderType::SELL)
            sellOrders.push_back(&order);
    }

    // Sort BUY orders in descending order (highest bids first).
    std::sort(buyOrders.begin(), buyOrders.end(), [](Order* a, Order* b) {
        return a->price > b->price;
        });

    // Sort SELL orders in ascending order (lowest asks first).
    std::sort(sellOrders.begin(), sellOrders.end(), [](Order* a, Order* b) {
        return a->price < b->price;
        });

    // Attempt to match orders.
    while (!buyOrders.empty() && !sellOrders.empty()) {
        Order* bestBuy = buyOrders.front();
        Order* bestSell = sellOrders.front();

        // A match occurs if the highest bid meets or exceeds the lowest ask.
        if (bestBuy->price >= bestSell->price) {
            // Execute a trade for the minimum amount between the two orders.
            int tradeAmount = std::min(bestBuy->amount, bestSell->amount);
            float tradePrice = bestSell->price; // Using the SELL price as the trade price.

            std::cout << "Trade executed: Product " << productId
                << " | Amount: " << tradeAmount
                << " | Price: " << tradePrice << "\n";

            bestBuy->amount -= tradeAmount;
            bestSell->amount -= tradeAmount;

            // Remove orders from temporary lists if fully executed.
            if (bestBuy->amount == 0)
                buyOrders.erase(buyOrders.begin());
            if (bestSell->amount == 0)
                sellOrders.erase(sellOrders.begin());
        }
        else {
            // No further matches are possible.
            break;
        }
    }

    // Clean up the main order book by removing fully executed orders.
    orders.erase(
        std::remove_if(orders.begin(), orders.end(), [](const Order& o) {
            return o.amount <= 0;
            }),
        orders.end()
    );
}
