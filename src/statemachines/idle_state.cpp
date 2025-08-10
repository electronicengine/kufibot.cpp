/*
* This file is part of Kufibot.
 *
 * Kufibot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kufibot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kufibot. If not, see <https://www.gnu.org/licenses/>.
 */

#include "idle_state.h"
#include "moving_state.h"
#include "../logger.h"
#include "robot.h"

// IdleState
IdleState::IdleState(std::string n, State* parent): State(n, parent) {}

std::optional<State*> IdleState::onEnter(const ControlEvent&) {
    INFO("onEnter IdleState");
    return std::optional<State*>();
}

std::optional<State*> IdleState::onExit(const ControlEvent&) {
    INFO("onExit IdleState");
    return std::optional<State*>();
}

std::optional<State*> IdleState::onEvent(const ControlEvent& ev) {
    INFO("onEvent IdleState");

    switch (ev.type) {
        case EventType::control:{
            INFO("Transitioning to Moving state from Idle State");
            return static_cast<Robot*>(_machine)->transState<MovingState>();
        }
        default:
            return std::optional<State*>();
    }
}
