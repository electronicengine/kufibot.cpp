
#include "state_machine.h"
#include "state.h"
#include "event.h"
#include "../logger.h"

StateMachine::StateMachine() {
}

StateMachine::~StateMachine() {
}

void StateMachine::start() {
    _running = true;
    _worker = std::thread([this](){ loop(); });
    INFO("State machine is started!");
}

void StateMachine::stop() {
    if (!_running) return;
    _running = false;
    _cv.notify_all();

    if (_worker.joinable())
        _worker.join();
    INFO("State machine is stopped!");

}

void StateMachine::postEvent(const ControlEvent& ev) {
    {
        std::lock_guard<std::mutex> lock(_mtx);
        _eventQueue.push(ev);
    }
    _cv.notify_one();
}


void StateMachine::postDelayedEvent(const ControlEvent& ev, int delay_ms) {
    if (delay_ms <= 0) { postEvent(ev); return; }
    auto when = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay_ms);
    {
        std::lock_guard<std::mutex> lock(_timerMtx);
        _timers.emplace(when, ev);
    }
    _cv.notify_one();
}


void StateMachine::loop() {
    while (_running) {
        ControlEvent ev;
        bool haveEvent = false;
        {
            std::lock_guard<std::mutex> tlock(_timerMtx);
            auto now = std::chrono::steady_clock::now();
            auto it = _timers.begin();
            while (it != _timers.end() && it->first <= now) {
                postEvent(it->second);
                it = _timers.erase(it);
            }

        }

        {
            std::unique_lock<std::mutex> lock(_mtx);
            if (_eventQueue.empty()) {
                std::chrono::steady_clock::time_point nextWake;
                bool hasNext = false;
                {
                    std::lock_guard<std::mutex> tlock(_timerMtx);
                    if (!_timers.empty()) {
                        nextWake = _timers.begin()->first;
                        hasNext = true;
                    }
                }
                if (hasNext) {
                    _cv.wait_until(lock, nextWake, [this] {
                        return !_eventQueue.empty() || !_running;
                    });
                } else {
                    _cv.wait(lock, [this] {
                        return !_eventQueue.empty() || !_running;
                    });
                }
            }
            if (!_eventQueue.empty()) {
                ev = _eventQueue.front();
                _eventQueue.pop();
                haveEvent = true;
            }
        }

        if (haveEvent) {
            dispatch(ev);
        }
    }
}

void StateMachine::dispatch(const ControlEvent& ev) {
    State* s = _currentState;
    while (s) {
        auto result = s->onEvent(ev);
        if (result.has_value()) {
            if (result.value() == s) {
                return;
            }else {
                transition(result.value(), ev);
                return;
            }
        }
        else {
            s = s->_parentState;
        }
    }

}


void StateMachine::transition(State* toState, const ControlEvent& cause) {
    if (!toState) return;

    auto ancestors = [](State* s) {
        std::vector<State*> chain;
        while (s) {
            chain.push_back(s);
            s = s->_parentState;
        }
        return chain;
    };

    std::vector<State*> fromChain = ancestors(_currentState);
    std::vector<State*> toChain   = ancestors(toState);

    State* lca = nullptr;
    size_t i = 0;
    while (i < fromChain.size() && i < toChain.size() &&
           fromChain[fromChain.size()-1-i] == toChain[toChain.size()-1-i]) {
        lca = fromChain[fromChain.size()-1-i];
        ++i;
    }

    for (State* s : fromChain) {
        if (s == lca) break;
        s->onExit(cause);
    }

    std::vector<State*> enterList;
    for (auto it = toChain.rbegin(); it != toChain.rend(); ++it) {
        if (*it == lca) break;
        enterList.push_back(*it);
    }
    if (!lca) {
        enterList.clear();
        for (auto it = toChain.rbegin(); it != toChain.rend(); ++it) enterList.push_back(*it);
    }

    for (auto s : enterList) {
        auto result = s->onEnter(cause);
        //for transition if available
        if (result.has_value()) {
            if (result.value() == s) {
                continue;
            }else {
                result.value()->onEnter(cause);
                toState = result.value();
            }
        }
    }
    _currentState = toState;
    _currentState->onEvent(cause);
}

