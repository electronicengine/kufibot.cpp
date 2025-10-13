//
// Created by ybulb on 10/10/2025.
//

#ifndef LISTENINGSTATE_H
#define LISTENINGSTATE_H

#include "state.h"

struct ListeningState : State {
    int timeout_ms = 3000;

    ListeningState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};

#endif //LISTENINGSTATE_H
