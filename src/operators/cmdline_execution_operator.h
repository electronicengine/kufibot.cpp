#ifndef CMDLINEEXECUTIONOPERATOR_H
#define CMDLINEEXECUTIONOPERATOR_H

#include <string>
#include "operator.h"

enum class ExecutionType {
    query,
    imageQuery,
    tunning
};

class CmdLineExecutionOperator : public Operator {
public:
    static CmdLineExecutionOperator* get_instance();
    ~CmdLineExecutionOperator() = default;

    bool initialize() override;
    void shutdown() override;
    bool isReady() const noexcept override;

    std::string run(const std::string path, const std::string& prompt = "");
    std::string execute(ExecutionType Type, const std::string& prompt);
    void set_venv(const std::string &venv);
private:
    CmdLineExecutionOperator();


    std::string _venv;  
    static CmdLineExecutionOperator* _instance;
};

#endif // CMDLINEEXECUTIONOPERATOR_H