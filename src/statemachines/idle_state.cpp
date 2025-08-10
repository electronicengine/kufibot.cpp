//
// Created by ybulb on 8/10/2025.
//

#include "idle_state.h"
#include "moving_state.h"
#include "../logger.h"
#include "robot.h"

// IdleState
IdleState::IdleState(std::string n, State* parent): State(n, parent) {}

std::optional<State*> IdleState::onEnter(const ControlEvent&) {
    INFO("onEnter IdleState");
    return std::optional<State*>();
}

std::optional<State*> IdleState::onExit(const ControlEvent&) {
    INFO("onExit IdleState");
    return std::optional<State*>();
}

std::optional<State*> IdleState::onEvent(const ControlEvent& ev) {
    INFO("onEvent IdleState");

    switch (ev.type) {
        case EventType::control:{
            INFO("Transitioning to Moving state from Idle State");
            return static_cast<Robot*>(_machine)->transState<MovingState>();
        }
        default:
            return std::optional<State*>();
    }
}
