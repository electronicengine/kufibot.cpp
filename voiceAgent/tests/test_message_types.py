"""
Unit tests for message_types.py – verifies enum values match the C++ protocol.

References:
  - src/public_data_messages.h  (MessageType)
  - src/gesture_defs.h          (EmotionType, ReactionType, DirectiveType)
"""

import pytest

from message_types import DirectiveType, EmotionType, MessageType, ReactionType


class TestMessageType:
    """Enum values must match ``enum class MessageType`` in public_data_messages.h."""

    def test_video_frame(self):
        assert MessageType.VideoFrame == 0

    def test_sensor_data(self):
        assert MessageType.SensorData == 1

    def test_control_data(self):
        assert MessageType.ControlData == 2

    def test_database_insert_data(self):
        assert MessageType.DatabaseInsertData == 3

    def test_llm_query(self):
        assert MessageType.LLMQuery == 4

    def test_recognized_speech(self):
        assert MessageType.RecognizedSpeech == 5

    def test_llm_response(self):
        assert MessageType.LLMResponse == 6

    def test_engage_reaction(self):
        assert MessageType.EngageReaction == 7

    def test_recognized_gesture(self):
        assert MessageType.RecognizedGesture == 8

    def test_gesture_performance_completed(self):
        assert MessageType.GesturePerformanceCompleted == 9

    def test_interactive_chat_started(self):
        assert MessageType.InteractiveChatStarted == 10

    def test_sensor_read_request(self):
        assert MessageType.SensorReadRequest == 11

    def test_speak_request(self):
        assert MessageType.SpeakRequest == 12

    def test_update_rag_database_request(self):
        assert MessageType.UpdateRAGDatabaseRequest == 13

    def test_clear_rag_database_request(self):
        assert MessageType.ClearRAGDatabaseRequest == 14

    def test_show_rag_database_request(self):
        assert MessageType.ShowRAGDatabaseRequest == 15

    def test_ai_mode_on_call(self):
        assert MessageType.AIModeOnCall == 16

    def test_ai_mode_off_call(self):
        assert MessageType.AIModeOffCall == 17

    def test_stop_perception_request(self):
        assert MessageType.StopPerceptionRequest == 18

    def test_start_perception_request(self):
        assert MessageType.StartPerceptionRequest == 19

    def test_camera_snap_shot_request(self):
        assert MessageType.CameraSnapShotRequest == 20

    def test_camera_snap_shot_response(self):
        assert MessageType.CameraSnapShotResponse == 21

    def test_range_is_contiguous(self):
        """All values 0..21 must be present – no gaps, no duplicates."""
        values = sorted(m.value for m in MessageType)
        assert values == list(range(22)), f"Gap or duplicate: {values}"

    def test_int_conversion_roundtrip(self):
        """int(m) -> MessageType(m) round-trips correctly."""
        for m in MessageType:
            assert MessageType(int(m)) == m


class TestEmotionType:
    """Enum values must match ``enum class EmotionType`` in gesture_defs.h."""

    def test_happy(self):
        assert EmotionType.happy == 0

    def test_angry(self):
        assert EmotionType.angry == 1

    def test_funny(self):
        assert EmotionType.funny == 2

    def test_serious(self):
        assert EmotionType.serious == 3

    def test_curious(self):
        assert EmotionType.curious == 4

    def test_worried(self):
        assert EmotionType.worried == 5

    def test_surprised(self):
        assert EmotionType.surprised == 6

    def test_confident(self):
        assert EmotionType.confident == 7

    def test_range_is_contiguous(self):
        values = sorted(e.value for e in EmotionType)
        assert values == list(range(8)), f"Gap or duplicate: {values}"


class TestReactionType:
    """Enum values must match ``enum class ReactionType`` in gesture_defs.h."""

    def test_greeting(self):
        assert ReactionType.greeting == 0

    def test_listening(self):
        assert ReactionType.listening == 1

    def test_talking(self):
        assert ReactionType.talking == 2

    def test_accepting(self):
        assert ReactionType.accepting == 3

    def test_rejecting(self):
        assert ReactionType.rejecting == 4

    def test_thinking(self):
        assert ReactionType.thinking == 5

    def test_agreeing(self):
        assert ReactionType.agreeing == 6

    def test_range_is_contiguous(self):
        values = sorted(r.value for r in ReactionType)
        assert values == list(range(7)), f"Gap or duplicate: {values}"


class TestDirectiveType:
    """Enum values must match ``enum class DirectiveType`` in gesture_defs.h."""

    def test_follow_finger(self):
        assert DirectiveType.followFinger == 0

    def test_stop_follow(self):
        assert DirectiveType.stopFollow == 1

    def test_range_is_contiguous(self):
        values = sorted(d.value for d in DirectiveType)
        assert values == list(range(2)), f"Gap or duplicate: {values}"
