#ifndef TRACKING_STATE_H
#define TRACKING_STATE_H


#include "state.h"


struct TrackingState : State {
    TrackingState(std::string n, State* parent = nullptr);
    std::optional<State*> onEnter(const ControlEvent& ev) override;
    std::optional<State*> onExit(const ControlEvent& ev) override;
    std::optional<State*> onEvent(const ControlEvent& ev) override;
};




#endif //TALKING_STATE_H