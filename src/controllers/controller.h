#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <atomic>
#include <string>
#include <utility>

class Controller {
public:
    virtual ~Controller() = default;

    const std::string& getName() const noexcept {
        return _name;
    }

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isReady() const noexcept = 0;

    void setEnable(bool enable) noexcept {
        _enable.store(enable);
    }

    bool isEnabled() const noexcept {
        return _enable.load();
    }

protected:
    explicit Controller(std::string name) : _name(std::move(name)) {}

    std::atomic<bool> _enable{true};
    std::atomic<bool> _initialized{false};

private:
    std::string _name;
};

#endif // CONTROLLER_H
