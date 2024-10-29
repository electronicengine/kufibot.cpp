#include "curl_controller.h"

using Json = nlohmann::json;

CurlController* CurlController::_instance = nullptr;

CurlController* CurlController::get_instance(const std::string& url) {
    if (_instance == nullptr) {
        _instance = new CurlController(url);
    }
    return _instance;
}


CurlController::CurlController(const std::string& url) : url_(url) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

CurlController::~CurlController()
{
        curl_global_cleanup();  
}

std::string CurlController::execute_llm(const std::string& prompt) {
    CURL* curl = curl_easy_init();  // Initialize CURL
    std::string response;

    if (curl) {
        // Create JSON payload
        Json data = {
            {"model", "kufi"},
            {"prompt", prompt},
            {"options", {{"num_predict", 120}}}
        };
        std::string jsonData = data.dump();  // Serialize JSON to string

        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

        // Set the write callback to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Set HTTP headers
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize CURL" << std::endl;
    }


    return parse_response_llm(response);
}

std::string CurlController::execute_gemini(const std::string& prompt) {
    CURL* curl = curl_easy_init();  // Initialize CURL
    std::string response;

    if (curl) {
        // Create JSON payload matching the required structure
        Json data = {
            {"contents", {
                {{"parts", {{{"text", prompt}}}}}
            }},
            {"generationConfig", {
                {"temperature", 1.0},
                {"maxOutputTokens", 800},
                {"topP", 0.8},
                {"topK", 10}
            }}
        };
        std::string jsonData = data.dump();  // Serialize JSON to string

        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

        // Set the write callback to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Set HTTP headers
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize CURL" << std::endl;
    }
    return parse_response_gemini(response);
}

std::string CurlController::parse_response_gemini(const std::string &response) {
   try {
        // Parse the JSON response
        Json parsed = Json::parse(response);

        // Navigate to the "text" field within the structure
        std::string text = parsed["candidates"][0]["content"]["parts"][0]["text"].get<std::string>();

        return text;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return "";
    }
}


std::string CurlController::parse_response_llm(const std::string &response) {
    // Parse the input response
    std::string combined_response;
    size_t start = 0, end = 0;

    // Loop through the input string to extract each JSON object
    while ((start = response.find('{', end)) != std::string::npos) {
        end = response.find('}', start);
        if (end == std::string::npos) break; // No more JSON objects found

        // Extract the JSON substring
        std::string json_str = response.substr(start, end - start + 1);
        Json data = Json::parse(json_str);

        // Combine the "response" field
        combined_response += data["response"].get<std::string>();

        // Move to the next part of the string
        end++;
    }

    return combined_response;
}


size_t CurlController::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}
