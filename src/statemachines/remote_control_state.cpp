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

// AutonomousState
RemoteControlState::RemoteControlState(std::string n, State* parent) : State(n, parent){}
std::optional<State*> RemoteControlState::onEnter(const ControlEvent&) {
    INFO("onEnter RemoteControlState");
    return std::optional<State*>();
}

std::optional<State*> RemoteControlState::onExit(const ControlEvent&) {
    INFO("onExit RemoteControlState");
    return std::optional<State*>();
}

std::optional<State*> RemoteControlState::onEvent(const ControlEvent& ev) {
    if (ev.source != SourceService::remoteConnectionService) {
        WARNING("The Service Source is not apropriate with state");
        return std::optional<State*>();
    }

    INFO("onEvent RemoteControlState");
    switch (ev.type) {
        case EventType::control: {
            INFO("control remote");
            _parentState->_lastEventTime = std::chrono::steady_clock::now();
            return this;
        }
        default:
            INFO("doesn't find the event in RemoteControlState");
            return std::optional<State*>();
    }

}