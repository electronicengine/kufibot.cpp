#ifndef GESTURE_DEFS_H
#define GESTURE_DEFS_H
#include <list>
#include <string>


    enum class GestureType {};

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

    inline std::list Emotional_Gesture_List{
        EmotionalGesture{EmotionType::happy, "<happy>", "Feeling joyful, cheerful, and content."},
        EmotionalGesture{EmotionType::angry, "<angry>", "Feeling irritated, annoyed, or full of rage."},
        EmotionalGesture{EmotionType::funny, "<funny>", "Lighthearted, humorous, or amusing in nature."},
        EmotionalGesture{EmotionType::serious, "<serious>", "Focused, thoughtful, or deeply concerned."},
        EmotionalGesture{EmotionType::curious, "<curious>", "Eager to learn, interested, or inquisitive."},
        EmotionalGesture{EmotionType::worried, "<worried>", "Anxious, uneasy, or full of concern."},
        EmotionalGesture{EmotionType::surprised, "<surprised>", "Shocked, amazed, or caught off guard."},
        EmotionalGesture{EmotionType::confident, "<confident>", "Self-assured, assertive, and secure in actions."},
    };

    inline std::list Reactional_Gesture_List{
        ReactionalGesture{ReactionType::greeting,  ReactionType::greeting,"<greeting>", "Waving or expressing a friendly welcome to someone."},
        ReactionalGesture{ReactionType::listening, ReactionType::talking,"<listening>", "Paying close attention and showing engagement without speaking."},
        ReactionalGesture{ReactionType::talking, ReactionType::listening,"<talking>", "Speaking or expressing thoughts verbally to someone."},
        ReactionalGesture{ReactionType::accepting, ReactionType::agreeing,"<accepting>", "Showing agreement or willingness to receive an idea or offer."},
        ReactionalGesture{ReactionType::rejecting, ReactionType::thinking,"<rejecting>", "Expressing disagreement or refusal of an idea or offer."},
        ReactionalGesture{ReactionType::thinking, ReactionType::thinking,"<thinking>", "Pausing to reflect, analyze, or consider something deeply."},
        ReactionalGesture{ReactionType::agreeing, ReactionType::accepting,"<agreeing>", "Nodding or expressing alignment or approval with an idea or statement."},
    };

    inline std::list Directive_List{
        Directive{DirectiveType::go, "<go>","Move forward or away from the current position."},
        Directive{DirectiveType::come, "<come>","Approach or move closer to the speaker."},
        Directive{DirectiveType::stop, "<stop>","Cease movement or activity immediately."},
        Directive{DirectiveType::look, "<look>","Direct your gaze or attention toward something."},
        Directive{DirectiveType::turnAround, "<turnAround>","Rotate your body or direction in the opposite way."},
        Directive{DirectiveType::standBack, "<standBack>","Move away or create distance from a person or object."},
    };

    inline std::string get_emotion_symbol(EmotionType emotion) {
        for (auto& emotional_gesture : Emotional_Gesture_List) {
            if (emotional_gesture.emotion == emotion) {
                return emotional_gesture.symbol;
            }
        }
        return "<invalid emotion>";
    };

    inline std::string get_reaction_symbol(ReactionType reaction) {
        for (auto& reactional_gesture : Reactional_Gesture_List) {
            if (reactional_gesture.reaction == reaction) {
                return reactional_gesture.symbol;
            }
        }
        return "<invalid emotion>";
    };

    inline void dump_embedding(std::vector<float> embedding) {
        for (float val : embedding)
            std::cout << val << " ";
        std::cout << std::endl;
    }


#endif //GESTURE_DEFS_H
