//
// Created by ybulb on 8/9/2025.

#include "robot.h"

// Robot
Robot::Robot() {
    idleState = createState<IdleState>("IdleState");
    movingState = createState<MovingState>("MovingState");
    tuiControlState = createState<TuiControlState>("TuiControlState", movingState);
    remoteControlState = createState<RemoteControlState>("RemoteControlState", movingState);

    movingState->setTimeout(2000);
    transition(idleState, ControlEvent());
}



void Robot::dumpActivePath() {
    std::vector<std::string> path;
    State* s = _currentState;
    while (s) { path.push_back(s->_name); s = s->_parentState; }
    std::cout << "Active path: ";
    for (size_t i = 0; i < path.size(); i++) {
        std::cout << path[i];
        if (i + 1 < path.size()) std::cout << " -> ";
    }
    std::cout << "\n";
}