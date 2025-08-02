#ifndef  GESTURE_PERFORMER_SERVICE_H
#define  GESTURE_PERFORMER_SERVICE_H


#include <thread>
#include <atomic>

#include <string>
#include "service.h"
#include "robot_controller_service.h"
#include "../operators/speech_performing_operator.h"
#include "../operators/speech_recognizing_operator.h"
#include "interactive_chat_service.h"
#include "web_socket_service.h"


class GesturePerformerService : public Service {

public:

    virtual ~GesturePerformerService();
    static GesturePerformerService *get_instance();


private:
    static GesturePerformerService *_instance;
    std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>> _jointGesturePositionList;

    std::atomic<bool> _gestureWorking{false};
    // Motion data loaded from JSON
    std::map<EmotionType, EmotionalMotion> _emotionalMotions;
    std::map<ReactionType, ReactionalMotion> _reactionalMotions;
    std::map<DirectiveType, DirectiveMotion> _directiveMotions;
    std::map<ServoMotorJoint, GestureJointState> _idlePosition;

    // Joint angle mappings
    std::map<ServoMotorJoint, std::map<GestureJointState, GestureJointAngle>> jointGesturePositionList;

    // Motion control
    std::queue<LLMResponseData> _llmResponseQueue;
    std::mutex _llmResponseQueueMutex;
    GesturePerformerService();

    // Private helper methods
    void control_motion(ServoMotorJoint joint, int angle);
    int getAngleForJointState(ServoMotorJoint joint, GestureJointState state);
    void setIdlePosition();
    void executeJointPositions(const std::map<ServoMotorJoint, GestureJointState>& positions);
    void stopCurrentMotion();

    // Motion execution methods
    void executeEmotionalMotion(EmotionType emotionType);
    void executeReactionalMotion(ReactionType reactionType);
    void executeDirectiveMotion(DirectiveType directiveType);
    void executeMotionSequence(
        const std::map<ServoMotorJoint, GestureJointState>& baseJoints,
        const std::vector<MotionSequenceItem>& sequence,
        int totalDuration
    );


    void service_function();
    void make_mimic(const LLMResponseData& llm_response);
    //subscribed Data Functions
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);


};

#endif //GESTURE_PERFORMER_SERVICE_H