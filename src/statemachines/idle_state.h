//
// Created by ybulb on 8/10/2025.
//

#ifndef IDLE_STATE_H
#define IDLE_STATE_H

#include "state.h"


struct IdleState : State {
    IdleState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};



#endif //IDLE_STATE_H
