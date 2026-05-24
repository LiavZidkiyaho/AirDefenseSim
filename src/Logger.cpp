#include "../include/Logger.h"

/*
Input: const std::string& filename - the name/path of the file to create or open.
Output: None (Constructor).
Description: Opens the log file in append mode. If successful, writes a starting header
             to mark the beginning of a new simulation session.
 */
Logger::Logger(const std::string& filename) {
    logFile.open(filename, std::ios::out | std::ios::app);
    if (logFile.is_open()) {
        logFile << "=== SIMULATION LOG STARTED ===" << std::endl;
    } else {
        std::cerr << "Error: Could not open log file!" << std::endl;
    }
}

/*
Input: None (Destructor).
Output: None.
Description: Writes a closing header to the log file and safely closes the file stream
             to ensure all data is saved correctly.
 */
Logger::~Logger() {
    if (logFile.is_open()) {
        logFile << "=== SIMULATION LOG ENDED ===\n" << std::endl;
        logFile.close();
    }
}

/*
Input: const std::string& message - the event description to be recorded.
Output: None.
Description: Writes a custom message to the log file, representing a specific system
             event such as a launch, impact, or system reset.
 */
void Logger::logEvent(const std::string& message) {
    if (logFile.is_open()) {
        logFile << message << std::endl;
    }
    //std::cout << "[LOG] " << message << std::endl;
}