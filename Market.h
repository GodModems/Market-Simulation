#pragma once
#include <vector>

enum class OrderType { BUY, SELL };

struct Order {
    int id;         // Unique order identifier
    int productId;  // The product for which the order is placed
    OrderType type;
    float price;    // For BUY orders, this is the maximum price; for SELL orders, it’s the asking price.
    int amount;     // Quantity of the product
    int ownerId;    // Identifier for the factory or market participant
};

class Market {
public:
    // Order book for all products.
    std::vector<Order> orders;
    int nextOrderId;

    Market();

    // Place a BUY order (bid) for a product.
    void placeBuyOrder(int productId, int amount, float maxPrice, int ownerId);

    // Place a SELL order (ask) for a product.
    void placeSellOrder(int productId, int amount, float price, int ownerId);

    // Remove an existing order (only if the owner requests it).
    // Returns true if the order is found and removed; false otherwise.
    bool removeOrder(int orderId, int ownerId);

private:
    // Matching engine for a given product. It matches BUY orders with SELL orders.
    void matchOrders(int productId);
};
