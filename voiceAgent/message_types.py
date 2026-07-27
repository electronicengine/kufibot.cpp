"""
Message type enums matching the C++ side's wire protocol.

References:
  - src/public_data_messages.h   -> MessageType enum (0-21)
  - src/gesture_defs.h           -> EmotionType, ReactionType, DirectiveType

Wire format (UDP JSON):
  {"type": <int MessageType>, "payload": "<json string>"}
"""

from enum import IntEnum


class MessageType(IntEnum):
    """Mirrors `enum class MessageType` in public_data_messages.h (values 0-21)."""

    VideoFrame = 0
    SensorData = 1
    ControlData = 2
    DatabaseInsertData = 3
    LLMQuery = 4
    RecognizedSpeech = 5
    LLMResponse = 6
    EngageReaction = 7
    RecognizedGesture = 8
    GesturePerformanceCompleted = 9
    InteractiveChatStarted = 10
    SensorReadRequest = 11
    SpeakRequest = 12
    UpdateRAGDatabaseRequest = 13
    ClearRAGDatabaseRequest = 14
    ShowRAGDatabaseRequest = 15
    AIModeOnCall = 16
    AIModeOffCall = 17
    StopPerceptionRequest = 18
    StartPerceptionRequest = 19
    CameraSnapShotRequest = 20
    CameraSnapShotResponse = 21


class EmotionType(IntEnum):
    """Mirrors `enum class EmotionType` in gesture_defs.h (values 0-7)."""

    happy = 0
    angry = 1
    funny = 2
    serious = 3
    curious = 4
    worried = 5
    surprised = 6
    confident = 7


class ReactionType(IntEnum):
    """Mirrors `enum class ReactionType` in gesture_defs.h (values 0-6)."""

    greeting = 0
    listening = 1
    talking = 2
    accepting = 3
    rejecting = 4
    thinking = 5
    agreeing = 6


class DirectiveType(IntEnum):
    """Mirrors `enum class DirectiveType` in gesture_defs.h (values 0-1)."""

    followFinger = 0
    stopFollow = 1
