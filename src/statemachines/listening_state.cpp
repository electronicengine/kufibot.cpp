
#include "listening_state.h"

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

#include "moving_state.h"
#include "tui_control_state.h"
#include "idle_state.h"
#include "critical_error_state.h"
#include "remote_control_state.h"
#include "robot.h"
#include "tracking_state.h"
#include "../logger.h"

// ListeningState
ListeningState::ListeningState(std::string n, State* parent) : State(n, parent) {
}

std::optional<State*> ListeningState::onEnter(const ControlEvent& ev) {
    INFO("onEnter ListeningState");
    INFO("started ms timeout {}", _timeoutMs);

    addPostDelayedEvent(ControlEvent(EventType::timeout), 4000);

    return stayOnThisState();
}

std::optional<State*> ListeningState::onExit(const ControlEvent&) {
    INFO("onExit ListeningState");
    return stayOnThisState();

}

std::optional<State*> ListeningState::onEvent(const ControlEvent& ev) {

    switch (ev.type) {
        case EventType::critical_error:
            INFO("onEvent ListeningState critical_error");
            return transTo<CriticalErrorState>();

        case EventType::control:
            INFO("onEvent ListeningState control");
            clearDelayedEvents();
            return transTo<MovingState>();

        case EventType::timeout: {
            INFO("timeout of ListeningState");
            auto now = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastEventTime);
            if (diff.count() <= _timeoutMs - 10) {
                INFO("skip timeout last - now = {} ms", diff.count());
                addPostDelayedEvent(ControlEvent(EventType::timeout), 4000);
                return stayOnThisState();
            }
            return transTo<IdleState>();
        }
        case EventType::stop: {
            INFO("onEvent MovingState stop");
            clearDelayedEvents();
            return transTo<IdleState>();
        }
        default:
            return stayOnThisState();
    }
}
