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

#include "remote_control_state.h"
#include "../logger.h"
#include "robot.h"

// AutonomousState
RemoteControlState::RemoteControlState(std::string n, State* parent) : State(n, parent){}
std::optional<State*> RemoteControlState::onEnter(const ControlEvent&) {
    INFO("onEnter RemoteControlState");
    CompassController::get_instance()->setEnable(true);
    DistanceController::get_instance()->setEnable(true);
    PowerController::get_instance()->setEnable(true);
    CompassController::get_instance()->setEnable(true);
    ServoMotorController::get_instance()->setEnable(true);
    return stayOnThisState();
}

std::optional<State*> RemoteControlState::onExit(const ControlEvent&) {
    INFO("onExit RemoteControlState");
    return stayOnThisState();
}

std::optional<State*> RemoteControlState::onEvent(const ControlEvent& ev) {

    switch (ev.type) {
        case EventType::control: {
            _parentState->_lastEventTime = std::chrono::steady_clock::now();
            static_cast<Robot*>(_machine)->control_motion(ev.controlData);
            return stayOnThisState();
        }
        default:
            INFO("doesn't find the event in RemoteControlState");
            return stayOnThisState();
    }
}