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
        go,
        come,
        stop,
        look,
        turnAround,
        standBack
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



#endif //GESTURE_DEFS_H
