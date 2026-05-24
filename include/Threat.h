#ifndef AIRDEFENSESIM_THREAT_H
#define AIRDEFENSESIM_THREAT_H

#include <string>
#include <iostream>
#include <cmath>

#include <GLFW/glfw3.h>
#include "Logger.h"

/**
 * @enum ThreatStatus
 * @brief Represents the lifecycle states of a ballistic threat.
 */
enum class ThreatStatus {
    ACTIVE,    ///< Threat is currently in flight
    DESTROYED, ///< Threat was successfully intercepted
    IMPACTED   ///< Threat hit the ground (interception failed)
};

/**
 * @class Threat
 * @brief Represents a ballistic threat moving in a 3D parabolic trajectory.
 * * This class is responsible for simulating the physical movement of a threat
 * under the influence of gravity and predicting its ground impact point for
 * the defense system's sector logic.
 */
class Threat {
private:
    std::string id;            ///< Unique identifier for the threat
    double x, y, z;            ///< Current 3D position
    double startX, startY, startZ; ///< Initial launch position
    double vx, vy, vz;         ///< Initial velocity components
    double impactX, impactY;   ///< Predicted landing coordinates
    ThreatStatus status;       ///< Current flight status
    bool targeted;             ///< Flag to indicate if an interceptor is locked on

    double time;               ///< Total elapsed flight time
    static constexpr double GRAVITY = 9.81; ///< Constant for gravitational acceleration

    /**
     * @brief Internal method to predict the impact point on the ground (Z=0)
     * based on kinematics and the quadratic formula.
     */
    void calculateImpactPosition();

public:

    /**
     * @brief Renders the threat as a visual entity in the simulation.
     */
    void draw() const;

    /**
     * @brief Constructor - Initializes the threat's trajectory and predicts its impact.
     */
    Threat(std::string threatId, double startX, double startY, double startZ, double vX, double vY, double vZ);

    // Getters for state and position
    std::string getId() const;
    double getX() const;
    double getY() const;
    double getZ() const;

    double getImpactX() const;
    double getImpactY() const;

    ThreatStatus getStatus() const;
    bool isTargeted() const;

    /**
     * @brief Updates the targeting flag to prevent duplicate interceptions.
     */
    void setTargeted(bool val);

    /**
     * @brief Changes status to DESTROYED upon a successful hit.
     */
    void markAsDestroyed();

    /**
     * @brief Advances the threat's position in the 3D space based on the time step.
     * @param dt The time elapsed since the last update.
     */
    void updatePosition(double dt);

    /**
     * @brief Returns the total time the threat has been active.
     */
    double getTime() const;

    /**
     * @brief Predicts the threat's 3D position at a specific future simulation time.
     * Used for lead-pursuit calculations if needed.
     */
    void getFuturePosition(double futureTime, double& outX, double& outY, double& outZ) const;

};

#endif //AIRDEFENSESIM_THREAT_H