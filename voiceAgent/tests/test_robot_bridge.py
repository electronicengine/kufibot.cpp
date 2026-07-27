"""
Unit tests for robot_bridge.py – async UDP client.

Uses ``unittest.mock.patch`` to avoid real network I/O.
"""

from __future__ import annotations

import asyncio
import json
from unittest.mock import AsyncMock, MagicMock, patch

import pytest

from message_types import MessageType
from robot_bridge import RobotBridge, make_llm_response_payload


# ---------------------------------------------------------------------------
# make_llm_response_payload  (pure-function tests – no async needed)
# ---------------------------------------------------------------------------

class TestMakeLlmResponsePayload:
    def test_defaults(self):
        p = make_llm_response_payload("merhaba")
        assert p["sentence"] == "merhaba"
        assert p["emotional_gesture"] == 0
        assert p["reactional_gesture"] == 2
        assert p["directive"] == ""
        assert p["end_marker"] is True
        assert p["emotion_similarity"] == 0.8
        assert p["reaction_similarity"] == 0.7
        assert p["directive_similarity"] == 0.0

    def test_custom_all_fields(self):
        p = make_llm_response_payload(
            "ileri git",
            emotional_gesture=3,  # serious
            reactional_gesture=0,  # greeting
            directive="followFinger",
            end_marker=False,
            emotion_similarity=0.55,
            reaction_similarity=0.66,
            directive_similarity=0.77,
        )
        assert p["sentence"] == "ileri git"
        assert p["emotional_gesture"] == 3
        assert p["reactional_gesture"] == 0
        assert p["directive"] == "followFinger"
        assert p["end_marker"] is False
        assert p["emotion_similarity"] == 0.55

    def test_serializable_to_json(self):
        p = make_llm_response_payload("test")
        dumped = json.dumps(p, ensure_ascii=False)
        assert "sentence" in dumped
        assert "test" in dumped


# ---------------------------------------------------------------------------
# RobotBridge  (async tests with mocked UDP)
# ---------------------------------------------------------------------------

@pytest.fixture
def fake_transport() -> MagicMock:
    """Return a MagicMock that quacks like an asyncio DatagramTransport."""
    t = MagicMock()
    t.get_protocol.return_value = MagicMock()
    t.get_protocol.return_value._clear_response = MagicMock()
    t.get_protocol.return_value._wait_response = AsyncMock()
    return t


@pytest.fixture
def bridge(fake_transport: MagicMock) -> RobotBridge:
    """Return a RobotBridge whose _transport is already a mock."""
    b = RobotBridge(host="127.0.0.1", port=9999)
    b._transport = fake_transport
    return b


class TestRobotBridgeSendMessage:
    @pytest.mark.asyncio
    async def test_sends_correct_wire_format(self, bridge: RobotBridge, fake_transport: MagicMock):
        await bridge.send_message(MessageType.LLMResponse, {"sentence": "test"})
        fake_transport.sendto.assert_called_once()
        raw = fake_transport.sendto.call_args[0][0]
        packet = json.loads(raw.decode("utf-8"))
        assert packet["type"] == 6
        assert "test" in packet["payload"]

    @pytest.mark.asyncio
    async def test_dict_payload_serialized(self, bridge: RobotBridge, fake_transport: MagicMock):
        await bridge.send_message(MessageType.SensorReadRequest, {"key": 42})
        raw = fake_transport.sendto.call_args[0][0]
        payload = json.loads(raw.decode("utf-8"))["payload"]
        assert json.loads(payload) == {"key": 42}

    @pytest.mark.asyncio
    async def test_string_payload_passthrough(self, bridge: RobotBridge, fake_transport: MagicMock):
        await bridge.send_message(MessageType.LLMResponse, '{"already":"json"}')
        raw = fake_transport.sendto.call_args[0][0]
        payload = json.loads(raw.decode("utf-8"))["payload"]
        assert payload == '{"already":"json"}'

    @pytest.mark.asyncio
    async def test_send_before_connect_noop(self):
        b = RobotBridge(host="127.0.0.1", port=9999)
        # _transport is None -> should warn, not crash
        await b.send_message(MessageType.LLMResponse, {"sentence": "x"})


class TestRobotBridgeRequestReply:
    """Test the _request_reply path with mocked protocol responses."""

    @pytest.mark.asyncio
    async def test_request_sensor_data_parses_reply(self, bridge: RobotBridge, fake_transport: MagicMock):
        proto = fake_transport.get_protocol.return_value
        proto._wait_response.return_value = json.dumps({"compass": {"angle": 180}}).encode()

        result = await bridge.request_sensor_data()
        assert result == {"compass": {"angle": 180}}

    @pytest.mark.asyncio
    async def test_request_sensor_data_timeout(self, bridge: RobotBridge, fake_transport: MagicMock):
        proto = fake_transport.get_protocol.return_value
        proto._wait_response.side_effect = asyncio.TimeoutError()

        result = await bridge.request_sensor_data()
        assert result is None

    @pytest.mark.asyncio
    async def test_request_camera_snapshot_parses_reply(self, bridge: RobotBridge, fake_transport: MagicMock):
        proto = fake_transport.get_protocol.return_value
        proto._wait_response.return_value = json.dumps({"image_path": "/tmp/snap.jpg"}).encode()

        result = await bridge.request_camera_snapshot()
        assert result == {"image_path": "/tmp/snap.jpg"}

    @pytest.mark.asyncio
    async def test_malformed_json_returns_none(self, bridge: RobotBridge, fake_transport: MagicMock):
        proto = fake_transport.get_protocol.return_value
        proto._wait_response.return_value = b"not-json-at-all"

        result = await bridge.request_camera_snapshot()
        assert result is None

    @pytest.mark.asyncio
    async def test_empty_response_returns_none(self, bridge: RobotBridge, fake_transport: MagicMock):
        proto = fake_transport.get_protocol.return_value
        proto._wait_response.return_value = b""

        result = await bridge.request_camera_snapshot()
        assert result is None

    @pytest.mark.asyncio
    async def test_request_before_connect(self):
        b = RobotBridge(host="127.0.0.1", port=9999)
        result = await b.request_sensor_data()
        assert result is None


class TestRobotBridgeDefaults:
    def test_host_default(self):
        b = RobotBridge()
        assert b._host == "127.0.0.1"

    def test_port_default(self):
        b = RobotBridge()
        assert b._port == 5005

    def test_explicit_host_port(self):
        b = RobotBridge(host="10.0.0.5", port=7000)
        assert b._host == "10.0.0.5"
        assert b._port == 7000
