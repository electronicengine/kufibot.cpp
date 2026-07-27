"""
Async UDP bridge between the Python Pipecat voice agent and the C++ VoiceAgentService.

The C++ side listens on a configurable UDP port (default 5005) and understands
JSON packets of the form:

    {"type": <int MessageType>, "payload": "<json string>"}

This module provides an asyncio-based client that sends LLM responses and can
optionally request sensor / camera data from the robot.
"""

from __future__ import annotations

import asyncio
import json
import os
from typing import Any, Dict, Optional

from loguru import logger

try:
    from .message_types import MessageType
except ImportError:
    from message_types import MessageType


# ---------------------------------------------------------------------------
# Default LLMResponse payload factory
# ---------------------------------------------------------------------------

def make_llm_response_payload(
    sentence: str,
    *,
    emotional_gesture: int = 0,  # EmotionType.happy
    reactional_gesture: int = 2,  # ReactionType.talking
    directive: str = "",
    end_marker: bool = True,
    emotion_similarity: float = 0.8,
    reaction_similarity: float = 0.7,
    directive_similarity: float = 0.0,
) -> Dict[str, Any]:
    """Build a wire-format LLMResponse dict matching C++ LLMResponseData::to_json()."""
    return {
        "sentence": sentence,
        "emotional_gesture": emotional_gesture,
        "reactional_gesture": reactional_gesture,
        "directive": directive,
        "end_marker": end_marker,
        "emotion_similarity": emotion_similarity,
        "reaction_similarity": reaction_similarity,
        "directive_similarity": directive_similarity,
    }


# ---------------------------------------------------------------------------
# RobotBridge – async UDP client
# ---------------------------------------------------------------------------

class RobotBridge:
    """Async UDP bridge to the C++ VoiceAgentService.

    Parameters
    ----------
    host : str
        Robot IP / hostname.  Reads ``ROBOT_HOST`` env var, defaults to ``127.0.0.1``.
    port : int
        UDP port.  Reads ``ROBOT_PORT`` env var, defaults to ``5005``.
    recv_timeout : float
        Seconds to wait for a response (used by ``request_*`` helpers).
    """

    def __init__(
        self,
        host: Optional[str] = None,
        port: Optional[int] = None,
        recv_timeout: float = 2.0,
    ) -> None:
        self._host = host or os.environ.get("ROBOT_HOST", "127.0.0.1")
        self._port = port or int(os.environ.get("ROBOT_PORT", "5005"))
        self._recv_timeout = recv_timeout
        self._transport: Optional[asyncio.DatagramTransport] = None

    # ------------------------------------------------------------------
    # Connection management
    # ------------------------------------------------------------------

    async def connect(self) -> None:
        """Create the UDP socket (non-blocking asyncio transport)."""
        if self._transport is not None:
            return
        loop = asyncio.get_running_loop()
        self._transport, _ = await loop.create_datagram_endpoint(
            lambda: _UdpProtocol(),
            remote_addr=(self._host, self._port),
        )
        logger.info(f"[RobotBridge] Connected to {self._host}:{self._port}")

    async def disconnect(self) -> None:
        """Close the UDP socket."""
        if self._transport is not None:
            self._transport.close()
            self._transport = None
            logger.info("[RobotBridge] Disconnected")

    # ------------------------------------------------------------------
    # Core send
    # ------------------------------------------------------------------

    async def send_message(self, msg_type: MessageType, payload: Any) -> None:
        """Encode and send a single JSON message to the C++ side.

        Parameters
        ----------
        msg_type : MessageType
            The message type enum value.
        payload : dict | str
            If a dict, it is JSON-serialised; if a str it is used verbatim.
        """
        if self._transport is None:
            logger.warning("[RobotBridge] send_message called before connect – ignoring")
            return

        payload_str = json.dumps(payload, ensure_ascii=False) if isinstance(payload, dict) else str(payload)

        packet = json.dumps(
            {"type": int(msg_type), "payload": payload_str},
            ensure_ascii=False,
        )
        self._transport.sendto(packet.encode("utf-8"))
        logger.debug(f"[RobotBridge] Sent type={msg_type.name}({msg_type.value}) | payload_len={len(payload_str)}")

    # ------------------------------------------------------------------
    # Request helpers  (send + await reply)
    # ------------------------------------------------------------------

    async def _request_reply(self, msg_type: MessageType, payload: Any = "") -> Optional[Dict[str, Any]]:
        """Send a request message and wait for a single JSON reply."""
        if self._transport is None:
            logger.warning("[RobotBridge] _request_reply called before connect")
            return None

        protocol: _UdpProtocol = self._transport.get_protocol()  # type: ignore[attr-defined]
        protocol._clear_response()

        await self.send_message(msg_type, payload)

        try:
            raw = await asyncio.wait_for(protocol._wait_response(), timeout=self._recv_timeout)
        except asyncio.TimeoutError:
            logger.warning(f"[RobotBridge] Timeout waiting for reply to {msg_type.name}")
            return None

        try:
            return json.loads(raw)  # type: ignore[no-any-return]
        except json.JSONDecodeError:
            logger.warning(f"[RobotBridge] Malformed JSON reply to {msg_type.name}: {raw[:200]}")
            return None

    async def request_sensor_data(self) -> Optional[Dict[str, Any]]:
        """Request current sensor data from the robot.

        Returns the parsed ``SensorData`` JSON dict or ``None`` on failure.
        """
        return await self._request_reply(MessageType.SensorReadRequest)

    async def request_camera_snapshot(self) -> Optional[Dict[str, Any]]:
        """Request a camera snapshot from the robot.

        Returns the parsed ``CameraSnapShotResponse`` JSON dict or ``None`` on failure.
        """
        return await self._request_reply(MessageType.CameraSnapShotRequest)


# ---------------------------------------------------------------------------
# Internal asyncio DatagramProtocol
# ---------------------------------------------------------------------------

class _UdpProtocol(asyncio.DatagramProtocol):
    """Minimal protocol that captures the last received datagram."""

    def __init__(self) -> None:
        self._response: Optional[bytes] = None
        self._event: Optional[asyncio.Event] = None

    def datagram_received(self, data: bytes, addr: tuple) -> None:  # type: ignore[override]
        self._response = data
        if self._event is not None:
            self._event.set()

    def _clear_response(self) -> None:
        self._response = None
        self._event = asyncio.Event()

    async def _wait_response(self) -> bytes:
        assert self._event is not None, "call _clear_response first"
        await self._event.wait()
        assert self._response is not None
        return self._response
