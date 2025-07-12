
#ifndef CURL_CONTROLLER_H
#define CURL_CONTROLLER_H


#include <string>
#include "../publisher.h"
#include "nlohmann/json.hpp"  // Include the nlohmann JSON library

using Json = nlohmann::json;

class CurlController : public Publisher {
public:
    // Constructor that takes the URL
    explicit CurlController(const std::string& url);
    ~CurlController();

    static CurlController *get_instance(const std::string& url);
    void query_llama_text(const std::string& prompt);

private:
    std::string url_;
    std::string _responseBuffer;
    static CurlController* _instance;
    std::string parse_response_llm(const std::string &response); 
    static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string& responseBuffer);
    
};

#endif //CURL_CONTROLLER_H