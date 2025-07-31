#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <map>
#include <vector>
#include <string>
#include <algorithm> 
#include <functional> 

// Structure to hold a single level of the orderbook
struct MBPLevel {
    double price;
    long long size;
    // Number of orders at this price level
    int count; 

    // Explicit constructor to allow brace-enclosed initialization
    MBPLevel(double p = 0.0, long long s = 0, int c = 0) : price(p), size(s), count(c) {}
};

class OrderBook {
public:
    // Using std::map for bids (price, size), sorted in descending order of price
    // Using std::map for asks (price, size), sorted in ascending order of price
    std::map<double, long long, std::greater<double>> bids;
    std::map<double, int, std::greater<double>> bid_counts;

    std::map<double, long long, std::less<double>> asks;
    std::map<double, int, std::less<double>> ask_counts;

    OrderBook();

    // Clears the entire order book
    void clear();

    // Adds a new order to the book
    void add_order(char side, double price, long long size);

    // Deletes an order from the book
    void delete_order(char side, double price, long long size);

    // Modifies an existing order (treated as delete old + add new)
    void modify_order(char side, double old_price, long long old_size, double new_price, long long new_size);

    // Processes a trade event
    void process_trade(char aggressor_side, double trade_price, long long trade_size);

    // Gets the top 10 bid and ask levels
    void get_10_snapshot(std::vector<MBPLevel>& bid_levels, std::vector<MBPLevel>& ask_levels) const;

private:
    // The previous 'reduce_quantity' helper has been inlined into other methods
    // to avoid type complexity with custom map comparators.
};

#endif 
