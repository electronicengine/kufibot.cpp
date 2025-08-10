//
// Created by ybulb on 8/9/2025.
//

#ifndef ROBOT_H
#define ROBOT_H

#include "state_machine.h"
#include "state.h"

struct Robot : StateMachine {

    IdleState* idleState = nullptr;
    MovingState* movingState = nullptr;
    TuiControlState* tuiControlState = nullptr;
    RemoteControlState* remoteControlState = nullptr;

    Robot();
    void dumpActivePath();
};


#endif //ROBOT_H
