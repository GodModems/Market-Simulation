#pragma once
#include <string>
#include <vector>
#include <utility>

enum class CommodityType {
    Resource,  // Raw materials that can only be bought/sold.
    Product    // Finished goods that can be produced as well.
};

struct Equipment {
    int id;                // Unique equipment type identifier.
    float price;
    int output_rate;         // Per Day.
    float operational_cost;  // Per Day.
};

struct Commodity {
    int id;
    std::string name;   // Human-readable name.
    float price;
    CommodityType type;
    // Production recipe – a list of required commodity IDs and the quantity needed.
    // This allows a product to require either resources or other commodities.
    std::vector<std::pair<int, int>> recipe;
    // Equipment required for production: equipment id and quantity required.
    std::vector<std::pair<int, int>> requiredEquipment;
};
