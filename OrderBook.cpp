// OrderBook.cpp
#include "OrderBook.h"
#include <iostream> // For debugging

OrderBook::OrderBook() {}

void OrderBook::clear() {
    bids.clear();
    bid_counts.clear();
    asks.clear();
    ask_counts.clear();
}

void OrderBook::add_order(char side, double price, long long size) {
    if (size <= 0) return;

    if (side == 'B') {
        bids[price] += size;
        bid_counts[price]++;
    } else if (side == 'A') {
        asks[price] += size;
        ask_counts[price]++;
    }
}

void OrderBook::delete_order(char side, double price, long long size) {
    if (size <= 0) return;

    if (side == 'B') {
        auto it_price = bids.find(price);
        auto it_count = bid_counts.find(price);

        if (it_price != bids.end() && it_count != bid_counts.end()) {
            it_price->second -= size;
            it_count->second--;

            if (it_price->second <= 0) { // If size drops to 0 or below, remove the level
                bids.erase(it_price);
                bid_counts.erase(it_count);
            }
        }
    } else if (side == 'A') {
        auto it_price = asks.find(price);
        auto it_count = ask_counts.find(price);

        if (it_price != asks.end() && it_count != ask_counts.end()) {
            it_price->second -= size;
            it_count->second--;

            // If size drops to 0 or below, remove the level
            if (it_price->second <= 0) { 
                asks.erase(it_price);
                ask_counts.erase(it_count);
            }
        }
    }
}

void OrderBook::modify_order(char side, double old_price, long long old_size, double new_price, long long new_size) {
    // This function is included for completeness but is not strictly necessary for the provided mbo.csv.
    // If 'M' actions were present and explicitly meant a modification, this would handle it.
    // For aggregated MBP, a modification can be seen as deleting the old entry and adding a new one.
    delete_order(side, old_price, old_size);
    add_order(side, new_price, new_size);
}

void OrderBook::process_trade(char aggressor_side, double trade_price, long long trade_size) {
    if (trade_size <= 0) return;

    long long remaining_trade_size = trade_size;

    if (aggressor_side == 'A') { // Ask aggressor, consuming from Bid side
        // Iterate bids from best (highest price) to worst (lowest price)
        auto it = bids.begin();
        while (it != bids.end() && remaining_trade_size > 0) {
            double current_bid_price = it->first;
            long long current_bid_size = it->second;

            // We consume bids at or below the trade price (from aggressor's perspective: fill at this price or better).
            // For an ASK aggressor, "better" means a lower or equal bid price.
            if (current_bid_price >= trade_price) { // Assuming trade_price is the limit or better for the other side
                long long consumed_size = std::min(remaining_trade_size, current_bid_size);

                // Inline logic to reduce quantity
                it->second -= consumed_size;
                bid_counts.at(current_bid_price)--; // Decrement count of orders at this level

                remaining_trade_size -= consumed_size;

                if (it->second <= 0) { // If the level is fully consumed
                    it = bids.erase(it); // Erase returns iterator to the next element
                    bid_counts.erase(current_bid_price); // Remove count entry
                } else {
                    ++it; // Move to the next element
                }
            } else {
                // If the current bid price is below the trade price, it shouldn't be consumed
                // by a trade at 'trade_price' (assuming simple price-time priority within levels).
                break;
            }
        }
    } else if (aggressor_side == 'B') { // Bid aggressor, consuming from Ask side
        // Iterate asks from best (lowest price) to worst (highest price)
        auto it = asks.begin();
        while (it != asks.end() && remaining_trade_size > 0) {
            double current_ask_price = it->first;
            long long current_ask_size = it->second;

            // We consume asks at or above the trade price (from aggressor's perspective: fill at this price or better).
            // For a BID aggressor, "better" means a higher or equal ask price.
            if (current_ask_price <= trade_price) { // Assuming trade_price is the limit or better for the other side
                long long consumed_size = std::min(remaining_trade_size, current_ask_size);

                // Inline logic to reduce quantity
                it->second -= consumed_size;
                ask_counts.at(current_ask_price)--; // Decrement count of orders at this level

                remaining_trade_size -= consumed_size;

                if (it->second <= 0) { // If the level is fully consumed
                    it = asks.erase(it); // Erase returns iterator to the next element
                    ask_counts.erase(current_ask_price); // Remove count entry
                } else {
                    ++it; // Move to the next element
                }
            } else {
                // If the current ask price is above the trade price, it shouldn't be consumed
                // by a trade at 'trade_price'.
                break;
            }
        }
    }
}

void OrderBook::get_10_snapshot(std::vector<MBPLevel>& bid_levels, std::vector<MBPLevel>& ask_levels) const {
    bid_levels.clear();
    ask_levels.clear();

    // Get top 10 bids
    int count = 0;
    for (const auto& pair : bids) {
        if (count >= 10) break;
        bid_levels.push_back(MBPLevel(pair.first, pair.second, bid_counts.at(pair.first)));
        count++;
    }

    // Fill remaining bid levels with zeros if less than 10
    while (bid_levels.size() < 10) {
        bid_levels.push_back(MBPLevel()); // Use default constructor for empty levels
    }

    // Get top 10 asks
    count = 0;
    for (const auto& pair : asks) {
        if (count >= 10) break;
        ask_levels.push_back(MBPLevel(pair.first, pair.second, ask_counts.at(pair.first)));
        count++;
    }

    // Fill remaining ask levels with zeros if less than 10
    while (ask_levels.size() < 10) {
        ask_levels.push_back(MBPLevel()); // Use default constructor for empty levels
    }
}