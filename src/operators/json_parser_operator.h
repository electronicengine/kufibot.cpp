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


#define CONFIG_PATHS_FILE "/usr/local/etc/config_paths.json"


class JsonParserOperator {

public:
    static JsonParserOperator* get_instance();

    std::optional<std::map<EmotionType, EmotionalMotion>> getEmotionalMotions(){return _emotionalMotions;}
    std::optional<std::map<ReactionType, ReactionalMotion>> getReactionalMotions(){return _reactionalMotions;}
    std::optional<std::map<DirectiveType, DirectiveMotion>> getDirectiveMotions(){return  _directiveMotions;}
    std::optional<std::map<ServoMotorJoint, GestureJointState>> getIdlePosition(){return _idlePosition;}

    std::optional<std::list<EmotionalGesture>> getEmotionalList(){return _emotionalList;}
    std::optional<std::list<ReactionalGesture>> getReactionalList(){return _reactionalList;}
    std::optional<std::list<Directive>> getDirectiveList(){return _directiveList;}

    std::optional<std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>>> getJointAngles(){return _jointAngles; }
    std::optional<ConfigPaths> getConfigPaths(){return _configPaths;}
    std::optional<AiConfig> getAiConfig(){return _aiConfig;}
    std::optional<RagDataset> &getRagDataset(){return _ragDataset;}
    std::optional<std::list<FaceReaction>> &getFaceReactions(){return _faceReactionList;}

private:

    static JsonParserOperator* _instance;

    JsonParserOperator();
    std::optional<RagDataset> _ragDataset;
    std::optional<std::map<EmotionType, EmotionalMotion>> _emotionalMotions;
    std::optional<std::map<ReactionType, ReactionalMotion>> _reactionalMotions;
    std::optional<std::map<DirectiveType, DirectiveMotion>> _directiveMotions;
    std::optional<std::map<ServoMotorJoint, GestureJointState>> _idlePosition;

    std::optional<std::list<EmotionalGesture>> _emotionalList;
    std::optional<std::list<ReactionalGesture>> _reactionalList;
    std::optional<std::list<Directive>> _directiveList;
    std::optional<std::list<FaceReaction>> _faceReactionList;

    std::optional<std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>>> _jointAngles;

    std::optional<ConfigPaths> _configPaths;
    std::optional<AiConfig> _aiConfig;


    void loadConfigPaths(const std::string& filename, ConfigPaths& paths);
    void loadAiConfig(const std::string& filename, AiConfig& config);
    void loadRagDataset(const std::string& filename, RagDataset& dataset);
    void loadFaceReactions(const std::string& filename,std::list<FaceReaction>& faceReactionList);

    void loadGesturesFromFile(
        const std::string& filename,
        std::list<EmotionalGesture>& emotionalList,
        std::list<ReactionalGesture>& reactionalList,
        std::list<Directive>& directiveList);

    void loadJointAnglesFromJson(
        const std::string& filename,
        std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>>& data);

    // Main motion loading/saving methods
    void loadMotionsFromFile(
        const std::string& filename,
        std::map<EmotionType, EmotionalMotion>& emotionalMotions,
        std::map<ReactionType, ReactionalMotion>& reactionalMotions,
        std::map<DirectiveType, DirectiveMotion>& directiveMotions,
        std::map<ServoMotorJoint, GestureJointState>& idlePosition
    );

    std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>> getJointLimits(const std::string& filename);

    // String conversion methods for enum types
    std::string to_string(EmotionType emotion);
    std::string to_string(ReactionType reaction);
    std::string to_string(DirectiveType directive);

    // Motion parsing methods
    static MotionSequenceItem parseMotionSequenceItem(const nlohmann::json& sequenceJson);
    static std::map<ServoMotorJoint, GestureJointState> parseJointPositions(const nlohmann::json& jointsJson);
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
