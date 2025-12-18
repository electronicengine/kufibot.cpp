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


#include "rag_service.h"
#include "../logger.h"
#include "tui_service.h"
#include <sqlite3.h>
#include <random>
#include "landmark_tracker_service.h"
#include "remote_connection_service.h"
#include "gesture_performer_service.h"
#include "../operators/speech_recognizing_operator.h"

RagService* RagService::_instance = nullptr;

RagService *RagService::get_instance() {
    if (_instance == nullptr) {
        _instance = new RagService();
    }
    return _instance;
}


RagService::~RagService() {

}


RagService::RagService() : Service("RagService") {

}


bool RagService::initialize() {
    auto embeddingConf = JsonParserOperator::get_instance()->getAiConfig()->llmEmbeddingConfig;

    auto parser = JsonParserOperator::get_instance();
    if (parser->getEmotionalList().has_value()) {
        _emotionalList = parser->getEmotionalList().value();
    }else {
        ERROR("Emotional List is not found!");
        return false;
    }

    if (parser->getReactionalList().has_value()) {
        _reactionalList = parser->getReactionalList().value();
    }else {
        ERROR("Reactional List is not found!");
        return false;
    }

    if (parser->getDirectiveList().has_value()) {
        _directiveList = parser->getDirectiveList().value();
    }else {
        ERROR("Directive List is not found!");
        return false;
    }


    _embeddingOperator.loadEmbedModel(embeddingConf.modelPath, (enum llama_pooling_type) embeddingConf.poolingType);
    INFO("Embedding Model is Loaded!");
    loadDatabase();
    calculate_gesture_embeddings();

    subscribe_to_service(TuiService::get_instance());
    subscribe_to_service(RemoteConnectionService::get_instance());
    subscribe_to_service(GesturePerformerService::get_instance());
    subscribe_to_service(LandmarkTrackerService::get_instance());

    INFO("Speech Recognizing Model is loading...");
    auto speechConfig = parser->getAiConfig()->speechRecognizerConfig;
    auto* recognizer = SpeechRecognizingOperator::get_instance(speechConfig.silenceThreshold, speechConfig.sampleRate, speechConfig.framesPerBuffer,
                                                              speechConfig.maxSilenceDurationSec, speechConfig.listenTimeoutMs, speechConfig.command);

    recognizer->load_model(parser->getAiConfig()->speechRecognizerConfig.modelPath);
    recognizer->open();
    return true;
}

void RagService::speak(std::string text) {
    INFO("Speaking: {}", text);
    std::unique_ptr<MessageData> data = std::make_unique<SpeakRequestData>();
    static_cast<SpeakRequestData *>(data.get())->text = text;
    publish(MessageType::SpeakRequest, data);
}


void RagService::service_function() {

    auto aiConfig = JsonParserOperator::get_instance()->getAiConfig();
    auto *recognizer = SpeechRecognizingOperator::get_instance();
    recognizer->start_listen();

    while (_running) {
        std::vector<int16_t> buffer = recognizer->get_buffer();

        if (!buffer.empty()) {
            publish(MessageType::StopVideoStreamRequest);
            std::string message = recognizer->get_text(buffer);
            INFO("Recognized Message: {}", message);
            if (!message.empty()) {
                std::string response = getRAGResponse(message);
                INFO("Response: {}", response);
                divideAndPublish(response);
            }

            publish(MessageType::StartVideoStreamRequest);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    recognizer->stop_listen();

}


void RagService::calculate_gesture_embeddings() {

    INFO("Calculating Emotional Embeddings...");
    for (auto& emotion : _emotionalList) {
        emotion.embedding = _embeddingOperator.calculateEmbeddings(emotion.description);
    }

    INFO("Calculating Reactional Embeddings...");
    for (auto &reaction : _reactionalList) {
        reaction.embedding = _embeddingOperator.calculateEmbeddings(reaction.description);
    }

    INFO("Calculating Directive Embeddings...");
    for (auto &directive : _directiveList) {
        directive.embedding = _embeddingOperator.calculateEmbeddings(directive.description);
    }
}

void RagService::updateRAGDatabase() {
    INFO("updateRAGDatabase");
    auto &ragDataset = JsonParserOperator::get_instance()->getRagDataset();

    // SQLite bağlantısı oluştur
    sqlite3 *db;
    int rc = sqlite3_open_v2("/home/kufi/rag_dataset.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        ERROR("Cannot open or create SQLite database: {}", sqlite3_errstr(rc));
        return;
    }


    // // Tablo oluştur
    const char *createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS rag_dataset (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            input TEXT,
            output TEXT,
            inputEmbedding BLOB
        );
    )";
    rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        ERROR("Failed to create table: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    for (auto &row: ragDataset->rows) {
        row.inputEmbedding = _embeddingOperator.calculateEmbeddings(row.input);

        const char *insertSQL = "INSERT INTO rag_dataset (input, output, inputEmbedding) VALUES (?, ?, ?);";
        sqlite3_stmt *stmt = nullptr;
        rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            ERROR("Failed to prepare insert statement: {}", sqlite3_errmsg(db));
            continue;
        }

        rc = sqlite3_bind_text(stmt, 1, row.input.c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            ERROR("Failed to bind input text: {}", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            continue;
        }

        rc = sqlite3_bind_text(stmt, 2, row.output.c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            ERROR("Failed to bind output text: {}", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            continue;
        }

        if (!row.inputEmbedding.empty()) {
            const void *blobData = static_cast<const void *>(row.inputEmbedding.data());
            int blobSize = static_cast<int>(row.inputEmbedding.size() * sizeof(float));
            rc = sqlite3_bind_blob(stmt, 3, blobData, blobSize, SQLITE_TRANSIENT);
            if (rc != SQLITE_OK) {
                ERROR("Failed to bind embedding blob: {}", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                continue;
            }
        } else {
            rc = sqlite3_bind_null(stmt, 3);
            if (rc != SQLITE_OK) {
                ERROR("Failed to bind null for embedding: {}", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                continue;
            }
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            ERROR("Failed to execute insert statement: {}", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
    }


    // SQLite bağlantısını kapat
    sqlite3_close(db);
}


void RagService::showRAGDatabase() {
    INFO("showRAGDatabase");

    sqlite3 *db;
    int rc = sqlite3_open_v2("/home/kufi/rag_dataset.db", &db, SQLITE_OPEN_READONLY, nullptr);
    if (rc != SQLITE_OK) {
        ERROR("Cannot open SQLite database: {}", sqlite3_errstr(rc));
        return;
    }

    const char *selectSQL = "SELECT id, input, output, inputEmbedding FROM rag_dataset;";
    sqlite3_stmt *stmt = nullptr;

    rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        ERROR("Failed to prepare select statement: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    INFO("=== Dumping rag_dataset table ===");

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);

        const unsigned char *inputText = sqlite3_column_text(stmt, 1);
        const unsigned char *outputText = sqlite3_column_text(stmt, 2);

        int blobSize = sqlite3_column_bytes(stmt, 3);

        // LOG string olarak
        std::string inputStr = inputText ? reinterpret_cast<const char *>(inputText) : "";
        std::string outputStr = outputText ? reinterpret_cast<const char *>(outputText) : "";

        INFO("Row id={} input='{}' output='{}' embeddingSize={}", id, inputStr, outputStr, blobSize);
        //const void *blobData = sqlite3_column_blob(stmt, 3);

        // Eğer istersen blob’u float vektör olarak da yazabilirsin:
        // if (blobData && blobSize > 0) {
        //     const float* floatData = reinterpret_cast<const float*>(blobData);
        //     int floatCount = blobSize / sizeof(float);
        //     for (int i = 0; i < std::min(floatCount, 5); ++i) { // ilk 5 elemanı göster
        //         std::cout << floatData[i] << " ";
        //     }
        //     if (floatCount > 5) std::cout << "...";
        //     std::cout << std::endl;
        // }
    }

    if (rc != SQLITE_DONE) {
        ERROR("Error while reading database: {}", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    INFO("=== Finished dumping rag_dataset table ===");
}

void RagService::clearRAGDatabase() {
    INFO("clearRAGDatabase");

    sqlite3 *db;
    int rc = sqlite3_open_v2("/home/kufi/rag_dataset.db", &db, SQLITE_OPEN_READWRITE, nullptr);
    if (rc != SQLITE_OK) {
        ERROR("Cannot open SQLite database: {}", sqlite3_errstr(rc));
        return;
    }

    const char *deleteSQL = "DELETE FROM rag_dataset;"; // Tüm satırları sil
    char *errMsg = nullptr;

    rc = sqlite3_exec(db, deleteSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        ERROR("Failed to clear rag_dataset table: {}", errMsg);
        sqlite3_free(errMsg);
    } else {
        INFO("All rows in rag_dataset table have been cleared.");
    }

    // Opsiyonel: AUTOINCREMENT resetlemek istersen
    // sqlite3_exec(db, "DELETE FROM sqlite_sequence WHERE name='rag_dataset';", nullptr, nullptr, &errMsg);

    sqlite3_close(db);
}


void RagService::loadDatabase() {
    _ragEmbeddings.clear();
    INFO("Loading embeddings...");

    sqlite3 *db = nullptr;
    int rc = sqlite3_open_v2("/home/kufi/rag_dataset.db", &db, SQLITE_OPEN_READONLY, nullptr);
    if (rc != SQLITE_OK) {
        // handle error
       ERROR("Cannot open SQLite database: {} ", sqlite3_errstr(rc));
        return;
    }

    const char *selectSQL = "SELECT inputEmbedding FROM rag_dataset;";
    sqlite3_stmt *stmt = nullptr;
    rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        ERROR("Failed to prepare select statement: {}", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int blobSize = sqlite3_column_bytes(stmt, 0);
        const void *blobData = sqlite3_column_blob(stmt, 0);

        if (blobData && blobSize > 0) {
            int floatCount = blobSize / sizeof(float);
            // Read the embedding vector for this row
            const float* floatData = reinterpret_cast<const float*>(blobData);
            std::vector<float> embedding(floatData, floatData + floatCount);
            _ragEmbeddings.push_back(std::move(embedding));
        }
    }

    INFO("Loaded {} size line of embeddings.", _ragEmbeddings.size());

    if (rc != SQLITE_DONE) {
        ERROR("Error while reading database: {}", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


std::string RagService::getOutputForId(int id) {

    INFO("getOutputForId {}", id);

    const std::string &dbPath = "/home/kufi/rag_dataset.db";
    std::string output;
    sqlite3 *db = nullptr;
    int rc = sqlite3_open_v2(dbPath.c_str(), &db, SQLITE_OPEN_READONLY, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open SQLite database: " << sqlite3_errstr(rc) << std::endl;
        return output;
    }

    const char *selectSQL = "SELECT output FROM rag_dataset WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return output;
    }

    rc = sqlite3_bind_int(stmt, 1, id);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind id: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return output;
    }

    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char *outputText = sqlite3_column_text(stmt, 0);
        output = outputText ? reinterpret_cast<const char *>(outputText) : "";
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return output;
}


std::string RagService::getRAGResponse(const std::string &query) {

    auto queryEmbedding = _embeddingOperator.calculateEmbeddings(query);
    int id = 0;
    std::vector<std::pair<int, float>> results;

    // calculate the similarities
    for (const auto &embedding: _ragEmbeddings) {
        float similarity = _embeddingOperator.getSimilarity(queryEmbedding, embedding);
        results.push_back({++id, similarity});
    }


    // sort of decreasing...
    std::sort(results.begin(), results.end(),
              [](const std::pair<int, float> &a, const std::pair<int, float> &b) { return a.second > b.second; });

    // for (auto embedding: results) {
    //     INFO("id: {} similarity: {}", embedding.first, embedding.second);
    // }
    if (results[0].second >= 0.95) {
        int pick;
        do {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dist(0, 4);
            pick = dist(gen);
            //INFO("Random pick index: {} -> id {} sim: {}", pick, results[pick].first, results[pick].second );

        } while (results[pick].second < 0.9);

        return getOutputForId(results[pick].first);

    } else {
        return getOutputForId(results[0].first);
    }
}


std::pair<EmotionalGesture,float> RagService::find_sentence_emotion(const std::string &sentence) {
    float max_similarity = 0.0f;
    EmotionalGesture best_match;
    std::vector<float> sentence_embeddings = _embeddingOperator.calculateEmbeddings(sentence);

    for (auto emotion : _emotionalList) {

        float sim = _embeddingOperator.getSimilarity(sentence_embeddings, emotion.embedding);

        if (sim > max_similarity && sim > 0.5f ) {
            max_similarity = sim;
            best_match = emotion;
        }
    }

    return std::make_pair(best_match, max_similarity);
}

std::pair<ReactionalGesture, float> RagService::find_sentence_reaction(const std::string &sentence) {
    float max_similarity = 0.0f;
    ReactionalGesture best_match;
    std::vector<float> sentence_embeddings = _embeddingOperator.calculateEmbeddings(sentence);

    for (auto reaction : _reactionalList) {
        float sim = _embeddingOperator.getSimilarity(sentence_embeddings, reaction.embedding);

        if (sim > max_similarity && sim > 0.5f ) {
            max_similarity = sim;
            best_match = reaction;
        }
    }

    return std::make_pair(best_match, max_similarity);
}

std::pair<Directive, float> RagService::find_sentence_directive(const std::string &sentence) {
    float max_similarity = 0.0f;
    Directive best_match;
    std::vector<float> sentence_embeddings = _embeddingOperator.calculateEmbeddings(sentence);

    for (auto directive : _directiveList) {
        float sim = _embeddingOperator.getSimilarity(sentence_embeddings, directive.embedding);

        if (sim > max_similarity && sim > 0.5f ) {
            max_similarity = sim;
            best_match = directive;
        }
    }

    return std::make_pair(best_match, max_similarity);
}



std::vector<std::string> RagService::splitIntoSentences(const std::string& paragraph) {
    std::vector<std::string> sentences;

    std::regex sentence_regex(R"([^.!?]*[.!?]+)");

    std::sregex_iterator iter(paragraph.begin(), paragraph.end(), sentence_regex);
    std::sregex_iterator end;

    while (iter != end) {
        std::string sentence = iter->str();

        // Başındaki ve sonundaki boşlukları temizle
        sentence.erase(0, sentence.find_first_not_of(" \t\n\r"));
        sentence.erase(sentence.find_last_not_of(" \t\n\r") + 1);

        // Boş cümleleri ekleme
        if (!sentence.empty()) {
            sentences.push_back(sentence);
        }

        ++iter;
    }

    return sentences;
}

void RagService::divideAndPublish(const std::string &response) {
    
    std::vector<std::string> sentences = splitIntoSentences(response);
    for (auto sentence: sentences) {
        std::unique_ptr<MessageData> data = std::make_unique<LLMResponseData>();
        //INFO("{}", sentence);

        static_cast<LLMResponseData *>(data.get())->sentence = sentence;
        auto emotionPair = find_sentence_emotion(sentence);
        static_cast<LLMResponseData *>(data.get())->emotionalGesture = emotionPair.first;
        static_cast<LLMResponseData *>(data.get())->emotionSimilarity = emotionPair.second;
        //INFO("emotionalGesture: {} sim: {}", emotionPair.first.symbol, find_sentence_emotion(sentence).second);

        auto reactionPair = find_sentence_reaction(sentence);
        static_cast<LLMResponseData *>(data.get())->reactionalGesture = reactionPair.first;
        static_cast<LLMResponseData *>(data.get())->reactionSimilarity = reactionPair.second;
        //INFO("reactionalGesture: {} sim: {}", reactionPair.first.symbol, reactionPair.second);

        static_cast<LLMResponseData *>(data.get())->endMarker = sentence == sentences.back();
        publish(MessageType::LLMResponse, data);

    }

}


void RagService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData> &data) {

    switch (type) {
        case MessageType::LLMQuery: {
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);

                std::string queryMsg = static_cast<LLMQueryData *>(data.get())->query;
                std::string response = getRAGResponse(queryMsg);
                divideAndPublish(response);

            }
            break;
        }

        case  MessageType::EngageReaction: {
            std::lock_guard<std::mutex> lock(_dataMutex);
            std::string reaction = static_cast<EngageReactionData *>(data.get())->reaction;
            divideAndPublish(reaction);
            break;
        }

        case  MessageType::UpdateRAGDatabaseRequest: {
            std::lock_guard<std::mutex> lock(_dataMutex);

            updateRAGDatabase();
            break;
        }

        case MessageType::ClearRAGDatabaseRequest: {
            std::lock_guard<std::mutex> lock(_dataMutex);

            clearRAGDatabase();
            break;
        }

        case MessageType::ShowRAGDatabaseRequest: {
            std::lock_guard<std::mutex> lock(_dataMutex);

            showRAGDatabase();
            break;
        }

        default:
            break;
    }
}
