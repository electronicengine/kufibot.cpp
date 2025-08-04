#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <map>
#include <mutex>
#include <chrono>
#include <string>
#include <functional>
#include <memory>
#include "logger.h"

template<typename StateType, typename EventType>
class StateMachine {
public:
    // Type definitions
    using StateHandler = std::function<void()>;
    using StateEntryHandler = std::function<void(StateType newState, StateType oldState)>;
    using StateExitHandler = std::function<void(StateType oldState)>;
    using TransitionValidator = std::function<bool(StateType from, StateType to, EventType event)>;
    using TimeoutHandler = std::function<void(StateType state, std::chrono::milliseconds duration)>;

    // Constructor
    explicit StateMachine(StateType initialState, const std::string& name = "StateMachine");

    virtual ~StateMachine();

    // Configuration methods
    void add_transition(StateType from, EventType event, StateType to) {
        std::lock_guard<std::mutex> lock(_configMutex);
        _transitions[from][event] = to;
    }

    void add_state_handler(StateType state, StateHandler handler) {
        std::lock_guard<std::mutex> lock(_configMutex);
        _stateHandlers[state] = handler;
    }

    void set_state_entry_handler(StateEntryHandler handler) {
        std::lock_guard<std::mutex> lock(_configMutex);
        _onStateEntry = handler;
    }

    void set_state_exit_handler(StateExitHandler handler) {
        std::lock_guard<std::mutex> lock(_configMutex);
        _onStateExit = handler;
    }

    void set_transition_validator(TransitionValidator validator) {
        std::lock_guard<std::mutex> lock(_configMutex);
        _transitionValidator = validator;
    }

    void set_state_timeout(StateType state, std::chrono::milliseconds timeout, TimeoutHandler handler) {
        std::lock_guard<std::mutex> lock(_configMutex);
        _stateTimeouts[state] = timeout;
        _timeoutHandlers[state] = handler;
    }

    // State transition
    bool trigger_event(EventType event) {
        std::lock_guard<std::mutex> lock(_stateMutex);

        auto stateIt = _transitions.find(_currentState);
        if (stateIt == _transitions.end()) {
            WARNING("[{}] No transitions defined for state: {}", _name, state_to_string(_currentState));
            return false;
        }

        auto eventIt = stateIt->second.find(event);
        if (eventIt == stateIt->second.end()) {
            WARNING("[{}] Invalid transition from {} with event {}",
                    _name, state_to_string(_currentState), event_to_string(event));
            return false;
        }

        StateType newState = eventIt->second;

        // Validate transition if validator is set
        if (_transitionValidator && !_transitionValidator(_currentState, newState, event)) {
            WARNING("[{}] Transition validation failed: {} -> {} (event: {})",
                    _name, state_to_string(_currentState), state_to_string(newState), event_to_string(event));
            return false;
        }

        return transition_to_state(newState, event);
    }

    // State queries
    StateType get_current_state() const {
        std::lock_guard<std::mutex> lock(_stateMutex);
        return _currentState;
    }

    std::chrono::milliseconds get_time_in_current_state() const {
        std::lock_guard<std::mutex> lock(_stateMutex);
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastStateChangeTime);
    }

    bool is_in_state(StateType state) const {
        std::lock_guard<std::mutex> lock(_stateMutex);
        return _currentState == state;
    }

    // Update method - should be called regularly
    void update() {
        std::lock_guard<std::mutex> lock(_stateMutex);


        // Execute state handler
        auto handlerIt = _stateHandlers.find(_currentState);
        if (handlerIt != _stateHandlers.end() && handlerIt->second) {
            handlerIt->second();
        }

        // Check for state timeout
        check_state_timeout();
    }

    // Utility methods - can be overridden by derived classes
    virtual std::string state_to_string(StateType state) const {
        return "State_" + std::to_string(static_cast<int>(state));
    }

    virtual std::string event_to_string(EventType event) const {
        return "Event_" + std::to_string(static_cast<int>(event));
    }

    // Get statistics
    std::map<StateType, int> get_state_visit_counts() const {
        std::lock_guard<std::mutex> lock(_stateMutex);
        return _stateVisitCounts;
    }

    int get_transition_count() const {
        std::lock_guard<std::mutex> lock(_stateMutex);
        return _transitionCount;
    }

protected:
    // Internal transition method
    bool transition_to_state(StateType newState, EventType triggerEvent) {
        StateType oldState = _currentState;

        INFO("[{}] State transition: {} -> {} (event: {})",
             _name, state_to_string(oldState), state_to_string(newState), event_to_string(triggerEvent));

        // Call exit handler
        if (_onStateExit) {
            _onStateExit(oldState);
        }

        // Update state
        _currentState = newState;
        _lastStateChangeTime = std::chrono::steady_clock::now();
        _stateVisitCounts[newState]++;
        _transitionCount++;

        // Call entry handler
        if (_onStateEntry) {
            _onStateEntry(newState, oldState);
        }

        return true;
    }

    // Check for state timeouts
    void check_state_timeout() {
        auto timeoutIt = _stateTimeouts.find(_currentState);
        if (timeoutIt != _stateTimeouts.end()) {
            auto timeInState = get_time_in_current_state();
            if (timeInState >= timeoutIt->second) {
                auto handlerIt = _timeoutHandlers.find(_currentState);
                if (handlerIt != _timeoutHandlers.end() && handlerIt->second) {
                    handlerIt->second(_currentState, timeInState);
                }
            }
        }
    }

private:
    // State machine data
    StateType _currentState;
    std::string _name;

    // Timing
    std::chrono::steady_clock::time_point _lastStateChangeTime;

    // Configuration
    std::map<StateType, std::map<EventType, StateType>> _transitions;
    std::map<StateType, StateHandler> _stateHandlers;
    std::map<StateType, std::chrono::milliseconds> _stateTimeouts;
    std::map<StateType, TimeoutHandler> _timeoutHandlers;

    // Handlers
    StateEntryHandler _onStateEntry;
    StateExitHandler _onStateExit;
    TransitionValidator _transitionValidator;

    // Statistics
    std::map<StateType, int> _stateVisitCounts;
    int _transitionCount = 0;

    // Thread safety
    mutable std::mutex _stateMutex;
    mutable std::mutex _configMutex;
};


// Convenience macros for state machine setup
#define SETUP_STATE_MACHINE(SM, INITIAL_STATE) \
    SM.start();

#define ADD_STATE_TRANSITION(SM, FROM, EVENT, TO) \
    SM.add_transition(FROM, EVENT, TO);

#define ADD_STATE_HANDLER(SM, STATE, HANDLER) \
    SM.add_state_handler(STATE, [this]() { HANDLER(); });

#define SET_STATE_TIMEOUT(SM, STATE, TIMEOUT_MS, HANDLER) \
    SM.set_state_timeout(STATE, std::chrono::milliseconds(TIMEOUT_MS), \
                        [this](auto state, auto duration) { HANDLER(state, duration); });

#endif // STATE_MACHINE_H


