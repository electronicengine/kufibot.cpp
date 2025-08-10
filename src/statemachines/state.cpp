
#include "state.h"

#include "../logger.h"

State::State(std::string n, State* parent) : _name(n), _parentState(parent) {
}

State::~State() {
}

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
        case EventType::control_wheels:
        case EventType::control_head:
        case EventType::control_joints: {
            INFO("Transitioning to Moving state from Idle State");
            return _machine->transState<MovingState>();
        }
        default:
            return std::optional<State*>();
    }
}

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
    if (ev.source != SourceService::tuiControlService) {
        WARNING("The Service Source is not apropriate with state");
        return std::optional<State*>();
    }

    INFO("onEvent TuiControlState");
    switch (ev.type) {
        case EventType::control_wheels:
        case EventType::control_head:
        case EventType::control_joints: {
            INFO("control tui");
            _parentState->_lastEventTime = std::chrono::steady_clock::now();
            return this;
        }
        default:
            INFO("doesn't find the event in TuiControlState");
            return std::optional<State*>();
    }
}

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
    if (ev.source != SourceService::remoteControlService) {
        WARNING("The Service Source is not apropriate with state");
        return std::optional<State*>();
    }

    INFO("onEvent RemoteControlState");
    switch (ev.type) {
        case EventType::control_wheels:
        case EventType::control_head:
        case EventType::control_joints: {
            INFO("control remote");
            _parentState->_lastEventTime = std::chrono::steady_clock::now();
            return this;
        }
        default:
            INFO("doesn't find the event in RemoteControlState");
            return std::optional<State*>();
    }

}

// MovingState
MovingState::MovingState(std::string n, State* parent) : State(n, parent) {
}

std::optional<State*> MovingState::onEnter(const ControlEvent& ev) {
    INFO("onEnter MovingState");
    INFO("started ms timeout {}", _timeoutMs);
    _machine->postDelayedEvent(ControlEvent(EventType::timeout), _timeoutMs);
    switch (ev.source) {
        case SourceService::tuiControlService:
            return _machine->transState<TuiControlState>();
        case SourceService::remoteControlService:
            return _machine->transState<RemoteControlState>();
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
            return _machine->transState<CriticalState>();

        case EventType::timeout: {
            INFO("timeout of MovingState");
            auto now = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastEventTime);
            if (diff.count() <= _timeoutMs - 10) {
                INFO("skip timeout last - now = {} ms", diff.count());
                _machine->postDelayedEvent(ControlEvent(EventType::timeout), _timeoutMs);
                return std::optional<State*>();
            }
            return _machine->transState<IdleState>();
        }
        case EventType::stop: {
            INFO("onEvent MovingState stop");
            return _machine->transState<IdleState>();
        }
        default:
            return std::optional<State*>();
    }
}

CriticalState::CriticalState(std::string n, State* parent) : State(n, parent) {}
std::optional<State*> CriticalState::onEnter(const ControlEvent& ev) {
    INFO("onEnter CriticalState");
    return std::optional<State*>();

}
std::optional<State*> CriticalState::onExit(const ControlEvent&) {
    INFO("onExit CriticalState");
    return std::optional<State*>();

}

std::optional<State*> CriticalState::onEvent(const ControlEvent& ev) {
    INFO("onEvent CriticalState");
    return std::optional<State*>();
}