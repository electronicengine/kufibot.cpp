#include <iostream>
#include <memory>
#include <array>
#include <cstdio>
#include <fcntl.h>
#include "cmdline_execution_operator.h"

#include "../logger.h"

CmdLineExecutionOperator* CmdLineExecutionOperator::_instance = nullptr;

CmdLineExecutionOperator* CmdLineExecutionOperator::get_instance() {
    if (_instance == nullptr) {
        _instance = new CmdLineExecutionOperator();
    }
    return _instance;
}

CmdLineExecutionOperator::CmdLineExecutionOperator(){}

std::string CmdLineExecutionOperator::execute(ExecutionType Type, const std::string& prompt) {

    switch (Type)
    {
    case ExecutionType::query:
        return run("../gemini/query.py", prompt);
    case ExecutionType::imageQuery:
        return run("/home/kufi/workspace/kufibot.cpp/gemini/image_query.py", prompt);
    case ExecutionType::tunning:
         return run("../gemini/tunning.py", prompt);
    default:
        return run("../gemini/query.py", prompt);
    }

}

std::string CmdLineExecutionOperator::run(const std::string path, const std::string& prompt) {
    std::string command = "bash -c 'python3 " + path + " \"" + prompt + "\" 2>&1'";
    Logger::trace("Executing: {}", command);

    std::array<char, 256> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe) {
        Logger::error("Error: Failed to run the Python script.");
        return "Error";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

 


void CmdLineExecutionOperator::set_venv(const std::string &venv){
    std::string command = "bash -c 'source " + venv + "/bin/activate'";
    Logger::trace("Executing: {}", command);

    std::system(command.c_str());

    _venv = venv;
}