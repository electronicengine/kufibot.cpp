//
// Created by ybulb on 7/12/2025.
//

#include "logger.h"

bool Logger::_useTui = true;
MainWindow *Logger::_mainWindow = nullptr;
std::list<CachedLog>  Logger::_cachedLogs;
