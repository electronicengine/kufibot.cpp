import cv2
import mediapipe as mp
import time
import math
import numpy as np # NumPy ekledik, çünkü C++'tan gelen görüntüler NumPy dizisi olarak gelecek

class HandTrackingDynamic:
    def __init__(self, mode=False, maxHands=2, detectionCon=0.5, trackCon=0.5):
        self.__mode__ = mode
        self.__maxHands__ = maxHands
        self.__detectionCon__ = detectionCon
        self.__trackCon__ = trackCon
        self.handsMp = mp.solutions.hands
        self.hands = self.handsMp.Hands(
            static_image_mode=mode,
            max_num_hands=maxHands,
            min_detection_confidence=detectionCon,
            min_tracking_confidence=trackCon
        )
        self.mpDraw = mp.solutions.drawing_utils
        self.tipIds = [4, 8, 12, 16, 20]

    def findFingers(self, frame, draw=True):
        imgRGB = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        self.results = self.hands.process(imgRGB)
        # Görüntü üzerine çizim yapılması C++ tarafında yapılacak
        return frame # Sadece işlem yapılmış frame'i döndürüyoruz

    def findPosition(self, frame, handNo=0, draw=True):
        xList = []
        yList = []
        bbox = []
        self.lmsList = []
        if self.results.multi_hand_landmarks:
            if handNo < len(self.results.multi_hand_landmarks):
                myHand = self.results.multi_hand_landmarks[handNo]
                for id, lm in enumerate(myHand.landmark):
                    h, w, c = frame.shape
                    cx, cy = int(lm.x * w), int(lm.y * h)
                    xList.append(cx)
                    yList.append(cy)
                    self.lmsList.append([id, cx, cy])

                if xList and yList:
                    xmin, xmax = min(xList), max(xList)
                    ymin, ymax = min(yList), max(yList)
                    bbox = xmin, ymin, xmax, ymax
        return self.lmsList, bbox

    def findFingerUp(self):
        fingers = []
        if not self.lmsList:
            return []

        if self.lmsList[self.tipIds[0]][1] > self.lmsList[self.tipIds[0]-1][1]:
            fingers.append(1)
        else:
            fingers.append(0)

        for id in range(1, 5):
            if self.lmsList[self.tipIds[id]][2] < self.lmsList[self.tipIds[id]-2][2]:
                fingers.append(1)
            else:
                fingers.append(0)
        return fingers

    def findDistance(self, p1, p2, frame, draw=True, r=15, t=3):
        if not self.lmsList or p1 >= len(self.lmsList) or p2 >= len(self.lmsList):
            return 0, frame, []

        x1, y1 = self.lmsList[p1][1], self.lmsList[p1][2]
        x2, y2 = self.lmsList[p2][1], self.lmsList[p2][2]
        cx, cy = (x1 + x2) // 2, (y1 + y2) // 2

        length = math.hypot(x2 - x1, y2 - y1)
        return length, frame, [x1, y1, x2, y2, cx, cy]

    def detectGesture(self):
        if not self.lmsList:
            return "No Hand"

        fingers = self.findFingerUp()

        if all(f == 1 for f in fingers):
            return "Open Hand"
        elif all(f == 0 for f in fingers):
            return "Fist"
        elif fingers == [0, 1, 0, 0, 0]:
            return "Pointer Finger"
        elif fingers == [0, 1, 1, 0, 0]:
            return "Peace Sign"
        elif fingers == [1, 0, 0, 0, 1]:
            return "Phone Hand"
        elif fingers == [0, 0, 0, 0, 1]:
            return "Pinky Finger"

        if fingers[1] == 0 and fingers[2] == 0 and fingers[3] == 0 and fingers[4] == 0:
            if self.lmsList:
                len_thumb_index, _, _ = self.findDistance(4, 8, np.zeros((1,1,3), dtype=np.uint8), draw=False)
                if len_thumb_index < 50:
                    return "OK Sign"

        return "Unknown Gesture"

# C++ tarafından çağrılacak ana fonksiyon
detector = None # Detector'ı global olarak tanımla veya her çağrıda oluştur

def initialize_detector():
    global detector
    if detector is None:
        detector = HandTrackingDynamic()
    return True # Başarılı olduğunu belirtmek için

def process_frame(frame_data, rows, cols, channels):
    global detector
    if detector is None:
        print("Detector not initialized. Call initialize_detector() first.")
        return "ERROR: Detector not initialized", [], []

    # C++'tan gelen bayt dizisini NumPy dizisine dönüştür
    # Reshape ile (rows, cols, channels) boyutuna getir
    frame = np.frombuffer(frame_data, dtype=np.uint8).reshape((rows, cols, channels))

    detector.findFingers(frame, draw=False) # findFingers'ın draw parametresini False yapın
    lmsList, bbox = detector.findPosition(frame, draw=False) # findPosition'ın draw parametresini False yapın

    gesture = "No Hand"
    if len(lmsList) != 0:
        gesture = detector.detectGesture()

    # Landmarkları ve bbox'ı C++'a geri döndürmek için listelere dönüştür
    # lmsList: [[id, cx, cy], ...]
    # bbox: [xmin, ymin, xmax, ymax]

    # Python listelerini C++'a uygun bir formatta döndürmek için özel bir işlem gerekir.
    # Şimdilik string ve düz listeler döndürelim. C++ tarafında bunu ayrıştırmak gerekecek.

    flat_lms_list = [item for sublist in lmsList for item in sublist] # lmsList'i düzleştir
    flat_bbox = list(bbox) if bbox else [] # bbox'ı listeye çevir

    return gesture, flat_lms_list, flat_bbox