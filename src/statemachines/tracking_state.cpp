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


#include "tracking_state.h"
#include "tui_control_state.h"
#include "idle_state.h"
#include "critical_error_state.h"
#include "remote_control_state.h"
#include "robot.h"
#include "../logger.h"

// MovingState
TrackingState::TrackingState(std::string n, State* parent) : State(n, parent) {
}

std::optional<State*> TrackingState::onEnter(const ControlEvent& ev) {
 INFO("onEnter TrackingState");
 CompassController::get_instance()->setEnable(false);
 DistanceController::get_instance()->setEnable(false);
 PowerController::get_instance()->setEnable(false);
 CompassController::get_instance()->setEnable(false);
 ServoMotorController::get_instance()->setEnable(true);

 return stayOnThisState();

}

std::optional<State*> TrackingState::onExit(const ControlEvent&) {
 INFO("onExit TrackingState");
 return stayOnThisState();

}

std::optional<State*> TrackingState::onEvent(const ControlEvent& ev) {

 if (ev.source == SourceService::landmarkTrackerService || ev.source == SourceService::gesturePerformerService) {
     INFO("onEvent TrackingState");
     switch (ev.type) {
         case EventType::control: {
             INFO("tracking");
             _parentState->_lastEventTime = std::chrono::steady_clock::now();
             static_cast<Robot*>(_machine)->control_motion(ev.controlData);
             return stayOnThisState();
         }
         default:
             INFO("doesn't find the event in TuiControlState");
             return stayOnThisState();
     }

 }else {
     WARNING("The Service Source is not apropriate with state");
     return stayOnThisState();
 }

}

