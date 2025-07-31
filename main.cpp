#include "OrderBook.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
// For std::fixed and std::setprecision
#include <iomanip> 

#include <cmath>
using std::isnan;

// Helper function to parse a CSV line
std::vector<std::string> parse_csv_line(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(line);
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    return tokens;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "provide " << argv[0] << "input_file.csv" << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];
    std::string output_filename = "mbp_output.csv"; 

    std::ifstream infile(input_filename);
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open input file " << input_filename << std::endl;
        return 1;
    }

    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open output file " << output_filename << std::endl;
        return 1;
    }

    OrderBook order_book;
    std::string line;

    // Read and skip header of mbo.csv
    if (std::getline(infile, line)) {
        // Output header for mbp_output.csv
        outfile << ",ts_recv,ts_event,rtype,publisher_id,instrument_id,action,side,depth,price,size,flags,ts_in_delta,sequence";
        for (int i = 0; i < 10; ++i) {
            outfile << ",bid_px_" << std::setw(2) << std::setfill('0') << i << ",bid_sz_" << std::setw(2) << std::setfill('0') << i << ",bid_ct_" << std::setw(2) << std::setfill('0') << i;
            outfile << ",ask_px_" << std::setw(2) << std::setfill('0') << i << ",ask_sz_" << std::setw(2) << std::setfill('0') << i << ",ask_ct_" << std::setw(2) << std::setfill('0') << i;
        }
        outfile << ",symbol,order_id\n";
    }

    // To match the "Unnamed: 0" column in sample mbp.csv
    long long row_index = 0; 

    // Read and process mbo.csv rows
    while (std::getline(infile, line)) {
        std::vector<std::string> tokens = parse_csv_line(line);

        // Skip the initial row as this is a “clear[R]” action to clear the orderbook
        // -> Assume we are starting the day with an empty orderbook
        if (tokens[5] == "R" && row_index == 0) {
            // Increment row index even for skipped row to align with mbp.csv's first row
            row_index++; 
            continue;
        }
        // Increment for subsequent rows
        row_index++; 

        // Extract relevant fields
        std::string ts_recv = tokens[0];
        std::string ts_event = tokens[1];
        int rtype = std::stoi(tokens[2]);
        int publisher_id = std::stoi(tokens[3]);
        int instrument_id = std::stoi(tokens[4]);
        char action = tokens[5][0]; 
        char side = tokens[6][0];   

        // Handle empty price for 'R' action or other cases
        double price = 0.0;
        if (!tokens[7].empty()) {
            try {
                price = std::stod(tokens[7]);
            } catch (const std::invalid_argument& e) {
                // Handle cases where price might be empty or invalid (e.g., for 'R' action)
                // Default to 0.0 
                price = 0.0; 
            }
        }
        long long size = std::stoll(tokens[8]);
        // Removed: unused variable
        // int channel_id = std::stoi(tokens[9]); 
        long long order_id = std::stoll(tokens[10]);
        int flags = std::stoi(tokens[11]);
        long long ts_in_delta = std::stoll(tokens[12]);
        long long sequence = std::stoll(tokens[13]);
        std::string symbol = tokens[14];


        // Removed: unused variable
        // bool orderbook_changed = false; 

        if (action == 'A') {
            order_book.add_order(side, price, size);
            // orderbook_changed = true;
        } else if (action == 'D') {
            order_book.delete_order(side, price, size);
            // orderbook_changed = true;
        } else if (action == 'T') {
            // "If the side of the row with action ‘T’ is ‘N’, we should not alter the orderbook."
            if (side != 'N') {
                // "combine these 3 actions in the MBO to a single T action in the MBP-10 output that does reflect in the orderbook."
                // This implies that 'T' itself triggers the orderbook change on the opposite side.
                // The price and size from the 'T' action are the traded price and size.
                order_book.process_trade(side, price, size);
                // orderbook_changed = true;
            }
            // 'F' and 'C' actions are considered part of the 'T' sequence and their
            // direct orderbook effects are handled by the 'T' action itself.
            // So, they don't trigger separate orderbook changes.
        }
        // 'M' actions are not directly handled as per problem statement focus,
        // if they were critical they would need specific old/new price/size from data.


        // Generate MBP-10 snapshot and write to output for every MBO row (except initial 'R')
        std::vector<MBPLevel> bid_levels;
        std::vector<MBPLevel> ask_levels;
        order_book.get_10_snapshot(bid_levels, ask_levels);

        // Write to output file in the exact format of mbp.csv
        // Unnamed: 0 column
        outfile << row_index -1 << ","; 
        outfile << ts_recv << "," << ts_event << "," << rtype << "," << publisher_id << "," << instrument_id << ",";
        // Depth is 0 in sample mbp.csv for MBO derived rows.
        outfile << action << "," << side << ",0,"; 
        
        // Output price and size from MBO row
        // Handle potential NaN for price field from MBO
        if (std::isnan(price)) { 
             outfile << ",";
        } else {
             outfile << std::fixed << std::setprecision(8) << price << ",";
        }
        outfile << size << ",";

        outfile << flags << "," << ts_in_delta << "," << sequence;

        // Output bid levels
        for (int i = 0; i < 10; ++i) {
            // Output NaN for empty price as per sample mbp.csv
            if (bid_levels[i].price == 0.0 && bid_levels[i].size == 0 && bid_levels[i].count == 0) {
                // Represents empty price, 0 for size and count
                 outfile << ",,0,0"; 
            } else {
                 outfile << "," << std::fixed << std::setprecision(8) << bid_levels[i].price << "," << bid_levels[i].size << "," << bid_levels[i].count;
            }
        }

        // Output ask levels
        for (int i = 0; i < 10; ++i) {
            if (ask_levels[i].price == 0.0 && ask_levels[i].size == 0 && ask_levels[i].count == 0) {
                // Represents empty price, 0 for size and count
                outfile << ",,0,0"; 
            } else {
                outfile << "," << std::fixed << std::setprecision(8) << ask_levels[i].price << "," << ask_levels[i].size << "," << ask_levels[i].count;
            }
        }

        outfile << "," << symbol << "," << order_id << "\n";
    }

    infile.close();
    outfile.close();

    std::cout << "MBP-10 reconstruction complete. Output saved to " << output_filename << std::endl;

    return 0;
}
