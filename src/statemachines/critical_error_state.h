//
// Created by ybulb on 8/10/2025.
//

#ifndef CRITICAL_ERROR_STATE_H
#define CRITICAL_ERROR_STATE_H

#include "state.h"

struct CriticalErrorState : State {
    int timeout_ms = 3000;

    CriticalErrorState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};




#endif //CRITICAL_ERROR_STATE_H
