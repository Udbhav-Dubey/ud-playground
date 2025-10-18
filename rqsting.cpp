#include <iostream>
#include <string>
#include <curl/curl.h>
#include "json.hpp" // The nlohmann/json header

// Function to handle the data received from libcurl
// libcurl writes the received data to this function in chunks.
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    // Cast the userp pointer to a std::string to append the data.
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// A simple struct to hold our stock data
struct StockData {
    std::string symbol;
    double price;
    long long volume;
};

// Main function to perform the request and parsing
int main() {
    // The string where we will store the API response
    std::string readBuffer;

    // Initialize a libcurl handle
    CURL* curl;
    CURLcode res;

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // Set the URL to a dummy JSON API for demonstration.
        // For a real stock system, you would use a stock data provider's URL.
        curl_easy_setopt(curl, CURLOPT_URL, "https://jsonplaceholder.typicode.com/posts/1");

        // Set the callback function to write the data into our buffer
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        // Pass the address of our buffer to the callback function
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // The request was successful, now let's parse the data.
            // For this example, we will use a hardcoded JSON string that
            // looks like stock data. In a real application, you would parse
            // the 'readBuffer' string directly.

            std::string stockJson = R"(
            {
                "symbol": "GOOGL",
                "current_price": 145.75,
                "volume": 2548792,
                "timestamp": 1678886400,
                "exchange": "NASDAQ"
            }
            )";

            try {
                // Use nlohmann/json to parse the string
                nlohmann::json jsonData = nlohmann::json::parse(stockJson);

                // Create a StockData object and extract values
                StockData stock;
                stock.symbol = jsonData["symbol"].get<std::string>();
                stock.price = jsonData["current_price"].get<double>();
                stock.volume = jsonData["volume"].get<long long>();

                // Print the parsed data
                std::cout << "Successfully fetched and parsed data:\n";
                std::cout << "Symbol: " << stock.symbol << "\n";
                std::cout << "Price: " << stock.price << "\n";
                std::cout << "Volume: " << stock.volume << "\n";
                
                // You can also check if a key exists before accessing it
                if (jsonData.contains("exchange")) {
                    std::cout << "Exchange: " << jsonData["exchange"].get<std::string>() << "\n";
                }

            } catch (nlohmann::json::parse_error& e) {
                // Handle JSON parsing errors
                std::cerr << "JSON parsing failed: " << e.what() << '\n';
            }
        }

        // Clean up the libcurl handle
        curl_easy_cleanup(curl);
    }

    // Clean up libcurl
    curl_global_cleanup();

    return 0;
}
