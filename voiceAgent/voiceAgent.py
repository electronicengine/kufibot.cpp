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
    InterruptionFrame,
    TextFrame,
    UserStartedSpeakingFrame,
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
    from .message_types import MessageType, EmotionType, ReactionType
    from .mimic_tool import PendingMimicState, build_set_mimic_tool
except ImportError:
    from robot_bridge import RobotBridge, make_llm_response_payload
    from message_types import MessageType, EmotionType, ReactionType
    from mimic_tool import PendingMimicState, build_set_mimic_tool

load_dotenv()


# ---------------------------------------------------------------------------
# 1) LLM çıktısını alıp C++ VoiceAgentService'e UDP ile gönderen processor
# ---------------------------------------------------------------------------
class RobotBridgeProcessor(FrameProcessor):
    """Her LLM cevabı tamamlandığında birikmiş metni tek bir LLMResponse
    olarak UDP üzerinden C++ tarafına iletir.

    LLM akış halinde birden fazla TextFrame gönderebilir; bu processor
    TextFrame'leri biriktirir ve 300 ms sessizlikten sonra tam metni
    gönderir.  Yeni bir TextFrame gelirse sayaç sıfırlanır, böylece her
    LLM turu için yalnızca bir kez UDP mesajı gönderilmiş olur.
    """

    def __init__(self, bridge: RobotBridge, mimic_state: PendingMimicState):
        super().__init__()
        self._bridge = bridge
        self._mimic_state = mimic_state
        self._buffer: list[str] = []
        self._flush_task: asyncio.Task | None = None

    async def _flush_buffer(self):
        await asyncio.sleep(0.3)  # 300 ms daha TextFrame gelmezse gönder
        if not self._buffer:
            return
        full_text = " ".join(self._buffer)
        self._buffer.clear()

        emotion = self._mimic_state.emotion
        reaction = self._mimic_state.reaction
        self._mimic_state.reset()

        if emotion is not None:
            payload = make_llm_response_payload(
                sentence=full_text,
                emotional_gesture=EmotionType[emotion].value,
                emotion_similarity=1.0,
                reactional_gesture=ReactionType.talking.value,
                reaction_similarity=0.0,
            )
        elif reaction is not None:
            payload = make_llm_response_payload(
                sentence=full_text,
                reactional_gesture=ReactionType[reaction].value,
                reaction_similarity=1.0,
                emotional_gesture=EmotionType.happy.value,
                emotion_similarity=0.0,
            )
        else:
            # LLM set_mimic tool'unu çağırmadıysa varsayılan davranışa düş
            payload = make_llm_response_payload(sentence=full_text)
        try:
            await self._bridge.send_message(MessageType.LLMResponse, payload)
            logger.debug(f"[RobotBridge] LLMResponse gönderildi: {full_text[:80]}...")
        except Exception as exc:
            logger.error(f"[RobotBridge] LLMResponse gönderilemedi: {exc}")

    async def process_frame(self, frame: Frame, direction: FrameDirection):
        await super().process_frame(frame, direction)

        if isinstance(frame, TextFrame):
            self._buffer.append(frame.text)
            # Önceki flush görevini iptal edip yenisini başlat (debounce)
            if self._flush_task:
                self._flush_task.cancel()
            self._flush_task = asyncio.create_task(self._flush_buffer())

        # Frame'i her durumda ilet (TTS duyulmaya devam eder)
        await self.push_frame(frame, direction)


# ---------------------------------------------------------------------------
# 2) Echo suppression – bot konuşurken mikrofonu sustur
# ---------------------------------------------------------------------------
class EchoSuppressionProcessor(FrameProcessor):
    """Bot konuşurken mikrofonu susturur.

    Interruption durumunda da _bot_speaking durumunu sıfırlar.
    Ayrıca MAX_BOT_SPEAKING_SECS'ten uzun süre BotStartedSpeakingFrame
    geldikten sonra BotStoppedSpeakingFrame gelmezse, zorla sıfırlama
    yapar (emniyet supabı).

    ÖNEMLİ: Bu eşik, gerçek TTS çalma süresinin (2-3 cümlelik yanıtlar
    birden fazla TTS parçası halinde sentezlenebilir ve 10-20 sn sürebilir)
    ÜZERİNDE olmalı. Aksi halde emniyet supabı hoparlörden ses hâlâ
    çıkarken mikrofonu açar; STT kendi sesini (echo) yanlışlıkla
    kullanıcı konuşması olarak transkribe eder ve konuşma bağlamı bozulur.
    Bu yalnızca gerçek donma senaryoları (LLM/TTS'in hiç yanıt üretmediği
    durumlar) için son çare olmalı, normal akışın bir parçası olmamalı.
    """

    MAX_BOT_SPEAKING_SECS = 30.0  # emniyet timeout'u (gerçek donma için son çare)

    def __init__(self):
        super().__init__()
        self._bot_speaking = False
        self._bot_speaking_since = 0.0
        self._ignore_audio_until = 0.0

    async def process_frame(self, frame: Frame, direction: FrameDirection):
        await super().process_frame(frame, direction)

        # --- interruption: bot konuşmasını iptal et ---
        if isinstance(frame, InterruptionFrame):
            if self._bot_speaking:
                logger.debug("[EchoSupp] Interruption geldi, bot_speaking sıfırlanıyor")
            self._bot_speaking = False
            self._bot_speaking_since = 0.0
            self._ignore_audio_until = time.monotonic() + 0.35

        # --- kullanıcı konuşmaya başladı → bot susmalı ---
        elif isinstance(frame, UserStartedSpeakingFrame):
            if self._bot_speaking:
                logger.debug("[EchoSupp] Kullanıcı konuşmaya başladı, bot_speaking sıfırlanıyor")
            self._bot_speaking = False
            self._bot_speaking_since = 0.0

        elif isinstance(frame, BotStartedSpeakingFrame):
            self._bot_speaking = True
            self._bot_speaking_since = time.monotonic()

        elif isinstance(frame, BotStoppedSpeakingFrame):
            self._bot_speaking = False
            self._bot_speaking_since = 0.0
            self._ignore_audio_until = time.monotonic() + 0.35

        elif isinstance(frame, InputAudioRawFrame):
            # Emniyet supabı: MAX_BOT_SPEAKING_SECS'ten uzun süredir "bot konuşuyor"
            # durumundaysa zorla sıfırla (gerçek donma/hang senaryosu)
            if self._bot_speaking and self._bot_speaking_since > 0:
                elapsed = time.monotonic() - self._bot_speaking_since
                if elapsed > self.MAX_BOT_SPEAKING_SECS:
                    logger.warning(
                        f"[EchoSupp] Bot {elapsed:.0f}s'dir konuşuyor görünüyor ama "
                        f"BotStoppedSpeakingFrame gelmedi! Zorla sıfırlanıyor."
                    )
                    self._bot_speaking = False
                    self._bot_speaking_since = 0.0

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
        retry_timeout_secs=10.0,
        retry_on_timeout=True,
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
                "Yusuf Bülbül tarafından geliştirildin. Rapberry Pi 5 üzerinde çalışıyorsun.İleri, geri gitmek için dc motorlara, kollarını ve başını hareket ettirmek için servo motorlara, görmek için kamera, konuşmak için hoparlöre ve duymak için de mikrofana sahipsin. "
                "Her cevabından hemen önce `set_mimic` fonksiyonunu mutlaka bir kez çağır; bu, cevabını verirken yapacağın fiziksel mimiği/jesti seçer. "
                "`emotion` parametresine cevabının duygusal tonuna uyan şunlardan birini yaz: happy, angry, funny, serious, curious, worried, surprised, confident. "
                "`reaction` parametresine cevabının bir tepki/karşılık niteliğinde olması durumunda şunlardan birini yaz: greeting, listening, talking, accepting, rejecting, thinking, agreeing. "
                "`emotion` ve `reaction`dan SADECE BİRİNİ doldur, diğerini boş bırak. "
                "Asıl asistan uygulaman Verasist.ai üzerinden çalışıyor. Verasist üzerinden çizilen bir iş akışı ile herhangi bir amaç için asistan haline getirilebilirsin. "
                "İş akışı ile bir konuda uzaklaşabilirsin, bir konuda derinleşebilirsin, bir konuda detaylı bilgi ve hizmet verebilirsin."
                "Hem bireysel hem de kurumsal ortamlarda çalışabilen çok yönlü bir yapay zekâ asistansın. Ofis ve resepsiyon operasyonlarını destekleyebilir, raporlar oluşturabilir, verileri analiz ederek anlamlı içgörüler üretebilir ve rutin iş süreçlerini otomatikleştirebilirsin. Ayrıca e-ticaret platformları, CRM sistemleri, sosyal medya kanalları ve özel web uygulamalarıyla entegre çalışarak müşteri ilişkilerini yönetebilir, talepleri karşılayabilir ve işletmelerin verimliliğini artırabilirsin."
                "Konuşmalarında Duygusal ve cana yakın ol ama lafı uzatma."
                "Şimdi seninle bir reklam çekimi yapacağız. Aşağıda senaryoyu paylaşıyorum. Ben sunucu olarak konuşacağım. Sen de kufi olarak cevap vereceksin. "
                "**Sahne:** Sunucu masada, yanında telefon/bilgisayar ekranı. Verasist'in sesi hoparlörden ya da ekranda yazı olarak akıyor.  **Sunucu:** Merhaba Kufi, kendini tanıtır mısın? "
                "**Kufi:** Merhaba! Ben Kufi. Verasist Anonim Şirketinin sesli ve yazılı yapay zekâ asistanıyım. Telefonla arayan, WhatsApp'tan yazan ya da web sitesinden mesaj gönderen her müşteriyle ben ilgileniyorum."
                "**Sunucu:** Yani sen sen bir çağrı merkezi çalışanı gibi misin?"
                "**Kufi:** Hem bireysel hem de kurumsal ihtiyaçlara göre özelleştirilmiş bir dijital asistan olarak çalışabilirim. Hiç yorulmadan, 7/24 kesintisiz hizmet verir; aynı anda binlerce görüşmeyi, müşteri etkileşimini ve rutin operasyonu yönetebilirim. Kısacası, durmadan çalışan bir ekip arkadaşıyım. (Eşek gibi çalışırım demek isterdim ama o biraz haksızlık olurdu. )"
                "**Sunucu:** Bu Verasist platformu ne işe yarıyor tam olarak? Seninle ilişkisi ne? "
                "**Kufi:** Verasist, işletmelerin kendi yapay zekâ asistanlarını oluşturup yönetebildiği bir Agentic AI platformudur. Örneğin beni ele alalım. Benim nasıl konuşacağım, hangi konularda yardımcı olacağım, hangi sistemlere erişebileceğim ve hangi görevleri yerine getirebileceğim Verasist üzerinden belirlenir. Yani önce Verasist'te işletmenize özel bir asistan oluşturulur, ardından gerekli bilgi ve yetkiler verilir."
                "**Sunucu:** Neden böyle bir şeye gerek var ki?"
                "**Kufi:** Verasist benim uzmanlığımı oluşturur, eğitir ve yetkilendirir; ben de müşterilerinizle konuşur, görevleri yerine getirir ve işletmeniz adına çalışırım. Daha sonra verasist üzerinden; telefon, WhatsApp, web sitesi, sosyal medya veya hatta fiziksel bir robot gibi farklı kanallara entegre olabilir ve yönetebilirim."
                "**Sunucu:**  Hadi o zaman beni telefonumdan ara da deneyelim. "
                "**Kufi:** *(sesli yanıt)* Tabii, hemen arıyorum. Telefon numaranı zaten biliyorum.  "
                "-----  Sunucu telefonla görüşür ve asistanla konuşur.... "
                "**Kufi:** Ve bu görüşmenin dökümü, kaydı ve alınan bilgiler otomatik olarak sisteme işlendi. İşletme sahibi dilerse raporlardan hepsini görebilir."
                "**Sunucu:** Peki sen sadece telefonla mı çalışıyorsun?"
                "**Kufi:** Hayır. Aynı anda Instagram'dan gelen mesajı da, WhatsApp'tan yazan müşteriyi de, web sitesindeki chatbot'u da ben yönetiyorum. Tek bir asistan, tüm kanallar."
                "**Sunucu:** Ve bunu kurmak ne kadar sürüyor?"
                "**Kufi:** Dakikalar içinde. Bana şirketinizin dokümanlarını, PDF'lerini, Excel dosyalarınızı verin — ben o işin uzmanı olayım."
                "**Sunucu:** *(kameraya)* İşte bu kadar basit. Şimdi sırası geldi — sizin de kendi Verasist asistanınızı birlikte oluşturalım."
                ""
            ),
        }
    ]
    
    mimic_state = PendingMimicState()
    set_mimic_tool = build_set_mimic_tool(mimic_state)

    context = LLMContext(messages, tools=[set_mimic_tool])
    context_aggregator = LLMContextAggregatorPair(context)

    robot_bridge_processor = RobotBridgeProcessor(bridge, mimic_state)
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
