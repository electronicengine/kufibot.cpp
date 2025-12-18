#ifndef GESTURE_DEFS_H
#define GESTURE_DEFS_H
#include <list>
#include <string>
#include "controllers/controller_data_structures.h"
#include <vector>


    enum class GestureJointState {
        fulldown,
        fullup,
        halfdown,
        halfup,
        middle
    };
    struct GestureJointAngle {
        int angle;
        std::string symbol;
    };

    enum class EmotionType{
        happy,
        angry,
        funny,
        serious,
        curious,
        worried,
        surprised,
        confident,
    };

    enum class ReactionType{
        greeting,
        listening,
        talking,
        accepting,
        rejecting,
        thinking,
        agreeing
    };

    enum class DirectiveType {
        followFinger,
        stopFollow,
    };

    struct MotionSequenceItem {
        int time; // milliseconds from start
        std::map<ServoMotorJoint, GestureJointState> joints;
    };

    struct EmotionalMotion {
        std::string name;
        std::string description;
        int duration; // milliseconds
        std::map<ServoMotorJoint, GestureJointState> joints;
        std::vector<MotionSequenceItem> sequence;
    };

    struct ReactionalMotion {
        std::string name;
        std::string description;
        int duration; // milliseconds
        std::map<ServoMotorJoint, GestureJointState> joints;
        std::vector<MotionSequenceItem> sequence;
    };

    struct DirectiveMotion {
        std::string name;
        std::string description;
        int duration; // milliseconds
        std::map<ServoMotorJoint, GestureJointState> joints;
        std::vector<MotionSequenceItem> sequence;
    };

    struct EmotionalGesture{
        EmotionType emotion;
        std::string symbol;
        std::string description;
        std::map<ServoMotorJoint, uint8_t> motorPos;
        std::vector<float> embedding;
    };

    struct ReactionalGesture {
        ReactionType reaction;
        ReactionType antiReaction;
        std::string symbol;
        std::string description;
        std::vector<float> embedding;

    };

    struct Directive {
        DirectiveType directive;
        std::string symbol;
        std::string description;
        std::vector<float> embedding;
    };

    struct FaceReaction {
        std::string emotion;
        std::string reaction;
    };



#endif //GESTURE_DEFS_H
