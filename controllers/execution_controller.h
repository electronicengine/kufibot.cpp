#ifndef EXECUTION_CONTROLLER_H
#define EXECUTION_CONTROLLER_H

#include <string>

enum class ExecutionType {
    query,
    imageQuery,
    tunning
};

class ExecutionController {
public:
    static ExecutionController* get_instance();
    ~ExecutionController() = default;

    // Method to run a Python script with a given prompt
    std::string execute(ExecutionType Type, const std::string& prompt);
    void set_venv(const std::string &venv);
private:
    ExecutionController();
    std::string run(const std::string path, const std::string& prompt);


    std::string _venv;  
    static ExecutionController* _instance; 
};

#endif // EXECUTION_CONTROLLER_H