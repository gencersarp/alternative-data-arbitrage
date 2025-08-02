#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <curl/curl.h>
#include "json.hpp"

// Use the nlohmann json library
using json = nlohmann::json;

// Enum to represent our trading signals
enum class Signal {
    BUY,
    SELL,
    HOLD
};

// Simple struct to hold daily price data
struct DailyPrice {
    double close;
};

// Simple struct to manage our portfolio
struct Portfolio {
    double cash;
    int shares;
    double initial_value;
};

// --- Forward Declarations ---
std::string fetchURL(const std::string& url);
Signal getSignalForDate(const json& sentiment_data, const std::string& date, const std::string& ticker);


// --- Backtester Class ---
class Backtester {
public:
    void run(const std::map<std::string, DailyPrice>& prices, const json& sentiment_data, const std::string& ticker) {
        if (prices.empty()) {
            std::cerr << "Price data is empty. Cannot run backtest." << std::endl;
            return;
        }

        Portfolio portfolio = {10000.0, 0, 10000.0}; // Start with $10,000
        
        std::vector<std::string> dates;
        for(auto const& [date, val] : prices) {
            dates.push_back(date);
        }
        std::sort(dates.begin(), dates.end());

        std::cout << "\n--- Running Backtest ---" << std::endl;
        std::cout << "Initial Portfolio Value: $" << portfolio.initial_value << std::endl;

        for (const auto& date : dates) {
            Signal signal = getSignalForDate(sentiment_data, date, ticker);
            double current_price = prices.at(date).close;

            if (signal == Signal::BUY && portfolio.cash >= current_price) {
                int shares_to_buy = portfolio.cash / current_price;
                portfolio.shares += shares_to_buy;
                portfolio.cash -= shares_to_buy * current_price;
                std::cout << date << ": BUY " << shares_to_buy << " shares at $" << current_price << std::endl;
            } else if (signal == Signal::SELL && portfolio.shares > 0) {
                portfolio.cash += portfolio.shares * current_price;
                std::cout << date << ": SELL " << portfolio.shares << " shares at $" << current_price << std::endl;
                portfolio.shares = 0;
            }
        }

        // Calculate final portfolio value
        double final_value = portfolio.cash + (portfolio.shares * prices.at(dates.back()).close);
        double pnl = final_value - portfolio.initial_value;

        // Calculate buy and hold performance
        double buy_and_hold_value = (portfolio.initial_value / prices.at(dates.front()).close) * prices.at(dates.back()).close;

        std::cout << "\n--- Backtest Results ---" << std::endl;
        std::cout << "Final Portfolio Value: $" << final_value << std::endl;
        std::cout << "Total Profit/Loss: $" << pnl << std::endl;
        std::cout << "Buy and Hold Value: $" << buy_and_hold_value << std::endl;
        std::cout << "-------------------------\n";
    }
};

// --- Utility Functions ---

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string fetchURL(const std::string& url) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Gemini-CLI");
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if(res != CURLE_OK) return "";
    }
    return readBuffer;
}

// Fetches historical daily prices
std::map<std::string, DailyPrice> fetchHistoricalPrices(const std::string& ticker, const std::string& apiKey) {
    std::string url = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=" + ticker + "&apikey=" + apiKey;
    std::string jsonResponse = fetchURL(url);
    std::map<std::string, DailyPrice> prices;

    try {
        json data = json::parse(jsonResponse);
        if (data.contains("Time Series (Daily)")) {
            for (auto& [date, price_data] : data["Time Series (Daily)"].items()) {
                prices[date] = {std::stod(price_data["4. close"].get<std::string>())};
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching or parsing price data: " << e.what() << std::endl;
    }
    return prices;
}

// Gets the signal for a specific date from the sentiment feed
Signal getSignalForDate(const json& sentiment_data, const std::string& date, const std::string& ticker) {
    try {
        if (sentiment_data.contains("feed")) {
            for (const auto& article : sentiment_data["feed"]) {
                std::string article_time_str = article["time_published"]; // e.g., 20231026T143000
                std::string article_date = article_time_str.substr(0, 4) + "-" + article_time_str.substr(4, 2) + "-" + article_time_str.substr(6, 2);

                if (article_date == date) {
                    for (const auto& ticker_sentiment : article["ticker_sentiment"]) {
                        if (ticker_sentiment["ticker"] == ticker) {
                            double score = std::stod(ticker_sentiment["ticker_sentiment_score"].get<std::string>());
                            if (score >= 0.35) return Signal::BUY;
                            if (score <= -0.15) return Signal::SELL;
                            return Signal::HOLD;
                        }
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        // Silently fail, as not every day will have a signal
    }
    return Signal::HOLD; // Default to HOLD if no article for that day
}


// --- Main Application Logic ---

int main() {
    std::string apiKey = "YourAPIKey";
    std::string ticker = "IBM"; // Using IBM as it has more news diversity
    
    std::cout << "Fetching data for ticker: " << ticker << std::endl;

    // 1. Fetch Sentiment Data
    std::string sentiment_url = "https://www.alphavantage.co/query?function=NEWS_SENTIMENT&tickers=" + ticker + "&limit=200&apikey=" + apiKey;
    std::string sentiment_json_str = fetchURL(sentiment_url);
    json sentiment_data;
    if (!sentiment_json_str.empty()) {
        try { sentiment_data = json::parse(sentiment_json_str); } catch (const std::exception& e) { 
            std::cerr << "Sentiment JSON parse error: " << e.what() << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Failed to fetch sentiment data." << std::endl;
        return 1;
    }

    // 2. Fetch Historical Price Data
    auto prices = fetchHistoricalPrices(ticker, apiKey);
    if (prices.empty()) {
        std::cerr << "Failed to fetch price data." << std::endl;
        return 1;
    }

    // 3. Run the Backtest
    Backtester backtester;
    backtester.run(prices, sentiment_data, ticker);

    return 0;
}
