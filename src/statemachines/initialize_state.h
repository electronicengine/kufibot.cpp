//
// Created by ybulb on 8/10/2025.
//

#ifndef INITIALIZE_STATE_H
#define INITIALIZE_STATE_H


#include "state.h"

struct InitializeState : State {
    InitializeState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};



#endif //INITIALIZE_STATE_H
