//
// Created by ybulb on 8/10/2025.
//

#include "initialize_state.h"
#include "moving_state.h"
#include "tui_control_state.h"
#include "idle_state.h"
#include "critical_error_state.h"
#include "remote_control_state.h"
#include "robot.h"
#include "../logger.h"



// MovingState
InitializeState::InitializeState(std::string n, State* parent) : State(n, parent) {
}

std::optional<State*> InitializeState::onEnter(const ControlEvent& ev) {
    INFO("onEnter InitializeState");
    Robot* robot = static_cast<Robot*>(_machine);
    bool ret = robot->initialize();
    if (ret) {
        return _machine->transState<IdleState>();
    }
    else {
        return _machine->transState<CriticalErrorState>();
    }

}

std::optional<State*> InitializeState::onExit(const ControlEvent&) {
    INFO("onExit InitializeState");
    return std::optional<State*>();

}

std::optional<State*> InitializeState::onEvent(const ControlEvent& ev) {
    INFO("onEvent InitializeState");
    return std::optional<State*>();
}
