#ifndef OPERATOR_H
#define OPERATOR_H

#include <string>
#include <utility>

class Operator {
public:
    virtual ~Operator() = default;

    const std::string& getName() const noexcept {
        return _name;
    }

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isReady() const noexcept = 0;

protected:
    explicit Operator(std::string name) : _name(std::move(name)) {}

private:
    std::string _name;
};

#endif // OPERATOR_H
