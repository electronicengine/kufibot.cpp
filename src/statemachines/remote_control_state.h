//
// Created by ybulb on 8/10/2025.
//

#ifndef REMOTE_CONTROL_STATE_H
#define REMOTE_CONTROL_STATE_H

#include "state.h"

struct RemoteControlState : State {
    RemoteControlState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};

#endif //REMOTE_CONTROL_STATE_H
