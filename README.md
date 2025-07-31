**Orderbook Reconstruction from MBO Data
=========================================

This project provides a C++ solution to reconstruct a Level 10 Market By Price (MBP-10) orderbook from Market By Order (MBO) data, following specific rules for trade processing.

**1. Project Structure:**
--------------------
- `OrderBook.h`: Header file defining the `OrderBook` class and `MBPLevel` structure.
- `OrderBook.cpp`: Implementation file for the `OrderBook` class methods.
- `main.cpp`: The main program logic for reading MBO data, processing it, and writing MBP-10 output.
- `Makefile`: Used to compile the source code into an executable.
- `Readme.txt`: Explanation(text file).

**2. Compilation Instructions:**
----------------------------
To build the executable, navigate to the project directory in your terminal and run `make`:

```bash
make
````
if you have mingw compiler to build executable,run:

```bash
mingw32-make
```

This will create an executable file named `reconstruction_username` (replace `username` with the name specified in the Makefile).

## **3. Usage:**

The executable takes one command-line argument: the path to the MBO input CSV file.

Example usage:

```bash
./reconstruction_dixit <path of mbo csv file>
```

The program will generate an output CSV file named `mbp_output.csv` in the same directory, containing the reconstructed MBP-10 orderbook.

## **4. Overall Optimization Steps Taken:**

  - **Standard Library Containers:** `std::map` is used for storing bid and ask levels. `std::map` provides logarithmic time complexity for insertions, deletions, and lookups, and keeps the keys (prices) sorted. This is efficient for maintaining sorted price levels, especially when the depth (10 levels) is relatively small.
      - `std::map<double, long long, std::greater<double>> bids;`: Ensures bids are sorted from highest price to lowest.
      - `std::map<double, long long, std::less<double>> asks;`: Ensures asks are sorted from lowest price to highest.
  - **Efficient I/O:** `std::ifstream` and `std::ofstream` are used for file operations. Basic string parsing with `std::stringstream` and `std::getline` is employed for reading CSV lines efficiently. `std::fixed` and `std::setprecision` are used for consistent floating-point output.
  - **Minimal Copying:** Data is processed line by line, minimizing the need to load the entire dataset into memory.
  - **Direct Orderbook Manipulation:** Orderbook updates (add, delete, trade) directly modify the `std::map` containers, avoiding intermediate data structures where possible.

## **5. Special Things to Take Note When Running  Code (Logic Explanation):**

**a. Ignoring Initial Row:**

  - The program specifically skips the very first data row from `mbo.csv` if its `action` is 'R' (Clear). This adheres to the requirement of assuming an empty orderbook at the start of the day. The `Unnamed: 0` column in `mbp_output.csv` will align with the sample `mbp.csv` by starting from 0 for the first *processed* MBO event.

**b. Handling 'T', 'F', 'C' Actions (Trade Logic):**

  - This is a crucial aspect of the reconstruction. As per the problem description:
      - The `[T]rade`, `[F]ill`, and `[C]ancel` actions from MBO are combined into a *single 'T' action* that reflects changes in the MBP-10 orderbook.
      - When a 'T' action occurs, its `side` (`'A'` for Ask aggressor, `'B'` for Bid aggressor) indicates the aggressor. The liquidity is consumed from the *opposite* side of the orderbook.
      - The `price` and `size` fields of the 'T' action directly represent the traded price and quantity.
      - **Mechanism:**
          - If `action` is 'T' and `side` is 'A' (Ask aggressor): The trade consumed liquidity from the **BID** side. The `process_trade` method iterates through the current bid levels (from best to worst) and reduces their quantities by the `trade_size` from the 'T' action. It attempts to fulfill the trade by matching against bids at prices *greater than or equal to* the `trade_price`.
          - If `action` is 'T' and `side` is 'B' (Bid aggressor): The trade consumed liquidity from the **ASK** side. The `process_trade` method iterates through the current ask levels (from best to worst) and reduces their quantities by the `trade_size` from the 'T' action. It attempts to fulfill the trade by matching against asks at prices *less than or equal to* the `trade_price`.
          - The subsequent `[F]ill` and `[C]ancel` actions (if they immediately follow a 'T' and relate to the same trade) are effectively treated as informational and do *not* trigger additional orderbook changes, as their effect is encompassed by the 'T' action's processing on the opposite side.

**c. Handling 'T' Action with Side 'N':**

  - If an `action` is 'T' and its `side` is 'N' (Neutral), the problem explicitly states that the orderbook should *not* be altered. This rule is applied directly, bypassing the trade processing logic for such events.

**d. MBP-10 Output Format:**

  - The output `mbp_output.csv` meticulously adheres to the column structure and formatting of the provided `mbp.csv` sample.
  - For MBP levels where there are fewer than 10 price points on a side, the remaining `bid_px_XX` and `ask_px_XX` columns are output as empty strings (effectively `NaN` in pandas), while `bid_sz_XX`, `bid_ct_XX`, `ask_sz_XX`, `ask_ct_XX` are output as `0`. This matches the behavior observed in the sample `mbp.csv`.
  - Floating-point prices are formatted with `std::fixed` and `std::setprecision(8)` to maintain consistency.

**e. Order ID and Count (`_ct_`) Handling:**

  - The problem statement focuses on reconstructing aggregated price levels (MBP). While `mbo.csv` contains `order_id`, the `OrderBook` class primarily aggregates `size` per `price` level.
  - The `bid_counts` and `ask_counts` maps are maintained to track the number of distinct orders at each price level, ensuring the `_ct_` columns in the `mbp_output.csv` are correct. When an order is added, its count increments. When an order is deleted, its count decrements. In case of trade processing, if a price level is completely consumed, its count is also removed.

<!-- end list -->

```
```
