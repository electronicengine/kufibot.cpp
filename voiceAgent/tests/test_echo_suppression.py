"""
Unit tests for EchoSuppressionProcessor in voiceAgent.py.

Covers:
  - Basic echo suppression (BotStarted/BotStopped)
  - Interruption handling (InterruptionFrame, UserStartedSpeakingFrame)
  - Safety valve (auto-reset after 10s without BotStoppedSpeakingFrame)
  - Edge cases (rapid toggling, multiple interruptions, non-audio frames)
  - Real-world regression scenarios
"""

from __future__ import annotations

import time
from unittest.mock import AsyncMock, patch

import pytest
from pipecat.frames.frames import (
    BotStartedSpeakingFrame,
    BotStoppedSpeakingFrame,
    InputAudioRawFrame,
    InterruptionFrame,
    TextFrame,
    UserStartedSpeakingFrame,
)
from pipecat.processors.frame_processor import FrameDirection

from voiceAgent import EchoSuppressionProcessor

# ── helpers ────────────────────────────────────────────────────────────────

GRACE_PERIOD = 0.35
SAFETY_TIMEOUT = 30.0


@pytest.fixture
def proc() -> EchoSuppressionProcessor:
    return EchoSuppressionProcessor()


def audio() -> InputAudioRawFrame:
    return InputAudioRawFrame(audio=b"\x00\x00" * 160, sample_rate=16000, num_channels=1)


def pushed_types(proc: EchoSuppressionProcessor):
    """Return list of type names for all frames pushed downstream."""
    return [type(call.args[0]).__name__ for call in proc.push_frame.call_args_list]


# ═══════════════════════════════════════════════════════════════════════════
# 1) Basic echo suppression
# ═══════════════════════════════════════════════════════════════════════════

class TestBasicEchoSuppression:

    @pytest.mark.asyncio
    async def test_audio_passes_when_bot_not_speaking(self, proc):
        proc.push_frame = AsyncMock()
        a = audio()
        await proc.process_frame(a, FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_audio_suppressed_when_bot_speaking(self, proc):
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
        await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        # only BotStartedSpeakingFrame pushed, not the audio
        assert pushed_types(proc) == ["BotStartedSpeakingFrame"]

    @pytest.mark.asyncio
    async def test_audio_passes_after_bot_stops_and_grace(self, proc):
        """Bot durduktan sonra grace period bitince audio gecmeli."""
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
        await proc.process_frame(BotStoppedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        # grace period icinde -> bastirilir
        await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" not in pushed_types(proc)

        # grace period bittikten sonra -> gecer
        after_grace = time.monotonic() + GRACE_PERIOD + 0.1
        with patch("time.monotonic", return_value=after_grace):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)

        assert "InputAudioRawFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_grace_period_after_bot_stops(self, proc):
        """BotStopped sonrasi grace period boyunca audio bastirilmali."""
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
        await proc.process_frame(BotStoppedSpeakingFrame(), FrameDirection.DOWNSTREAM)
        await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" not in pushed_types(proc)


# ═══════════════════════════════════════════════════════════════════════════
# 2) Interruption handling
# ═══════════════════════════════════════════════════════════════════════════

class TestInterruptionHandling:

    @pytest.mark.asyncio
    async def test_interruption_resets_and_audio_passes_after_grace(self, proc):
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
        await proc.process_frame(InterruptionFrame(), FrameDirection.DOWNSTREAM)

        # grace period icinde -> bastirilir
        await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" not in pushed_types(proc)

        # grace period sonrasi -> gecer
        after_grace = time.monotonic() + GRACE_PERIOD + 0.1
        with patch("time.monotonic", return_value=after_grace):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_user_started_speaking_resets_bot_speaking(self, proc):
        """UserStartedSpeakingFrame: _bot_speaking sifirlanir, grace period YOK."""
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
        await proc.process_frame(UserStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        # hemen gecer (grace period yok)
        await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_interruption_when_bot_not_speaking(self, proc):
        """Bot konusmuyorken InterruptionFrame -> grace period yine uygulanir."""
        proc.push_frame = AsyncMock()
        await proc.process_frame(InterruptionFrame(), FrameDirection.DOWNSTREAM)

        # grace period icinde -> bastirilir
        await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" not in pushed_types(proc)

        # grace period sonrasi -> gecer
        after_grace = time.monotonic() + GRACE_PERIOD + 0.1
        with patch("time.monotonic", return_value=after_grace):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_user_speaking_when_bot_not_speaking(self, proc):
        """Bot konusmuyorken UserStartedSpeakingFrame -> NOOP, audio hemen gecer."""
        proc.push_frame = AsyncMock()
        await proc.process_frame(UserStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
        await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_multiple_interruptions(self, proc):
        """Pes pese interruptionlar sorun cikarmamali."""
        proc.push_frame = AsyncMock()
        for _ in range(3):
            await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
            await proc.process_frame(InterruptionFrame(), FrameDirection.DOWNSTREAM)

        after_grace = time.monotonic() + GRACE_PERIOD + 0.1
        with patch("time.monotonic", return_value=after_grace):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" in pushed_types(proc)


# ═══════════════════════════════════════════════════════════════════════════
# 3) Safety valve
# ═══════════════════════════════════════════════════════════════════════════

class TestSafetyValve:

    @pytest.mark.asyncio
    async def test_triggers_after_timeout(self, proc):
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        after = time.monotonic() + SAFETY_TIMEOUT + 1.0
        with patch("time.monotonic", return_value=after):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_not_triggered_before_timeout(self, proc):
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        before = time.monotonic() + SAFETY_TIMEOUT - 1.0
        with patch("time.monotonic", return_value=before):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" not in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_resets_on_bot_stopped(self, proc):
        """Bot normal durursa sayac sifirlanir, grace sonrasi audio gecer."""
        proc.push_frame = AsyncMock()
        base = time.monotonic()

        # Bot baslangic (t=0)
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        # Bot durdu (t=5)
        with patch("time.monotonic", return_value=base + 5.0):
            await proc.process_frame(BotStoppedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        # grace period'dan sonra (t=5.5)
        with patch("time.monotonic", return_value=base + 5.5):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)

        assert "InputAudioRawFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_exactly_at_threshold_no_trigger(self, proc):
        """Tam 10.0 saniyede safety valve tetiklenmez (float edge-case korumali)."""
        proc.push_frame = AsyncMock()
        base = time.monotonic()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        # tam 10 sn (float sapmasi olmasin diye ayni base kullan)
        with patch("time.monotonic", return_value=base + SAFETY_TIMEOUT):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" not in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_just_above_threshold_triggers(self, proc):
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        just_above = time.monotonic() + SAFETY_TIMEOUT + 0.001
        with patch("time.monotonic", return_value=just_above):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_triggers_only_once_then_audio_free(self, proc):
        """Emniyet supabi tetiklendikten sonra surekli audio gecmeli."""
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        t1 = time.monotonic() + SAFETY_TIMEOUT + 1.0
        with patch("time.monotonic", return_value=t1):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)

        t2 = time.monotonic() + SAFETY_TIMEOUT + 2.0
        with patch("time.monotonic", return_value=t2):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)

        assert pushed_types(proc).count("InputAudioRawFrame") == 2

    @pytest.mark.asyncio
    async def test_timer_reset_on_new_bot_started(self, proc):
        """Ikinci BotStarted zamanlayiciyi sifirlar."""
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        # SAFETY_TIMEOUT - 1 sn sonra yeni BotStarted (sayac sifirlanir)
        t1 = time.monotonic() + (SAFETY_TIMEOUT - 1.0)
        with patch("time.monotonic", return_value=t1):
            await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        # ikinci BotStarted'tan sadece 5 sn sonra
        t2 = time.monotonic() + (SAFETY_TIMEOUT - 1.0) + 5.0
        with patch("time.monotonic", return_value=t2):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)

        # 5 sn < SAFETY_TIMEOUT -> tetiklenmemeli
        assert "InputAudioRawFrame" not in pushed_types(proc)


# ═══════════════════════════════════════════════════════════════════════════
# 4) Edge cases
# ═══════════════════════════════════════════════════════════════════════════

class TestEdgeCases:

    @pytest.mark.asyncio
    async def test_non_audio_frames_always_pass(self, proc):
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
        text = TextFrame("merhaba")
        await proc.process_frame(text, FrameDirection.DOWNSTREAM)
        assert "TextFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_all_control_frames_pass_through(self, proc):
        """BotStarted, BotStopped, Interruption, UserStartedSpeaking hep downstream'e iletilir."""
        proc.push_frame = AsyncMock()
        frames = [
            BotStartedSpeakingFrame(),
            BotStoppedSpeakingFrame(),
            InterruptionFrame(),
            UserStartedSpeakingFrame(),
        ]
        for f in frames:
            await proc.process_frame(f, FrameDirection.DOWNSTREAM)

        # Her frame en az 1 kez push edilmis olmali (super().process_frame ekstra push yapabilir)
        type_names = pushed_types(proc)
        for expected in ["BotStartedSpeakingFrame", "BotStoppedSpeakingFrame",
                         "InterruptionFrame", "UserStartedSpeakingFrame"]:
            assert expected in type_names, f"{expected} downstream'e iletilmedi!"

    @pytest.mark.asyncio
    async def test_rapid_toggle(self, proc):
        """Hizli ac/kapa dongusu."""
        proc.push_frame = AsyncMock()
        for _ in range(3):
            await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
            await proc.process_frame(BotStoppedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        after_grace = time.monotonic() + GRACE_PERIOD + 0.1
        with patch("time.monotonic", return_value=after_grace):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_bot_stopped_without_bot_started(self, proc):
        """BotStarted olmadan BotStopped (grace period olur ama sorun cikarmaz)."""
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStoppedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        after_grace = time.monotonic() + GRACE_PERIOD + 0.1
        with patch("time.monotonic", return_value=after_grace):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_initial_state(self, proc):
        assert proc._bot_speaking is False
        assert proc._bot_speaking_since == 0.0
        assert proc._ignore_audio_until == 0.0
        assert proc.MAX_BOT_SPEAKING_SECS > 0

    @pytest.mark.asyncio
    async def test_safety_valve_followed_by_normal_cycle(self, proc):
        """Emniyet supabi sonrasi normal dongu calisir."""
        proc.push_frame = AsyncMock()

        # safety valve trigger
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
        with patch("time.monotonic", return_value=time.monotonic() + SAFETY_TIMEOUT + 1.0):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)

        # normal dongu
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
        await proc.process_frame(BotStoppedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        after_grace = time.monotonic() + GRACE_PERIOD + 0.1
        with patch("time.monotonic", return_value=after_grace):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" in pushed_types(proc)


# ═══════════════════════════════════════════════════════════════════════════
# 5) Gercek senaryo simulasyonlari (regresyon)
# ═══════════════════════════════════════════════════════════════════════════

class TestRealWorldScenarios:

    @pytest.mark.asyncio
    async def test_llm_hangs_then_user_interrupts(self, proc):
        """
        KRITIK: LLM takilir -> BotStarted gelir, BotStopped gelmez.
        41 sn sonra kullanici konusur -> InterruptionFrame.
        Audio gecmeli! (eski bug: sagir kaliyordu)
        """
        proc.push_frame = AsyncMock()
        base = time.monotonic()

        # Bot konusmaya baslasin (LLM ciktisi) t=0
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        # 41 sn sonra kullanici interruption yapsin
        with patch("time.monotonic", return_value=base + 41.0):
            await proc.process_frame(InterruptionFrame(), FrameDirection.DOWNSTREAM)
            await proc.process_frame(UserStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        # grace period sonrasi audio gecmeli (t=41.5)
        with patch("time.monotonic", return_value=base + 41.5):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)

        assert "InputAudioRawFrame" in pushed_types(proc), (
            "REGRESYON: Interruption sonrasi audio gecmedi! Bot sagir kaldi."
        )

    @pytest.mark.asyncio
    async def test_user_speaks_while_bot_speaking(self, proc):
        """Bot konusurken UserStartedSpeakingFrame -> audio hemen gecer."""
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
        await proc.process_frame(UserStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)
        assert "InputAudioRawFrame" in pushed_types(proc), (
            "REGRESYON: Kullanici konusmaya basladi ama audio engellendi!"
        )

    @pytest.mark.asyncio
    async def test_multi_segment_tts_does_not_trigger_safety_valve_early(self, proc):
        """
        KRITIK REGRESYON (10:39 log analizi): 2-3 cumlelik yanit birden fazla
        TTS parcasi halinde sentezlenip ~15-20 sn surebilir. Emniyet supabi
        bu sure dolmadan tetiklenip mikrofonu ACMAMALI; aksi halde STT kendi
        sesini (speaker echo) yanlislikla kullanici konusmasi sanip
        transkribe eder ve konusma baglami bozulur.
        """
        proc.push_frame = AsyncMock()
        base = time.monotonic()

        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        # 2. TTS parcasi baslar (~1 sn sonra), toplam konusma ~18 sn surer
        # 18 sn < SAFETY_TIMEOUT (30) oldugu icin mikrofon HALA KAPALI olmali
        with patch("time.monotonic", return_value=base + 18.0):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)

        assert "InputAudioRawFrame" not in pushed_types(proc), (
            "REGRESYON: Emniyet supabi normal (18sn) TTS oynatimi sirasinda "
            "erken tetiklendi! Bu, STT'nin bot'un kendi sesini (echo) "
            "transkribe etmesine yol acar."
        )

        # Ses gercekten bitince (BotStopped) mikrofon acilmali
        with patch("time.monotonic", return_value=base + 18.2):
            await proc.process_frame(BotStoppedSpeakingFrame(), FrameDirection.DOWNSTREAM)
        with patch("time.monotonic", return_value=base + 18.2 + GRACE_PERIOD + 0.1):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)

        assert "InputAudioRawFrame" in pushed_types(proc)

    @pytest.mark.asyncio
    async def test_long_silence_safety_valve(self, proc):
        """BotStarted sonrasi uzun sessizlik -> safety valve."""
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        after = time.monotonic() + SAFETY_TIMEOUT + 5.0
        with patch("time.monotonic", return_value=after):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)

        assert "InputAudioRawFrame" in pushed_types(proc), (
            f"REGRESYON: {SAFETY_TIMEOUT + 5.0:.0f} sn sonra safety valve devreye girmedi!"
        )

    @pytest.mark.asyncio
    async def test_consecutive_interruptions(self, proc):
        """3 kez pes pese interruption -> her seferinde audio gecmeli."""
        proc.push_frame = AsyncMock()
        for _ in range(3):
            await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)
            await proc.process_frame(InterruptionFrame(), FrameDirection.DOWNSTREAM)

            after_grace = time.monotonic() + GRACE_PERIOD + 0.1
            with patch("time.monotonic", return_value=after_grace):
                await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)

        assert pushed_types(proc).count("InputAudioRawFrame") == 3

    @pytest.mark.asyncio
    async def test_llm_timeout_no_interruption(self, proc):
        """
        LLM timeout olur, BotStopped GELMEZ, kullanici da interruption yapmaz.
        Safety valve SAFETY_TIMEOUT sonra devreye girmeli.
        """
        proc.push_frame = AsyncMock()
        await proc.process_frame(BotStartedSpeakingFrame(), FrameDirection.DOWNSTREAM)

        after = time.monotonic() + SAFETY_TIMEOUT + 0.5
        with patch("time.monotonic", return_value=after):
            await proc.process_frame(audio(), FrameDirection.DOWNSTREAM)

        assert "InputAudioRawFrame" in pushed_types(proc), (
            "REGRESYON: LLM timeout sonrasi safety valve devreye girmedi!"
        )
