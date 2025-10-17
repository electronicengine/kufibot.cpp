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

// MovingState
MovingState::MovingState(std::string n, State* parent) : State(n, parent) {
}

std::optional<State*> MovingState::onEnter(const ControlEvent& ev) {
    INFO("onEnter MovingState");
    INFO("started ms timeout {}", _timeoutMs);

    static_cast<Robot*>(_machine)->setEnableSensorContinuousReadings(false);
    //add here sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    addPostDelayedEvent(ControlEvent(EventType::timeout), _timeoutMs);
    switch (ev.source) {
        case SourceService::tuiService:
            return transTo<TuiControlState>();
        case SourceService::remoteConnectionService:
            return transTo<RemoteControlState>();
        case SourceService::gesturePerformerService:
            return transTo<TalkingState>();
        case SourceService::landmarkTrackerService:
            return transTo<TalkingState>();
        default:
            return stayOnThisState();
    }

}

std::optional<State*> MovingState::onExit(const ControlEvent&) {
    INFO("onExit MovingState");
    return stayOnThisState();

}

std::optional<State*> MovingState::onEvent(const ControlEvent& ev) {

    switch (ev.type) {
        case EventType::critical_error:
            INFO("onEvent MovingState critical_error");
            return transTo<CriticalErrorState>();

        case EventType::timeout: {
            INFO("timeout of MovingState");
            auto now = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastEventTime);
            if (diff.count() <= _timeoutMs - 10) {
                INFO("skip timeout last - now = {} ms", diff.count());
                addPostDelayedEvent(ControlEvent(EventType::timeout), _timeoutMs);
                return stayOnThisState();
            }
            return transTo<IdleState>();
        }
        case EventType::stop: {
            INFO("onEvent MovingState stop");
            return transTo<IdleState>();
        }
        default:
            return stayOnThisState();
    }
}
