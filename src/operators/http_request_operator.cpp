/*
* This file is part of Kufibot.
 *
 * Kufibot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kufibot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kufibot. If not, see <https://www.gnu.org/licenses/>.
 */

#include "http_request_operator.h"
#include "nlohmann/json.hpp"
#include <curl/curl.h>
#include "../logger.h"

using Json = nlohmann::json;

HttpRequestOperator* HttpRequestOperator::_instance = nullptr;

HttpRequestOperator* HttpRequestOperator::get_instance(const std::string& url) {
    if (_instance == nullptr) {
        _instance = new HttpRequestOperator(url);
    }
    return _instance;
}


HttpRequestOperator::HttpRequestOperator(const std::string& url) : url_(url) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

HttpRequestOperator::~HttpRequestOperator()
{
        curl_global_cleanup();  
}

void HttpRequestOperator::query_llama_text(const std::string& prompt) {
    std::string url = "http://192.168.1.20:11434/api/generate";
    std::string payload = R"({"model": "kufi", "prompt":")" + prompt + R"("})";

    // Initialize libcurl
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL." << std::endl;
        return;
    }

    CURLcode res;

    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &_responseBuffer);

    // Perform the request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        ERROR("CURL error: {}" + std::string(curl_easy_strerror(res)));
        curl_easy_cleanup(curl);
        return;
    }

    curl_easy_cleanup(curl);
}

size_t HttpRequestOperator::write_callback(void *contents, size_t size, size_t nmemb, std::string &out)
{
    size_t totalSize = size * nmemb; // Total size of the incoming data
    std::string data(static_cast<char*>(contents), totalSize); // Convert to std::string
    auto jsonData = nlohmann::json::parse(data);
    std::string word = jsonData["response"];
    out.append(word );
    if(word.find("!") != std::string::npos || word.find(".")!= std::string::npos ||
        word.find("?") != std::string::npos){

            out.erase(out.begin(), std::find_if(out.begin(), out.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            out.erase(std::find_if(out.rbegin(), out.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), out.end());

            out.clear();
        }
        
    return totalSize;
}


std::string HttpRequestOperator::parse_response_llm(const std::string &response) {
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