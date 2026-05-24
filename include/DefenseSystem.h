#ifndef AIRDEFENSESIM_DEFENSESYSTEM_H
#define AIRDEFENSESIM_DEFENSESYSTEM_H

#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include <queue>
#include <mutex>

#include <GLFW/glfw3.h>
#include "Logger.h"
#include "Threat.h"
#include "Interceptor.h"

/**
 * @struct PendingLaunch
 * @brief Represents an interception mission that is scheduled for a future time.
 * Used by the priority queue to manage delayed launches.
 */
struct PendingLaunch {
    Interceptor* interceptor; ///< The battery assigned to the launch
    Threat* target;           ///< The specific threat to be intercepted
    double launchTime;        ///< The calculated simulation time for the launch

    // Overload for priority queue (Min-Heap based on launch time)
    bool operator<(const PendingLaunch& other) const {
        return launchTime > other.launchTime;
    }
};

/**
 * @class DefenseSystem
 * @brief The central controller of the air defense simulation.
 * * This class manages the lifecycle of all entities, executes the sector-based
 * logic, and synchronizes the simulation state between logic and rendering.
 */
class DefenseSystem {
private:
    std::vector<Threat*> threats;          ///< Container for all tracked threats
    std::vector<Interceptor*> inventory;  ///< Container for all deployed batteries
    Logger* sysLogger;                    ///< Pointer to the system logging utility

    double currentSimTime;                ///< Cumulative simulation time
    std::priority_queue<PendingLaunch> pendingLaunches; ///< Queue for scheduled interceptions

public:
    /**
     * @brief Mutex for thread-safe access to simulation data.
     * Prevents race conditions between the main loop and user input callbacks.
     */
    std::mutex simMutex;

    /**
     * @brief Renders all system entities (Threats and Interceptors) to the screen.
     */
    void draw() const;

    /**
     * @brief Constructor - Initializes the system with a specific logger.
     */
    DefenseSystem(Logger* logger);

    /**
     * @brief Destructor - Ensures proper deallocation of all dynamic entities.
     */
    ~DefenseSystem();

    /**
     * @brief Registers a new threat into the tracking system.
     */
    void addThreat(Threat* t);

    /**
     * @brief Adds a new interceptor battery to the defensive array.
     */
    void addInterceptor(Interceptor* i);

    /**
     * @brief Executes the primary decision-making algorithm based on geographic sectors.
     */
    void runSectorAlgorithm();

    /**
     * @brief Advances the simulation state by a time step (dt).
     * @param dt The time difference since the last update.
     */
    void updateWorld(double dt);

    /**
     * @brief Resets the simulation to its initial state, clearing all data.
     */
    void reset();
};

#endif //AIRDEFENSESIM_DEFENSESYSTEM_H