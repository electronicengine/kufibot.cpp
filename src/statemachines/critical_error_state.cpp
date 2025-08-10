//
// Created by ybulb on 8/10/2025.
//

#include "critical_error_state.h"
#include "../logger.h"

CriticalErrorState::CriticalErrorState(std::string n, State* parent) : State(n, parent) {}
std::optional<State*> CriticalErrorState::onEnter(const ControlEvent& ev) {
    INFO("onEnter CriticalErrorState");
    return std::optional<State*>();

}
std::optional<State*> CriticalErrorState::onExit(const ControlEvent&) {
    INFO("onExit CriticalErrorState");
    return std::optional<State*>();

}

std::optional<State*> CriticalErrorState::onEvent(const ControlEvent& ev) {
    INFO("onEvent CriticalErrorState");
    return std::optional<State*>();
}