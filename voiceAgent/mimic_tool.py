"""
LLM tool (function call) that lets the model explicitly choose the robot's
mimic (gesture) while composing its answer.

The model calls ``set_mimic`` with either an ``emotion`` or a ``reaction``
(mutually exclusive, matching the C++ ``EmotionType``/``ReactionType``
enums in gesture_defs.h). The selection is stored on a shared
``PendingMimicState`` instance so that ``RobotBridgeProcessor`` can read it
when it builds the ``LLMResponse`` UDP payload for that turn.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

from pipecat.adapters.schemas.function_schema import FunctionSchema
from pipecat.services.llm_service import FunctionCallParams

try:
    from .message_types import EmotionType, ReactionType
except ImportError:
    from message_types import EmotionType, ReactionType


# ---------------------------------------------------------------------------
# Shared mutable state between the tool handler and RobotBridgeProcessor
# ---------------------------------------------------------------------------

@dataclass
class PendingMimicState:
    """Holds the mimic selection made by the LLM for the current turn."""

    emotion: Optional[str] = None
    reaction: Optional[str] = None

    def reset(self) -> None:
        self.emotion = None
        self.reaction = None


# ---------------------------------------------------------------------------
# Tool schema + handler factory
# ---------------------------------------------------------------------------

def _normalize(name: Optional[str]) -> Optional[str]:
    """Return the exact enum member name (case-insensitive lookup), or None."""
    if not isinstance(name, str):
        return None
    return name.strip().lower() or None


def build_set_mimic_tool(state: PendingMimicState) -> FunctionSchema:
    """Build the ``set_mimic`` FunctionSchema bound to ``state``."""

    emotion_names = [e.name for e in EmotionType]
    reaction_names = [r.name for r in ReactionType]

    async def set_mimic_handler(params: FunctionCallParams) -> None:
        emotion = _normalize(params.arguments.get("emotion"))
        reaction = _normalize(params.arguments.get("reaction"))

        valid_emotion = emotion if emotion in EmotionType.__members__ else None
        valid_reaction = reaction if reaction in ReactionType.__members__ else None

        if valid_emotion is not None:
            state.emotion = valid_emotion
            state.reaction = None
        elif valid_reaction is not None:
            state.reaction = valid_reaction
            state.emotion = None

        await params.result_callback(
            {"ok": True, "emotion": state.emotion, "reaction": state.reaction}
        )

    return FunctionSchema(
        name="set_mimic",
        description=(
            "Cevabını söylemeden hemen önce çağır. Robotun bu cevabı verirken "
            "hangi fiziksel mimik/jesti yapacağını seçer. `emotion` veya "
            "`reaction` alanlarından SADECE BİRİNİ doldur; cevabının tonuna "
            "en uygun olanı seç, diğerini boş bırak. Her turda mutlaka bir kez çağır."
        ),
        properties={
            "emotion": {
                "type": "string",
                "enum": emotion_names,
                "description": "Cevap bir duygu ifadesi taşıyorsa doldur.",
            },
            "reaction": {
                "type": "string",
                "enum": reaction_names,
                "description": "Cevap bir tepki/karşılık niteliğindeyse doldur.",
            },
        },
        required=[],
        handler=set_mimic_handler,
    )
