
#include "state.h"
#include "robot.h"

#include "../logger.h"

State::State(std::string n, State* parent) : _name(n), _parentState(parent) {
}

State::~State() {
}

