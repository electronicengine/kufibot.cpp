//
// Created by ybulb on 8/10/2025.
//

#include "moving_state.h"
#include "tui_control_state.h"
#include "idle_state.h"
#include "critical_error_state.h"
#include "remote_control_state.h"
#include "robot.h"
#include "../logger.h"

// MovingState
MovingState::MovingState(std::string n, State* parent) : State(n, parent) {
}

std::optional<State*> MovingState::onEnter(const ControlEvent& ev) {
    INFO("onEnter MovingState");
    INFO("started ms timeout {}", _timeoutMs);
    static_cast<Robot*>(_machine)->postDelayedEvent(ControlEvent(EventType::timeout), _timeoutMs);
    switch (ev.source) {
        case SourceService::tuiService:
            return static_cast<Robot*>(_machine)->transState<TuiControlState>();
        case SourceService::remoteConnectionService:
            return static_cast<Robot*>(_machine)->transState<RemoteControlState>();
        default:
            return std::optional<State*>();
    }

}

std::optional<State*> MovingState::onExit(const ControlEvent&) {
    INFO("onExit MovingState");
    return std::optional<State*>();

}

std::optional<State*> MovingState::onEvent(const ControlEvent& ev) {

    switch (ev.type) {
        case EventType::critical_error:
            INFO("onEvent MovingState critical_error");
            return _machine->transState<CriticalErrorState>();

        case EventType::timeout: {
            INFO("timeout of MovingState");
            auto now = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastEventTime);
            if (diff.count() <= _timeoutMs - 10) {
                INFO("skip timeout last - now = {} ms", diff.count());
                _machine->postDelayedEvent(ControlEvent(EventType::timeout), _timeoutMs);
                return std::optional<State*>();
            }
            return static_cast<Robot*>(_machine)->transState<IdleState>();
        }
        case EventType::stop: {
            INFO("onEvent MovingState stop");
            return static_cast<Robot*>(_machine)->transState<IdleState>();
        }
        default:
            return std::optional<State*>();
    }
}
