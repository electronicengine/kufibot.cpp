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

#include "critical_error_state.h"
#include "../logger.h"

CriticalErrorState::CriticalErrorState(std::string n, State* parent) : State(n, parent) {}
std::optional<State*> CriticalErrorState::onEnter(const ControlEvent& ev) {
    INFO("onEnter CriticalErrorState");
    return std::optional<State*>();

}
std::optional<State*> CriticalErrorState::onExit(const ControlEvent&) {
    INFO("onExit CriticalErrorState");
    return std::optional<State*>();

}

std::optional<State*> CriticalErrorState::onEvent(const ControlEvent& ev) {
    INFO("onEvent CriticalErrorState");
    return std::optional<State*>();
}