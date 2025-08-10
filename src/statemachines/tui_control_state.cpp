//
// Created by ybulb on 8/10/2025.
//

#include "tui_control_state.h"

#include "robot.h"
#include "../logger.h"

// TuiControlState
TuiControlState::TuiControlState(std::string n, State* parent) : State(n, parent) {}
std::optional<State*> TuiControlState::onEnter(const ControlEvent&) {
    INFO("onEnter TuiControlState");
    return std::optional<State*>();

}

std::optional<State*> TuiControlState::onExit(const ControlEvent&) {
    INFO("onExit TuiControlState");
    return std::optional<State*>();
}

std::optional<State*> TuiControlState::onEvent(const ControlEvent& ev) {
    if (ev.source != SourceService::tuiService) {
        WARNING("The Service Source is not apropriate with state");
        return std::optional<State*>();
    }

    INFO("onEvent TuiControlState");
    switch (ev.type) {
        case EventType::control: {
            INFO("control tui");
            _parentState->_lastEventTime = std::chrono::steady_clock::now();
            static_cast<Robot*>(_machine)->control_motion(ev.controlData);
            return this;
        }
        default:
            INFO("doesn't find the event in TuiControlState");
            return std::optional<State*>();
    }
}
