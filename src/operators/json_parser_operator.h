//
// Created by ybulb on 7/26/2025.
//

#ifndef JSON_PARSER_OPERATOR_H
#define JSON_PARSER_OPERATOR_H

#include <string>
#include <list>
#include <map>
#include "../gesture_defs.h"
#include "../public_data_messages.h"




class JsonParserOperator {



public:
    static JsonParserOperator* get_instance();

    static void loadGesturesFromFile(
        const std::string& filename,
        std::list<EmotionalGesture>& emotionalList,
        std::list<ReactionalGesture>& reactionalList,
        std::list<Directive>& directiveList);

    static void loadGestureJointAnglesFromJson(
        const std::string& filename,
        std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>>& data);

    static void writeGestureJointAnglesToJson(
        const std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>>& data,
        const std::string& filename);
    // Main motion loading/saving methods
    static void loadMotionsFromFile(
        const std::string& filename,
        std::map<EmotionType, EmotionalMotion>& emotionalMotions,
        std::map<ReactionType, ReactionalMotion>& reactionalMotions,
        std::map<DirectiveType, DirectiveMotion>& directiveMotions,
        std::map<ServoMotorJoint, GestureJointState>& idlePosition
    );
private:

    static JsonParserOperator* _instance;

    JsonParserOperator();

    // String conversion methods for enum types
    std::string to_string(EmotionType emotion);
    std::string to_string(ReactionType reaction);
    std::string to_string(DirectiveType directive);

    // Motion parsing methods
    static MotionSequenceItem parseMotionSequenceItem(const nlohmann::json& sequenceJson);
    static  std::map<ServoMotorJoint, GestureJointState> parseJointPositions(const nlohmann::json& jointsJson);
    static EmotionalMotion parseEmotionalMotion(const nlohmann::json& motionJson);
    static ReactionalMotion parseReactionalMotion(const nlohmann::json& motionJson);
    static DirectiveMotion parseDirectiveMotion(const nlohmann::json& motionJson);



    void writeMotionsToFile(
        const std::string& filename,
        const std::map<EmotionType, EmotionalMotion>& emotionalMotions,
        const std::map<ReactionType, ReactionalMotion>& reactionalMotions,
        const std::map<DirectiveType, DirectiveMotion>& directiveMotions,
        const std::map<ServoMotorJoint, GestureJointState>& idlePosition
    );

    static EmotionType emotionTypeFromString(const std::string& str);
    static ReactionType reactionTypeFromString(const std::string& str);
    static DirectiveType directiveTypeFromString(const std::string& str);
    static ServoMotorJoint servoJointFromString(const std::string& str);
    static GestureJointState gestureStateFromString(const std::string& str);

    static std::string to_string(ServoMotorJoint joint);
    static std::string to_string(GestureJointState state);

};




#endif //JSON_PARSER_OPERATOR_H
