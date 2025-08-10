
#include "remote_control_state.h"
#include "../logger.h"

// AutonomousState
RemoteControlState::RemoteControlState(std::string n, State* parent) : State(n, parent){}
std::optional<State*> RemoteControlState::onEnter(const ControlEvent&) {
    INFO("onEnter RemoteControlState");
    return std::optional<State*>();
}

std::optional<State*> RemoteControlState::onExit(const ControlEvent&) {
    INFO("onExit RemoteControlState");
    return std::optional<State*>();
}

std::optional<State*> RemoteControlState::onEvent(const ControlEvent& ev) {
    if (ev.source != SourceService::remoteConnectionService) {
        WARNING("The Service Source is not apropriate with state");
        return std::optional<State*>();
    }

    INFO("onEvent RemoteControlState");
    switch (ev.type) {
        case EventType::control: {
            INFO("control remote");
            _parentState->_lastEventTime = std::chrono::steady_clock::now();
            return this;
        }
        default:
            INFO("doesn't find the event in RemoteControlState");
            return std::optional<State*>();
    }

}