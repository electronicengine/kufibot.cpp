//
// Created by ulak on 04.08.2025.
//

#ifndef ROBOT_STATE_MACHINE_H
#define ROBOT_STATE_MACHINE_H


#include "state_machine.h"
#include <iostream>
#include <thread>
#include <chrono>

// Define the states and events as enums
enum class RobotState {
    IdleState,
    MovingState,
    TalkingState,
    TrackingState
};

enum class RobotEvent {
    StartMoving,
    StopMoving,
    StartTalking,
    StopTalking,
    StartTracking,
    StopTracking,
    Reset
};

// Derived Robot class from StateMachine
class Robot : public StateMachine<RobotState, RobotEvent> {
private:
    std::string robotName;
    int batteryLevel;
    std::string targetObject;

public:
    // Constructor
    Robot(const std::string& name)
        : StateMachine<RobotState, RobotEvent>(RobotState::IdleState, "Robot_" + name),
          robotName(name), batteryLevel(100) {

        setupStateMachine();
    }

    // Override string conversion methods for better logging
    std::string state_to_string(RobotState state) const override {
        switch(state) {
            case RobotState::IdleState: return "IdleState";
            case RobotState::MovingState: return "MovingState";
            case RobotState::TalkingState: return "TalkingState";
            case RobotState::TrackingState: return "TrackingState";
            default: return "UnknownState";
        }
    }

    std::string event_to_string(RobotEvent event) const override {
        switch(event) {
            case RobotEvent::StartMoving: return "StartMoving";
            case RobotEvent::StopMoving: return "StopMoving";
            case RobotEvent::StartTalking: return "StartTalking";
            case RobotEvent::StopTalking: return "StopTalking";
            case RobotEvent::StartTracking: return "StartTracking";
            case RobotEvent::StopTracking: return "StopTracking";
            case RobotEvent::Reset: return "Reset";
            default: return "UnknownEvent";
        }
    }

private:
    void setupStateMachine() {
        // Define state transitions
        setupTransitions();

        // Set up state handlers
        setupStateHandlers();

        // Set up entry/exit handlers
        setupEntryExitHandlers();

        // Set up state timeouts
        setupTimeouts();

        // Set up transition validator
        setupValidator();
    }

    void setupTransitions() {
        // From IdleState
        add_transition(RobotState::IdleState, RobotEvent::StartMoving, RobotState::MovingState);
        add_transition(RobotState::IdleState, RobotEvent::StartTalking, RobotState::TalkingState);
        add_transition(RobotState::IdleState, RobotEvent::StartTracking, RobotState::TrackingState);

        // From MovingState
        add_transition(RobotState::MovingState, RobotEvent::StopMoving, RobotState::IdleState);
        add_transition(RobotState::MovingState, RobotEvent::StartTalking, RobotState::TalkingState);
        add_transition(RobotState::MovingState, RobotEvent::StartTracking, RobotState::TrackingState);

        // From TalkingState
        add_transition(RobotState::TalkingState, RobotEvent::StopTalking, RobotState::IdleState);
        add_transition(RobotState::TalkingState, RobotEvent::StartMoving, RobotState::MovingState);
        add_transition(RobotState::TalkingState, RobotEvent::StartTracking, RobotState::TrackingState);

        // From TrackingState
        add_transition(RobotState::TrackingState, RobotEvent::StopTracking, RobotState::IdleState);
        add_transition(RobotState::TrackingState, RobotEvent::StartMoving, RobotState::MovingState);
        add_transition(RobotState::TrackingState, RobotEvent::StartTalking, RobotState::TalkingState);

        // Global reset from any state
        add_transition(RobotState::IdleState, RobotEvent::Reset, RobotState::IdleState);
        add_transition(RobotState::MovingState, RobotEvent::Reset, RobotState::IdleState);
        add_transition(RobotState::TalkingState, RobotEvent::Reset, RobotState::IdleState);
        add_transition(RobotState::TrackingState, RobotEvent::Reset, RobotState::IdleState);
    }

    void setupStateHandlers() {
        // IdleState handler
        add_state_handler(RobotState::IdleState, [this]() {
            handleIdleState();
        });

        // MovingState handler
        add_state_handler(RobotState::MovingState, [this]() {
            handleMovingState();
        });

        // TalkingState handler
        add_state_handler(RobotState::TalkingState, [this]() {
            handleTalkingState();
        });

        // TrackingState handler
        add_state_handler(RobotState::TrackingState, [this]() {
            handleTrackingState();
        });
    }

    void setupEntryExitHandlers() {
        // State entry handler
        set_state_entry_handler([this](RobotState newState, RobotState oldState) {
            std::cout << "[" << robotName << "] Entered " << state_to_string(newState)
                      << " from " << state_to_string(oldState) << std::endl;
        });

        // State exit handler
        set_state_exit_handler([this](RobotState oldState) {
            std::cout << "[" << robotName << "] Exiting " << state_to_string(oldState) << std::endl;
        });
    }

    void setupTimeouts() {
        // Set timeout for TalkingState (5 seconds)
        set_state_timeout(RobotState::TalkingState, std::chrono::milliseconds(5000),
            [this](RobotState state, std::chrono::milliseconds duration) {
                std::cout << "[" << robotName << "] Talking timeout after "
                          << duration.count() << "ms - going to idle" << std::endl;
                trigger_event(RobotEvent::StopTalking);
            });

        // Set timeout for TrackingState (10 seconds)
        set_state_timeout(RobotState::TrackingState, std::chrono::milliseconds(10000),
            [this](RobotState state, std::chrono::milliseconds duration) {
                std::cout << "[" << robotName << "] Tracking timeout after "
                          << duration.count() << "ms - target lost" << std::endl;
                trigger_event(RobotEvent::StopTracking);
            });
    }

    void setupValidator() {
        // Transition validator - prevent certain transitions based on battery level
        set_transition_validator([this](RobotState from, RobotState to, RobotEvent event) {
            // Don't allow moving if battery is too low
            if (to == RobotState::MovingState && batteryLevel < 20) {
                std::cout << "[" << robotName << "] Cannot move - battery too low ("
                          << batteryLevel << "%)" << std::endl;
                return false;
            }
            return true;
        });
    }

    // State handler implementations
    void handleIdleState() {
        // Simulate idle behavior - slow battery drain
        static int idleCounter = 0;
        if (++idleCounter % 1000 == 0) { // Every ~1000 updates
            batteryLevel = std::max(0, batteryLevel - 1);
            if (batteryLevel % 10 == 0) {
                std::cout << "[" << robotName << "] Battery: " << batteryLevel << "%" << std::endl;
            }
        }
    }

    void handleMovingState() {
        // Simulate movement - faster battery drain
        static int moveCounter = 0;
        if (++moveCounter % 500 == 0) { // Every ~500 updates
            batteryLevel = std::max(0, batteryLevel - 2);
            std::cout << "[" << robotName << "] Moving... Battery: " << batteryLevel << "%" << std::endl;
        }
    }

    void handleTalkingState() {
        // Simulate talking
        static int talkCounter = 0;
        if (++talkCounter % 750 == 0) { // Every ~750 updates
            batteryLevel = std::max(0, batteryLevel - 1);
            std::cout << "[" << robotName << "] *Speaking* Battery: " << batteryLevel << "%" << std::endl;
        }
    }

    void handleTrackingState() {
        // Simulate tracking
        static int trackCounter = 0;
        if (++trackCounter % 600 == 0) { // Every ~600 updates
            batteryLevel = std::max(0, batteryLevel - 2);
            std::cout << "[" << robotName << "] Tracking target... Battery: " << batteryLevel << "%" << std::endl;
        }
    }

public:
    // Public methods for robot control
    void setBatteryLevel(int level) { batteryLevel = std::clamp(level, 0, 100); }
    int getBatteryLevel() const { return batteryLevel; }
    void setTargetObject(const std::string& target) { targetObject = target; }

    // Convenience methods for triggering events
    bool startMoving() { return trigger_event(RobotEvent::StartMoving); }
    bool stopMoving() { return trigger_event(RobotEvent::StopMoving); }
    bool startTalking() { return trigger_event(RobotEvent::StartTalking); }
    bool stopTalking() { return trigger_event(RobotEvent::StopTalking); }
    bool startTracking() { return trigger_event(RobotEvent::StartTracking); }
    bool stopTracking() { return trigger_event(RobotEvent::StopTracking); }
    bool reset() { return trigger_event(RobotEvent::Reset); }

    void printStatus() {
        std::cout << "\n=== " << robotName << " Status ===" << std::endl;
        std::cout << "Current State: " << state_to_string(get_current_state()) << std::endl;
        std::cout << "Time in State: " << get_time_in_current_state().count() << "ms" << std::endl;
        std::cout << "Battery Level: " << batteryLevel << "%" << std::endl;
        std::cout << "Total Transitions: " << get_transition_count() << std::endl;

        auto visitCounts = get_state_visit_counts();
        std::cout << "State Visit Counts:" << std::endl;
        for (const auto& [state, count] : visitCounts) {
            std::cout << "  " << state_to_string(state) << ": " << count << std::endl;
        }
        std::cout << "==================\n" << std::endl;
    }
};

// Example usage and demonstration
int main() {
    // Create a robot instance
    Robot myRobot("R2D2");

    std::cout << "=== Robot State Machine Demo ===" << std::endl;

    // Start the robot update loop in a separate thread
    bool running = true;
    std::thread updateThread([&myRobot, &running]() {
        while (running) {
            myRobot.update();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    // Demonstrate state transitions
    myRobot.printStatus();

    std::cout << "\n--- Starting Movement ---" << std::endl;
    myRobot.startMoving();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    myRobot.printStatus();

    std::cout << "\n--- Switching to Talking ---" << std::endl;
    myRobot.startTalking();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    myRobot.printStatus();

    std::cout << "\n--- Switching to Tracking ---" << std::endl;
    myRobot.setTargetObject("Ball");
    myRobot.startTracking();
    std::this_thread::sleep_for(std::chrono::seconds(4));
    myRobot.printStatus();

    std::cout << "\n--- Testing Low Battery Scenario ---" << std::endl;
    myRobot.setBatteryLevel(15);
    myRobot.stopTracking();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    myRobot.startMoving(); // Should fail due to low battery
    myRobot.printStatus();

    std::cout << "\n--- Resetting Robot ---" << std::endl;
    myRobot.setBatteryLevel(80);
    myRobot.reset();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    myRobot.printStatus();

    // Demonstrate timeout behavior
    std::cout << "\n--- Testing Timeout (Talking for >5 seconds) ---" << std::endl;
    myRobot.startTalking();
    std::this_thread::sleep_for(std::chrono::seconds(6)); // Wait for timeout
    myRobot.printStatus();

    // Stop the update thread
    running = false;
    updateThread.join();

    std::cout << "\n=== Demo Complete ===" << std::endl;
    return 0;
}

// Compiler command:
// g++ -std=c++17 -pthread robot_state_machine.cpp -o robot_demo


#endif //ROBOT_STATE_MACHINE_H
