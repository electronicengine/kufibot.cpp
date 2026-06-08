#ifndef DRIVER_H
#define DRIVER_H

#include <string>
#include <utility>

class Driver {
public:
    virtual ~Driver() = default;

    const std::string& getName() const noexcept {
        return _name;
    }

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isReady() const noexcept = 0;

protected:
    explicit Driver(std::string name) : _name(std::move(name)) {}

private:
    std::string _name;
};

#endif // DRIVER_H
