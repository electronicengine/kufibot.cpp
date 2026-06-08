/*
 * This file is part of Kufibot.
 *
 * Kufibot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kufibot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kufibot. If not, see <https://www.gnu.org/licenses/>.
 */

#include "perception_service.h"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <thread>

#include "../logger.h"
#include "../operators/json_parser_operator.h"
#include "../operators/speech_recognizing_operator.h"
#include "remote_connection_service.h"

PerceptionService* PerceptionService::_instance = nullptr;

PerceptionService *PerceptionService::get_instance(bool showFrame, const std::string& name)
{
	if (_instance == nullptr) {
		_instance = new PerceptionService(name, showFrame);
	}
	return _instance;
}

PerceptionService::PerceptionService(const std::string& name, bool showFrame)
	: Service(name), _videoOperator(0), _showFrame(showFrame) {
}

PerceptionService::~PerceptionService() {
	stop();
	_videoOperator.shutdown();
	cv::destroyAllWindows();
}

bool PerceptionService::initialize() {
	subscribe_to_service(RemoteConnectionService::get_instance());

	if (!_videoOperator.initialize()) {
		ERROR("Video operator failed to initialize!");
		return false;
	}
	INFO("Video operator initialized.");

	if (!_faceGestureRecognizingOperator.initialize()) {
		ERROR("Face gesture recognition module failed to initialize!");
		return false;
	}
	INFO("Face gesture recognition module initialized.");

	if (!_handGestureRecognizingOperator.initialize()) {
		ERROR("Hand gesture recognition module failed to initialize!");
		return false;
	}
	INFO("Hand gesture recognition module initialized.");

	auto parser = JsonParserOperator::get_instance();
	auto speechConfig = parser->getAiConfig()->speechRecognizerConfig;

	INFO("Speech Recognizing Model is loading...");
	auto* recognizer = SpeechRecognizingOperator::get_instance(
		speechConfig.silenceThreshold,
		speechConfig.sampleRate,
		speechConfig.framesPerBuffer,
		speechConfig.maxSilenceDurationSec,
		speechConfig.listenTimeoutMs,
		speechConfig.command);

	recognizer->load_model(speechConfig.modelPath);
	if (!recognizer->open()) {
		ERROR("Speech recognizing module failed to open audio stream!");
		return false;
	}
	INFO("Speech recognizing module initialized.");

	return true;
}

void PerceptionService::service_function() {
	auto* recognizer = SpeechRecognizingOperator::get_instance();
	bool speechListening = false;

	if (!_videoOperator.open()) {
		ERROR("Perception service could not open camera.");
		_running = false;
		return;
	}

	while (_running) {
		cv::Mat frame;
		if (!_videoOperator.readFrame(frame)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
			continue;
		}

		publishVideoFrame(frame);

		const bool aiModeEnabled = _aiModeEnabled.load();
		if (aiModeEnabled && !speechListening) {
			if (!recognizer->start_listen()) {
				WARNING("Speech recognizer could not start listening. Perception service will continue with vision only.");
			} else {
				speechListening = true;
			}
		} else if (!aiModeEnabled && speechListening) {
			recognizer->stop_listen();
			speechListening = false;
		}

		if (aiModeEnabled) {
			processFrame(frame);
			processSpeechInput();
		}

		if (_showFrame) {
			cv::imshow("Perception Service", frame);
			cv::waitKey(1);
		}
	}

	if (speechListening) {
		recognizer->stop_listen();
	}
	_videoOperator.close();
}

void PerceptionService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData> &data) {
	(void)data;

	switch (type) {
		case MessageType::AIModeOnCall: {
			_aiModeEnabled = true;
			if (!_running) {
				start();
			}
			break;
		}
		case MessageType::AIModeOffCall: {
			_aiModeEnabled = false;
			if (!_running) {
				start();
			}
			break;
		}
		case MessageType::StartVideoStreamRequest: {
			if (!_running) {
				start();
			}
			break;
		}
		case MessageType::StopVideoStreamRequest: {
			stop();
			break;
		}
		default:
			break;
	}
}

void PerceptionService::publishVideoFrame(const cv::Mat& frame) {
	std::unique_ptr<MessageData> data = std::make_unique<VideoFrameData>();
	data->source = SourceService::perceptionService;
	static_cast<VideoFrameData*>(data.get())->frame = frame.clone();
	publish(MessageType::VideoFrame, data);
}

void PerceptionService::processFrame(cv::Mat& frame) {
	std::unique_ptr<MessageData> data = std::make_unique<RecognizedGestureData>();
	data->source = SourceService::perceptionService;
	auto &faceLandMarks = static_cast<RecognizedGestureData*>(data.get())->faceLandmarks;
	auto &faceEmotion = static_cast<RecognizedGestureData*>(data.get())->faceEmotion;
	auto &faceInfo = static_cast<RecognizedGestureData*>(data.get())->faceInfo;

	auto &handBox = static_cast<RecognizedGestureData*>(data.get())->handBbox;
	auto &handLandMarks = static_cast<RecognizedGestureData*>(data.get())->handLandmarks;
	auto &handGesture = static_cast<RecognizedGestureData*>(data.get())->handGesture;

	faceLandMarks = _faceGestureRecognizingOperator.getFaceLandmarks(frame);

	if (!faceLandMarks.empty()) {
		faceEmotion = _faceGestureRecognizingOperator.detectEmotion();
		faceInfo = _faceGestureRecognizingOperator.getFaceInfo();
	}
	if (_showFrame) {
		_faceGestureRecognizingOperator.drawLandmarks(frame, true);
	}

	_handGestureRecognizingOperator.findFingers(frame, _showFrame);
	int handCount = _handGestureRecognizingOperator.getHandCount();

	for (int handNo = 0; handNo < handCount; handNo++) {
		auto [LandMarks, Box] = _handGestureRecognizingOperator.findPosition(frame, handNo, _showFrame);
		handLandMarks = LandMarks;
		handBox = Box;
		handGesture = _handGestureRecognizingOperator.detectGesture();
	}

	publish(MessageType::RecognizedGesture, data);
}

void PerceptionService::processSpeechInput() {
	auto* recognizer = SpeechRecognizingOperator::get_instance();
	std::vector<int16_t> buffer = recognizer->get_buffer();

	if (buffer.empty()) {
		return;
	}

	std::string recognizedText = recognizer->get_text(buffer);
	if (recognizedText.empty()) {
		return;
	}

	std::unique_ptr<MessageData> data = std::make_unique<RecognizedSpeechData>();
	data->source = SourceService::perceptionService;
	static_cast<RecognizedSpeechData*>(data.get())->text = recognizedText;
	publish(MessageType::RecognizedSpeech, data);
}

std::map<std::string, float> PerceptionService::parseFaceInfoString(const std::string& faceInfoStr) {
	std::map<std::string, float> infoMap;
	std::string cleanedStr = faceInfoStr;

	cleanedStr.erase(std::remove(cleanedStr.begin(), cleanedStr.end(), '{'), cleanedStr.end());
	cleanedStr.erase(std::remove(cleanedStr.begin(), cleanedStr.end(), '}'), cleanedStr.end());
	cleanedStr.erase(std::remove(cleanedStr.begin(), cleanedStr.end(), '\''), cleanedStr.end());
	cleanedStr.erase(std::remove(cleanedStr.begin(), cleanedStr.end(), ' '), cleanedStr.end());

	std::stringstream ss(cleanedStr);
	std::string segment;
	while (std::getline(ss, segment, ',')) {
		size_t colonPos = segment.find(':');
		if (colonPos != std::string::npos) {
			std::string key = segment.substr(0, colonPos);
			std::string valueStr = segment.substr(colonPos + 1);
			try {
				infoMap[key] = std::stof(valueStr);
			} catch (const std::exception& e) {
				ERROR("Error parsing value: {} - {}", valueStr, e.what());
			}
		}
	}
	return infoMap;
}

void PerceptionService::displayFPS(cv::Mat& frame, double& ptime) {
	double ctime = static_cast<double>(cv::getTickCount()) / cv::getTickFrequency();
	double fps = 1.0 / (ctime - ptime);
	ptime = ctime;
	cv::putText(frame, "FPS: " + std::to_string(static_cast<int>(fps)),
				cv::Point(10, frame.rows - 20), cv::FONT_HERSHEY_SIMPLEX, 0.7,
				cv::Scalar(255, 0, 255), 2, cv::LINE_AA);
}

VideoOperator& PerceptionService::get_video_operator() {
	return _videoOperator;
}

std::shared_ptr<cv::VideoCapture> PerceptionService::get_capture() {
	return _videoOperator.getCapture();
}
