//
// Created by ybulb on 8/3/2025.
//

#include "state_machine.h"

template<typename StateType, typename EventType>
StateMachine<StateType, EventType>::StateMachine(StateType initialState, const std::string &name)
        : _currentState(initialState), _name(name){
    _lastStateChangeTime = std::chrono::steady_clock::now();
}

template<typename StateType, typename EventType>
StateMachine<StateType, EventType>::~StateMachine() {
}
