import cv2
import mediapipe as mp
import time
import math
import numpy as np

class FaceEmotionDetector:
    def __init__(self, min_detection_confidence=0.5, min_tracking_confidence=0.5):
        self.min_detection_confidence = min_detection_confidence
        self.min_tracking_confidence = min_tracking_confidence

        self.mp_face_mesh = mp.solutions.face_mesh
        self.mp_face_detection = mp.solutions.face_detection
        self.mp_drawing = mp.solutions.drawing_utils
        self.mp_drawing_styles = mp.solutions.drawing_styles

        self.face_mesh = self.mp_face_mesh.FaceMesh(
            max_num_faces=1,
            refine_landmarks=True,
            min_detection_confidence=min_detection_confidence,
            min_tracking_confidence=min_tracking_confidence
        )

        self.face_detection = self.mp_face_detection.FaceDetection(
            min_detection_confidence=min_detection_confidence
        )

        self.LEFT_EYE = [33, 7, 163, 144, 145, 153, 154, 155, 133, 173, 157, 158, 159, 160, 161, 246]
        self.RIGHT_EYE = [362, 382, 381, 380, 374, 373, 390, 249, 263, 466, 388, 387, 386, 385, 384, 398]
        self.MOUTH = [78, 191, 80, 81, 82, 13, 312, 311, 310, 415, 308, 324, 318, 402, 317, 14, 87, 178, 88, 95]
        self.EYEBROW_LEFT = [70, 63, 105, 66, 107, 55, 65, 52, 53, 46]
        self.EYEBROW_RIGHT = [296, 334, 293, 300, 276, 283, 282, 295, 285, 336]

        self.previous_emotion = "Neutral"
        self.emotion_history = []
        self.history_size = 10
        self.landmarks = []
        self.face_results = None
        self.mesh_results = None

    def detect_faces(self, frame):
        """Yüzleri tespit eder."""
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        self.face_results = self.face_detection.process(rgb_frame)
        return self.face_results.detections if self.face_results else []

    def get_face_landmarks(self, frame):
        """Yüz landmark'larını tespit eder"""
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        self.mesh_results = self.face_mesh.process(rgb_frame)

        self.landmarks = []
        if self.mesh_results.multi_face_landmarks:
            h, w, c = frame.shape
            for face_landmarks in self.mesh_results.multi_face_landmarks:
                for id, lm in enumerate(face_landmarks.landmark):
                    cx, cy = int(lm.x * w), int(lm.y * h)
                    self.landmarks.append([id, cx, cy])

        return self.landmarks

    def calculate_distance(self, point1_idx, point2_idx):
        """İki nokta arasındaki mesafeyi hesaplar"""
        if not self.landmarks or len(self.landmarks) <= max(point1_idx, point2_idx):
            return 0

        x1, y1 = self.landmarks[point1_idx][1], self.landmarks[point1_idx][2]
        x2, y2 = self.landmarks[point2_idx][1], self.landmarks[point2_idx][2]
        return math.hypot(x2 - x1, y2 - y1)

    def get_eye_aspect_ratio(self, eye_points):
        """Göz açıklık oranını hesaplar (EAR - Eye Aspect Ratio)"""
        if not self.landmarks:
            return 0

        # Dikey mesafeler
        vertical_1 = self.calculate_distance(eye_points[1], eye_points[5])
        vertical_2 = self.calculate_distance(eye_points[2], eye_points[4])

        # Yatay mesafe
        horizontal = self.calculate_distance(eye_points[0], eye_points[3])

        if horizontal == 0:
            return 0

        ear = (vertical_1 + vertical_2) / (2.0 * horizontal)
        return ear

    def get_mouth_aspect_ratio(self):
        """Ağız açıklık oranını hesaplar (MAR - Mouth Aspect Ratio)"""
        if not self.landmarks:
            return 0

        # Ağız dikey mesafesi (üst-alt)
        vertical_1 = self.calculate_distance(13, 14)

        # Ağız yatay mesafesi
        horizontal = self.calculate_distance(78, 308)

        if horizontal == 0:
            return 0

        mar = vertical_1 / horizontal
        return mar

    def get_eyebrow_position(self):
        """Kaş pozisyonunu hesaplar (yüksek/alçak)"""
        if not self.landmarks:
            return 0

        # Sol kaş ve göz arasındaki mesafe
        left_eyebrow_height = self.calculate_distance(70, 159)
        # Sağ kaş ve göz arasındaki mesafe
        right_eyebrow_height = self.calculate_distance(296, 386)

        avg_eyebrow_height = (left_eyebrow_height + right_eyebrow_height) / 2
        return avg_eyebrow_height

    def detect_emotion(self):
        """Yüz ifadesinden emotion tespit eder - Geliştirilmiş Versiyon"""
        if not self.landmarks:
            return "No Face"

        left_ear = self.get_eye_aspect_ratio([33, 160, 158, 133, 153, 144])
        right_ear = self.get_eye_aspect_ratio([362, 385, 387, 263, 373, 380])
        avg_ear = (left_ear + right_ear) / 2

        mar = self.get_mouth_aspect_ratio()

        # Kaş pozisyonu (göz merkezinie göre dikey mesafe)
        # 159 ve 386 noktaları gözün üst kapağının merkezine yakın noktalar.
        # Bu değerlerin her yüz tipine göre normalleştirilmesi gerekebilir.
        left_eyebrow_to_eye_dist = self.calculate_distance(159, 10) # 10 burun köküne yakın bir göz merkezi
        right_eyebrow_to_eye_dist = self.calculate_distance(386, 338) # 338 burun köküne yakın bir göz merkezi
        avg_eyebrow_height = (left_eyebrow_to_eye_dist + right_eyebrow_to_eye_dist) / 2

        # Ağız köşeleri ve burun ucu referansı
        mouth_corner_left_y = self.landmarks[308][2] if len(self.landmarks) > 308 else 0
        mouth_corner_right_y = self.landmarks[78][2] if len(self.landmarks) > 78 else 0
        nose_tip_y = self.landmarks[1][2] if len(self.landmarks) > 1 else 0

        # Ağız köşelerinin dikey eğimi (gülümseme için)
        if mouth_corner_left_y and mouth_corner_right_y:
            mouth_corner_avg_y = (mouth_corner_left_y + mouth_corner_right_y) / 2
            smile_intensity = nose_tip_y - mouth_corner_avg_y # Pozitif değerler köşelerin yukarı olduğunu gösterir
        else:
            smile_intensity = 0

        # Kaşların yatay mesafesi (çatık kaşlar için)
        # Sağ kaşın iç noktası (296) ve sol kaşın iç noktası (66)
        eyebrow_inner_distance = self.calculate_distance(296, 66)
        # Normal yüz genişliğine göre normalize edilebilir, örneğin şakaklar arası mesafe.
        # Bu sadece bir örnek, daha sağlam bir referans noktası bulmak gerekebilir.
        face_width = self.calculate_distance(234, 454) # Şakaklar arası mesafe
        if face_width > 0:
            normalized_eyebrow_distance = eyebrow_inner_distance / face_width
        else:
            normalized_eyebrow_distance = 0

        emotion = "Neutral"

        # Duygu Karar Ağacı/Kuralları
        # Not: Bu değerler hala deneme yanılma ile bulunmalıdır veya ML modelinden öğrenilmelidir.

        # 1. Mutluluk
        if smile_intensity > 15 and mar > 0.3: # Daha belirgin gülümseme ve açık ağız
            emotion = "Happy"

        # 2. Şaşkınlık
        elif avg_ear > 0.4 and mar > 0.4 and avg_eyebrow_height > 50: # Geniş açık gözler, açık ağız, yüksek kaşlar
            emotion = "Surprised"

        # 3. Üzüntü
        elif smile_intensity < -10 and mar < 0.2 and avg_ear < 0.3: # Aşağı ağız köşeleri, dar ağız, hafif kısık gözler
            emotion = "Sad"

        # 4. Öfke
        # Çatık kaşlar (normalized_eyebrow_distance küçülür), kısık gözler
        elif normalized_eyebrow_distance < 0.15 and avg_ear < 0.2:
            emotion = "Angry"

        # 5. Korku
        # Çok açık gözler, çok yüksek kaşlar ama ağız şaşkınlık kadar açık olmayabilir veya gergin.
        elif avg_ear > 0.4 and avg_eyebrow_height > 60 and mar < 0.3:
            emotion = "Fear"

        # 6. Uyku/Göz Kırpma
        elif avg_ear < 0.1:
            emotion = "Sleepy/Wink"

        # Duygu geçmişi ve yumuşatma
        self.emotion_history.append(emotion)
        if len(self.emotion_history) > self.history_size:
            self.emotion_history.pop(0)

        if len(self.emotion_history) >= 3: # En az 3 çerçeve aynı duygu için oy kullansın
            # En sık tekrar eden duyguyu bul
            from collections import Counter
            emotion_counts = Counter(self.emotion_history)
            current_emotion_smoothed = emotion_counts.most_common(1)[0][0]
            # Eğer önceki duygu "Neutral" değilse ve yeni duygu "Neutral" ise,
            # ancak baskınlık yoksa, önceki duyguyu koru. Bu bir çeşit yapışkanlık sağlar.
            if self.previous_emotion != "Neutral" and current_emotion_smoothed == "Neutral" and emotion_counts["Neutral"] < len(self.emotion_history) / 2:
                pass # Önceki duyguya sadık kal
            else:
                emotion = current_emotion_smoothed

        self.previous_emotion = emotion
        return emotion

    def get_face_info(self):
        """Yüz analizi bilgilerini döndürür"""
        if not self.landmarks:
            return {}

        left_ear = self.get_eye_aspect_ratio([33, 160, 158, 133, 153, 144])
        right_ear = self.get_eye_aspect_ratio([362, 385, 387, 263, 373, 380])
        mar = self.get_mouth_aspect_ratio()
        eyebrow_height = self.get_eyebrow_position()

        return {
            "left_ear": round(left_ear, 3),
            "right_ear": round(right_ear, 3),
            "avg_ear": round((left_ear + right_ear) / 2, 3),
            "mar": round(mar, 3),
            "eyebrow_height": round(eyebrow_height, 1)
        }

# C++ tarafından çağrılacak ana fonksiyonlar için global detector
face_detector = None

def initialize_detector_face():
    """
    Yüz tanıma dedektörünü başlatır. C++ tarafından bir kez çağrılmalıdır.
    """
    global face_detector
    try:
        if face_detector is None:
            face_detector = FaceEmotionDetector()
        return True
    except Exception as e:
        print(f"Error initializing face detector: {e}")
        return False

def process_frame_face(frame_data, rows, cols, channels):
    """
    C++'tan gelen görüntü verisini işler ve yüz ifadesi sonuçlarını döndürür.

    Args:
        frame_data (bytes): C++'tan gelen görüntü bayt dizisi.
        rows (int): Görüntünün yüksekliği.
        cols (int): Görüntünün genişliği.
        channels (int): Görüntünün kanal sayısı (örneğin BGR için 3).

    Returns:
        tuple: (emotion (str), flat_landmarks (list), face_info (str))
    """
    global face_detector

    try:
        if face_detector is None:
            return "ERROR: Detector not initialized", [], "{}"

        # C++'tan gelen bayt dizisini NumPy dizisine dönüştür
        frame = np.frombuffer(frame_data, dtype=np.uint8).reshape((rows, cols, channels))

        # Yüz tespiti
        face_detector.detect_faces(frame)

        # Landmark tespiti
        landmarks = face_detector.get_face_landmarks(frame)

        # Duygu tespiti
        emotion = "No Face"
        if len(landmarks) != 0:
            emotion = face_detector.detect_emotion()

        # Yüz bilgi metriklerini al
        face_info = face_detector.get_face_info()

        # Landmarkları C++'a geri döndürmek için listeleri düzleştir
        flat_landmarks = [item for sublist in landmarks for item in sublist]

        # Face info'yu string formatında döndür
        face_info_str = str(face_info)

        return emotion, flat_landmarks, face_info_str

    except Exception as e:
        print(f"Error processing frame: {e}")
        return "ERROR", [], "{}"