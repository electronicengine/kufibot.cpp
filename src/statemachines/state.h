//
// Created by ybulb on 8/9/2025.
//

#ifndef STATE_H
#define STATE_H

#include "state_machine.h"
#include "event.h"

struct State {

    std::string _name;
    State* _parentState = nullptr;
    StateMachine* _machine = nullptr;
    std::chrono::steady_clock::time_point _lastEventTime;
    int _timeoutMs = 0;

    State() = default;
    State(std::string n, State* parent = nullptr);
    virtual ~State();

    void setTimeout(int timeoutMs){_timeoutMs = timeoutMs;}
    virtual std::optional<State*> onEnter(const ControlEvent& ev) = 0;
    virtual std::optional<State*> onEvent(const ControlEvent& ev) = 0;
    virtual std::optional<State*> onExit(const ControlEvent& ev) = 0;

};

struct TuiControlState : State {
    TuiControlState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};

struct RemoteControlState : State {
    RemoteControlState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};

struct IdleState : State {
    IdleState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};

struct MovingState : State {
    int timeout_ms = 3000;

    MovingState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};

struct CriticalState : State {
    int timeout_ms = 3000;

    CriticalState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};





#endif //STATE_H
