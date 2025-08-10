//
// Created by ybulb on 8/10/2025.
//

#ifndef TUI_CONTROL_STATE_H
#define TUI_CONTROL_STATE_H

#include "state.h"


struct TuiControlState : State {
    TuiControlState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};



#endif //TUI_CONTROL_STATE_H
