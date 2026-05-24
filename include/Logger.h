#ifndef AIRDEFENSESIM_LOGGER_H
#define AIRDEFENSESIM_LOGGER_H

#include <string>
#include <fstream>
#include <iostream>

/**
 * @class Logger
 * @brief Provides file-based logging services for the simulation.
 * * This class handles the creation and management of the simulation log file,
 * allowing the system to record critical events such as launches, interceptions,
 * and ground impacts for post-simulation analysis.
 */
class Logger {
private:
    std::ofstream logFile; ///< The output file stream for recording simulation data

public:
    /**
     * @brief Constructor - Opens the log file and prepares it for writing.
     * @param filename The name or path of the file where logs will be saved.
     */
    Logger(const std::string& filename);

    /**
     * @brief Destructor - Ensures the log file is properly closed and saved.
     */
    ~Logger();

    /**
     * @brief Records a specific event message into the log file.
     * @param message A string containing the details of the event to be logged.
     */
    void logEvent(const std::string& message);
};

#endif //AIRDEFENSESIM_LOGGER_H