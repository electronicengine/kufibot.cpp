import asyncio
import os
import time

from dotenv import load_dotenv
from loguru import logger

from pipecat.audio.vad.silero import SileroVADAnalyzer
from pipecat.audio.vad.vad_analyzer import VADParams
from pipecat.frames.frames import (
    BotStartedSpeakingFrame,
    BotStoppedSpeakingFrame,
    Frame,
    InputAudioRawFrame,
    TextFrame,
)
from pipecat.pipeline.pipeline import Pipeline
from pipecat.pipeline.runner import PipelineRunner
from pipecat.pipeline.task import PipelineParams, PipelineTask
from pipecat.processors.aggregators.llm_context import LLMContext
from pipecat.processors.aggregators.llm_response_universal import LLMContextAggregatorPair
from pipecat.processors.frame_processor import FrameDirection, FrameProcessor
from pipecat.services.deepgram.stt import DeepgramSTTService
from pipecat.services.elevenlabs.tts import ElevenLabsTTSService
from pipecat.services.openai.llm import OpenAILLMService
from pipecat.transcriptions.language import Language
from pipecat.transports.local.audio import LocalAudioTransport, LocalAudioTransportParams

try:
    from .robot_bridge import RobotBridge, make_llm_response_payload
    from .message_types import MessageType
except ImportError:
    from robot_bridge import RobotBridge, make_llm_response_payload
    from message_types import MessageType

load_dotenv()


# ---------------------------------------------------------------------------
# 1) LLM çıktısını alıp C++ VoiceAgentService'e UDP ile gönderen processor
# ---------------------------------------------------------------------------
class RobotBridgeProcessor(FrameProcessor):
    """Her LLM TextFrame çıktısını LLMResponse olarak UDP üzerinden C++ tarafına iletir.

    Frame TTS'e de iletilmeye devam eder (robot konuşur).
    Bridge gönderimi başarısız olsa bile pipeline çalışmaya devam eder.
    """

    def __init__(self, bridge: RobotBridge):
        super().__init__()
        self._bridge = bridge

    async def process_frame(self, frame: Frame, direction: FrameDirection):
        await super().process_frame(frame, direction)

        if isinstance(frame, TextFrame):
            payload = make_llm_response_payload(sentence=frame.text)
            try:
                await self._bridge.send_message(MessageType.LLMResponse, payload)
                logger.debug(f"[RobotBridge] LLMResponse gönderildi: {frame.text[:80]}...")
            except Exception as exc:
                logger.error(f"[RobotBridge] LLMResponse gönderilemedi: {exc}")

        # Frame'i her durumda ilet (TTS duyulmaya devam eder)
        await self.push_frame(frame, direction)


# ---------------------------------------------------------------------------
# 2) Echo suppression – bot konuşurken mikrofonu sustur
# ---------------------------------------------------------------------------
class EchoSuppressionProcessor(FrameProcessor):
    def __init__(self):
        super().__init__()
        self._bot_speaking = False
        self._ignore_audio_until = 0.0

    async def process_frame(self, frame: Frame, direction: FrameDirection):
        await super().process_frame(frame, direction)

        if isinstance(frame, BotStartedSpeakingFrame):
            self._bot_speaking = True
        elif isinstance(frame, BotStoppedSpeakingFrame):
            self._bot_speaking = False
            self._ignore_audio_until = time.monotonic() + 0.35
        elif isinstance(frame, InputAudioRawFrame):
            if self._bot_speaking or time.monotonic() < self._ignore_audio_until:
                return

        await self.push_frame(frame, direction)


# ---------------------------------------------------------------------------
# 3) Ana pipeline
# ---------------------------------------------------------------------------
async def main():
    # --- Robot UDP bridge ---
    bridge = RobotBridge()
    await bridge.connect()

    transport = LocalAudioTransport(
        LocalAudioTransportParams(
            audio_in_enabled=True,
            audio_out_enabled=True,
            vad_analyzer=SileroVADAnalyzer(
                params=VADParams(
                    confidence=0.4,
                    min_volume=0.05,
                )
            ),
        )
    )

    stt = DeepgramSTTService(
        api_key=os.environ["DEEPGRAM_API_KEY"],
        settings=DeepgramSTTService.Settings(
            language="tr",
            endpointing=1000,
            utterance_end_ms=1000,
        ),
    )

    llm = OpenAILLMService(
        api_key=os.environ["OPENAI_API_KEY"],
        model="gpt-4o-mini",
    )

    tts = ElevenLabsTTSService(
        api_key=os.environ["ELEVENLABS_API_KEY"],
        settings=ElevenLabsTTSService.Settings(
            voice="NoiYxL9g25M4orC8Q0ls",
            model="eleven_flash_v2_5",
            language=Language.TR,
        ),
    )

    messages = [
        {
            "role": "system",
            "content": (
                "Sen bir sosyal robot asistanısın. Adın Kufi. "
                "Kısa, samimi ve Türkçe cevap ver (en fazla 2-3 cümle). "
                "Kullanıcı senden hareket etmeni isterse (ör. 'ileri git', 'dur', "
                "'sağa dön', 'sola dön') bunu onaylayan kısa bir cümle kur ve "
                "komutu cümlenin içinde tekrar et. "
                "Duygusal ve cana yakın ol ama lafı uzatma."
            ),
        }
    ]
    context = LLMContext(messages)
    context_aggregator = LLMContextAggregatorPair(context)

    robot_bridge_processor = RobotBridgeProcessor(bridge)
    echo_suppression = EchoSuppressionProcessor()

    pipeline = Pipeline(
        [
            transport.input(),               # mikrofon
            echo_suppression,                # hoparlör yankısını STT/VAD'a gönderme
            stt,                             # konuşma -> metin
            context_aggregator.user(),
            llm,                             # metin -> LLM cevabı
            robot_bridge_processor,          # cevabı C++ tarafına UDP ile ilet
            tts,                             # metin -> ses
            transport.output(),              # hoparlör
            context_aggregator.assistant(),
        ]
    )

    task = PipelineTask(pipeline, params=PipelineParams(allow_interruptions=True))

    try:
        runner = PipelineRunner()
        await runner.run(task)
    finally:
        await bridge.disconnect()


if __name__ == "__main__":
    asyncio.run(main())
