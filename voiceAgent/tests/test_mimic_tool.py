"""
Unit tests for mimic_tool.py – the set_mimic LLM tool.
"""

from __future__ import annotations

from types import SimpleNamespace
from unittest.mock import AsyncMock

import pytest

from message_types import EmotionType, ReactionType
from mimic_tool import PendingMimicState, build_set_mimic_tool


class TestPendingMimicState:
    def test_defaults(self):
        state = PendingMimicState()
        assert state.emotion is None
        assert state.reaction is None

    def test_reset_clears_both(self):
        state = PendingMimicState(emotion="happy", reaction="greeting")
        state.reset()
        assert state.emotion is None
        assert state.reaction is None


class TestBuildSetMimicToolSchema:
    def test_schema_name(self):
        tool = build_set_mimic_tool(PendingMimicState())
        assert tool.name == "set_mimic"

    def test_emotion_enum_matches_emotion_type(self):
        tool = build_set_mimic_tool(PendingMimicState())
        assert tool.properties["emotion"]["enum"] == [e.name for e in EmotionType]

    def test_reaction_enum_matches_reaction_type(self):
        tool = build_set_mimic_tool(PendingMimicState())
        assert tool.properties["reaction"]["enum"] == [r.name for r in ReactionType]

    def test_no_required_fields(self):
        tool = build_set_mimic_tool(PendingMimicState())
        assert tool.required == []


def make_params(**arguments) -> SimpleNamespace:
    return SimpleNamespace(arguments=arguments, result_callback=AsyncMock())


class TestSetMimicHandler:
    @pytest.mark.asyncio
    async def test_valid_emotion_sets_state_and_clears_reaction(self):
        state = PendingMimicState(reaction="greeting")
        tool = build_set_mimic_tool(state)
        params = make_params(emotion="happy")

        await tool.handler(params)

        assert state.emotion == "happy"
        assert state.reaction is None
        params.result_callback.assert_awaited_once()

    @pytest.mark.asyncio
    async def test_valid_reaction_sets_state_and_clears_emotion(self):
        state = PendingMimicState(emotion="angry")
        tool = build_set_mimic_tool(state)
        params = make_params(reaction="listening")

        await tool.handler(params)

        assert state.reaction == "listening"
        assert state.emotion is None
        params.result_callback.assert_awaited_once()

    @pytest.mark.asyncio
    async def test_case_insensitive_matching(self):
        state = PendingMimicState()
        tool = build_set_mimic_tool(state)
        params = make_params(emotion="Surprised")

        await tool.handler(params)

        assert state.emotion == "surprised"

    @pytest.mark.asyncio
    async def test_invalid_values_ignored(self):
        state = PendingMimicState()
        tool = build_set_mimic_tool(state)
        params = make_params(emotion="not-a-real-emotion", reaction="not-a-real-reaction")

        await tool.handler(params)

        assert state.emotion is None
        assert state.reaction is None
        params.result_callback.assert_awaited_once()

    @pytest.mark.asyncio
    async def test_no_arguments_leaves_state_untouched(self):
        state = PendingMimicState()
        tool = build_set_mimic_tool(state)
        params = make_params()

        await tool.handler(params)

        assert state.emotion is None
        assert state.reaction is None
        params.result_callback.assert_awaited_once()

    @pytest.mark.asyncio
    async def test_emotion_takes_priority_over_reaction_when_both_valid(self):
        state = PendingMimicState()
        tool = build_set_mimic_tool(state)
        params = make_params(emotion="funny", reaction="agreeing")

        await tool.handler(params)

        assert state.emotion == "funny"
        assert state.reaction is None
