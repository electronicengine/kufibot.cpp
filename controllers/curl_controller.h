#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>  // Include the nlohmann JSON library

using Json = nlohmann::json;

class CurlController {
public:
    // Constructor that takes the URL
    explicit CurlController(const std::string& url);
    ~CurlController();

    static CurlController *get_instance(const std::string& url);
    std::string execute_llm(const std::string& prompt);
    std::string execute_gemini(const std::string& prompt);
private:
    std::string url_;
    static CurlController* _instance;
    std::string parse_response_llm(const std::string &response); 
    std::string parse_response_gemini(const std::string &response); 

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);
};