//
// Created by ybulb on 8/10/2025.
//

#ifndef MOVING_STATE_H
#define MOVING_STATE_H

#include "state.h"

struct MovingState : State {
    int timeout_ms = 3000;

    MovingState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};

#endif //MOVING_STATE_H
