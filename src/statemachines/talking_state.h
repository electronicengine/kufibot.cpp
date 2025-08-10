//
// Created by ybulb on 8/10/2025.
//

#ifndef TALKING_STATE_H
#define TALKING_STATE_H


#include "state.h"


struct TalkingState : State {
    TalkingState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};




#endif //TALKING_STATE_H
