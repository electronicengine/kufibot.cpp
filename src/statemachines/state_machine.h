//
// Created by ybulb on 8/9/2025.
//

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <iostream>
#include "event.h"

struct State;
// --------------------------
// State Machine class
// --------------------------
struct StateMachine {
    std::mutex _mtx;
    std::condition_variable _cv;
    std::queue<ControlEvent> _eventQueue;
    std::atomic<bool> _running{false};
    std::thread _worker;

    std::mutex _timerMtx;
    std::multimap<std::chrono::steady_clock::time_point, ControlEvent> _timers;
    State* _initialState = nullptr;
    State* _currentState = nullptr;
    std::vector<std::unique_ptr<State>> states;

    StateMachine();
    virtual ~StateMachine();

    template<typename T, typename... Args>
    T* createState(Args&&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = ptr.get();
        raw->_machine = this;
        states.emplace_back(std::move(ptr));
        return raw;
    }

    template<typename T>
    T* transState() {
        for (auto& statePtr : states) {
            if (typeid(*statePtr) == typeid(T)) {
                return static_cast<T*>(statePtr.get());
            }
        }
        return nullptr;
    }

    template<typename T>
    bool isInState() {
        if (typeid(*_currentState) == typeid(T)) {
            return true;
        }else {
            return false;
        }
    }

    void start();
    void stop();
    void postEvent(const ControlEvent& ev);
    void setInitialState(State* state);
    void postDelayedEvent(const ControlEvent& ev, int delay_ms);
    void transition(State* toState, const ControlEvent& cause = ControlEvent());

private:
    void loop();
    void dispatch(const ControlEvent& ev);
};

#endif //STATE_MACHINE_H
