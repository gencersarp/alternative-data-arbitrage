# Alternative Data Arbitrage Engine

This project is a C++ application that demonstrates a complete, end-to-end quantitative trading strategy based on alternative data. It fetches real-world news sentiment data from the Alpha Vantage API, generates trading signals based on that sentiment, and runs a historical backtest to evaluate the strategy's performance.

This project is intended as a portfolio piece to showcase skills in C++, software architecture, data handling, API integration, and quantitative analysis.

## Core Features

1.  **Data Pipeline**: Fetches news sentiment and historical price data from the [Alpha Vantage](https://www.alphavantage.co/) API.
2.  **Signal Generation**: Implements a simple, rule-based logic to convert raw sentiment scores into actionable BUY, SELL, or HOLD signals.
3.  **Event-Driven Backtester**: Simulates the trading strategy against historical daily price data to measure its effectiveness.
4.  **Performance Analysis**: Calculates the strategy's total profit/loss and compares it against a baseline "Buy and Hold" strategy.

## Technologies Used

*   **Language**: C++ (17)
*   **Build System**: CMake
*   **Networking**: libcurl
*   **JSON Parsing**: nlohmann/json
*   **Financial Data**: Alpha Vantage API

## How to Build and Run

### Prerequisites

*   A C++ compiler (like GCC, Clang, or MSVC)
*   CMake (version 3.10 or higher)
*   libcurl (must be installed on your system)

### Build Steps

1.  **Clone the repository:**
    ```bash
    git clone <your-repository-url>
    cd alternative-data-arbitrage
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Run CMake and Make:**
    ```bash
    cmake ..
    make
    ```

4.  **Run the engine:**
    The executable `arbitrage_engine` will be created in the `build` directory.
    ```bash
    ./arbitrage_engine
    ```

## Interpreting the Results

The program will output the results of the backtest, for example:

```
--- Backtest Results ---
Final Portfolio Value: $8831.87
Total Profit/Loss: $-1168.13
Buy and Hold Value: $9854.03
-------------------------
```

This shows that for this particular run, the sentiment strategy underperformed a simple "buy and hold" strategy. This is a realistic outcome and highlights the importance of rigorous backtesting. The value of this project lies in the creation of the testing *framework* itself.

## Future Improvements

The current strategy is intentionally simple. The real value of this engine is as a framework for testing more complex ideas. Future improvements could include:

*   **More Sophisticated Signals**: Incorporate the *relevance* or *magnitude* of news, not just a simple sentiment score.
*   **Multiple Data Sources**: Combine sentiment from different sources (e.g., Twitter, Reddit) for a more robust signal.
*   **Advanced Risk Management**: Implement position sizing based on signal confidence or market volatility, rather than investing 100% of capital on each trade.
