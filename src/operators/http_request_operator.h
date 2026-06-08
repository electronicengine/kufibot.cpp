
#ifndef HTTP_REQUEST_OPERATOR_H
#define HTTP_REQUEST_OPERATOR_H


#include <string>
#include "operator.h"
#include "nlohmann/json.hpp"  // Include the nlohmann JSON library

using Json = nlohmann::json;

class HttpRequestOperator : public Operator {
public:
    // Constructor that takes the URL
    explicit HttpRequestOperator(const std::string& url);
    ~HttpRequestOperator();

    bool initialize() override;
    void shutdown() override;
    bool isReady() const noexcept override;

    static HttpRequestOperator *get_instance(const std::string& url);
    void query_llama_text(const std::string& prompt);

private:
    std::string url_;
    std::string _responseBuffer;
    bool _curlInitialized;
    static HttpRequestOperator* _instance;
    std::string parse_response_llm(const std::string &response); 
    static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string& responseBuffer);
    
};

#endif //HTTP_REQUEST_OPERATOR_H