#include <iostream>
#include <memory>
#include <array>
#include <cstdio> // For popen and pclose
#include <iostream>
#include <memory>
#include <array>
#include <cstdio>
#include <fcntl.h>
#include "execution_controller.h"
#include <unistd.h>  // For read, close
#include <sys/select.h>  // For select()
#include "../ui/main_window.h"

ExecutionController* ExecutionController::_instance = nullptr;

ExecutionController* ExecutionController::get_instance() {
    if (_instance == nullptr) {
        _instance = new ExecutionController();
    }
    return _instance;
}

ExecutionController::ExecutionController(){}

std::string ExecutionController::execute(ExecutionType Type, const std::string& prompt) {

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

std::string ExecutionController::run(const std::string path, const std::string& prompt) {
    std::string command = "bash -c 'python3 " + path + " \"" + prompt + "\" 2>&1'";
    MainWindow::log("Executing: " + command, LogLevel::LOG_TRACE);

    std::array<char, 256> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe) {
        MainWindow::log("Error: Failed to run the Python script.", LogLevel::LOG_ERROR);
        return "Error";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

 


void ExecutionController::set_venv(const std::string &venv){
    std::string command = "bash -c 'source " + venv + "/bin/activate'";
    MainWindow::log("Executing: " + command, LogLevel::LOG_TRACE);

    std::system(command.c_str());

    _venv = venv;
}