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

#include "tui_control_state.h"

#include "robot.h"
#include "../logger.h"

// TuiControlState
TuiControlState::TuiControlState(std::string n, State* parent) : State(n, parent) {}
std::optional<State*> TuiControlState::onEnter(const ControlEvent&) {
    INFO("onEnter TuiControlState");
    return stayOnThisState();

}

std::optional<State*> TuiControlState::onExit(const ControlEvent&) {
    INFO("onExit TuiControlState");
    return stayOnThisState();
}

std::optional<State*> TuiControlState::onEvent(const ControlEvent& ev) {
    if (ev.source != SourceService::tuiService) {
        WARNING("The Service Source is not apropriate with state");
        return stayOnThisState();
    }

    INFO("onEvent TuiControlState");
    switch (ev.type) {
        case EventType::control: {
            INFO("control tui");
            _parentState->_lastEventTime = std::chrono::steady_clock::now();
            static_cast<Robot*>(_machine)->control_motion(ev.controlData);
            return stayOnThisState();
        }
        default:
            INFO("doesn't find the event in TuiControlState");
            return stayOnThisState();
    }
}
