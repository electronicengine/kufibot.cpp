"""
Unit tests for the RobotBridgeProcessor in voiceAgent.py.

Verifies that LLM TextFrames are correctly forwarded to the C++ side via
RobotBridge UDP, while still being passed through for TTS.
"""

from __future__ import annotations

from unittest.mock import AsyncMock, MagicMock

import pytest
from pipecat.frames.frames import (
    BotStartedSpeakingFrame,
    BotStoppedSpeakingFrame,
    TextFrame,
)
from pipecat.processors.frame_processor import FrameDirection

from message_types import MessageType

# Import after ensuring the voiceAgent module is importable (run from voiceAgent/)
from voiceAgent import RobotBridgeProcessor


@pytest.fixture
def mock_bridge() -> AsyncMock:
    """Return an AsyncMock standing in for RobotBridge."""
    bridge = AsyncMock()
    bridge.send_message = AsyncMock()
    return bridge


@pytest.fixture
def processor(mock_bridge: AsyncMock) -> RobotBridgeProcessor:
    return RobotBridgeProcessor(bridge=mock_bridge)


class TestRobotBridgeProcessor:
    @pytest.mark.asyncio
    async def test_text_frame_sends_llm_response(
        self, processor: RobotBridgeProcessor, mock_bridge: AsyncMock
    ):
        frame = TextFrame("Merhaba dünya!")
        processor.push_frame = AsyncMock()

        await processor.process_frame(frame, FrameDirection.DOWNSTREAM)

        mock_bridge.send_message.assert_called_once()
        call_args = mock_bridge.send_message.call_args
        assert call_args[0][0] == MessageType.LLMResponse
        payload = call_args[0][1]
        assert payload["sentence"] == "Merhaba dünya!"
        assert "emotional_gesture" in payload
        assert "reactional_gesture" in payload
        assert "directive" in payload
        assert "end_marker" in payload

    @pytest.mark.asyncio
    async def test_text_frame_passes_through_for_tts(
        self, processor: RobotBridgeProcessor, mock_bridge: AsyncMock
    ):
        frame = TextFrame("Nasılsın?")
        processor.push_frame = AsyncMock()

        await processor.process_frame(frame, FrameDirection.DOWNSTREAM)

        processor.push_frame.assert_called_once_with(frame, FrameDirection.DOWNSTREAM)

    @pytest.mark.asyncio
    async def test_non_text_frame_not_sent_to_bridge(
        self, processor: RobotBridgeProcessor, mock_bridge: AsyncMock
    ):
        frame = BotStartedSpeakingFrame()
        processor.push_frame = AsyncMock()

        await processor.process_frame(frame, FrameDirection.DOWNSTREAM)

        mock_bridge.send_message.assert_not_called()
        processor.push_frame.assert_called_once_with(frame, FrameDirection.DOWNSTREAM)

    @pytest.mark.asyncio
    async def test_bot_stopped_speaking_passthrough(
        self, processor: RobotBridgeProcessor, mock_bridge: AsyncMock
    ):
        frame = BotStoppedSpeakingFrame()
        processor.push_frame = AsyncMock()

        await processor.process_frame(frame, FrameDirection.DOWNSTREAM)

        mock_bridge.send_message.assert_not_called()
        processor.push_frame.assert_called_once_with(frame, FrameDirection.DOWNSTREAM)

    @pytest.mark.asyncio
    async def test_bridge_send_failure_does_not_crash(
        self, processor: RobotBridgeProcessor, mock_bridge: AsyncMock
    ):
        """Graceful degradation: if UDP send fails, the frame still goes to TTS."""
        mock_bridge.send_message.side_effect = OSError("Network down")
        frame = TextFrame("Bu mesaj TTS'e ulaşmalı")
        processor.push_frame = AsyncMock()

        await processor.process_frame(frame, FrameDirection.DOWNSTREAM)

        processor.push_frame.assert_called_once_with(frame, FrameDirection.DOWNSTREAM)

    @pytest.mark.asyncio
    async def test_empty_text_frame_still_forwarded(
        self, processor: RobotBridgeProcessor, mock_bridge: AsyncMock
    ):
        frame = TextFrame("")
        processor.push_frame = AsyncMock()

        await processor.process_frame(frame, FrameDirection.DOWNSTREAM)

        mock_bridge.send_message.assert_called_once()
        processor.push_frame.assert_called_once_with(frame, FrameDirection.DOWNSTREAM)
