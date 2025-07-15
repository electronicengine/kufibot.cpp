
#ifndef HTTP_REQUEST_OPERATOR_H
#define HTTP_REQUEST_OPERATOR_H


#include <string>
#include "nlohmann/json.hpp"  // Include the nlohmann JSON library

using Json = nlohmann::json;

class HttpRequestOperator {
public:
    // Constructor that takes the URL
    explicit HttpRequestOperator(const std::string& url);
    ~HttpRequestOperator();

    static HttpRequestOperator *get_instance(const std::string& url);
    void query_llama_text(const std::string& prompt);

private:
    std::string url_;
    std::string _responseBuffer;
    static HttpRequestOperator* _instance;
    std::string parse_response_llm(const std::string &response); 
    static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string& responseBuffer);
    
};

#endif //HTTP_REQUEST_OPERATOR_H