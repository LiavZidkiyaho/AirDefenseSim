#ifndef AIRDEFENSESIM_INTERCEPTOR_H
#define AIRDEFENSESIM_INTERCEPTOR_H

#include <string>
#include <iostream>
#include <cmath>

#include <GLFW/glfw3.h>
#include "Threat.h"

/**
 * @enum InterceptorStatus
 * @brief Represents the operational states of an interceptor battery/missile.
 */
enum class InterceptorStatus {
    STANDBY,    ///< Ready for assignment
    IN_FLIGHT,  ///< Missile is active and chasing a target
    HIT_TARGET, ///< Target destroyed successfully
    MISSED      ///< Interception failed
};

/**
 * @class Interceptor
 * @brief Represents a defensive battery and its launched interceptors.
 * * This class handles the logic for geographic sector validation,
 * target tracking (homing missile logic), and rendering its coverage area.
 */
class Interceptor {
private:
    double baseX, baseY; ///< Static origin of the battery

    std::string id;      ///< Unique identifier
    double x, y, z;      ///< Current 3D position
    double speed;        ///< Flight speed of the interceptor
    InterceptorStatus status; ///< Current operational state
    Threat* target;      ///< Pointer to the assigned threat

    // Geographic Sector boundaries
    double minSectorX, maxSectorX;
    double minSectorY, maxSectorY;

    float maxInterceptRange; ///< Radius of effective fire

public:

    /**
     * @brief Renders the battery, its sector box, range circle, and active missiles.
     */
    void draw() const;

    /**
     * @brief Constructor - Initializes battery location, stats, and its assigned sector.
     */
    Interceptor(std::string interceptorId, double startX, double startY, double startZ, double startSpeed,
                double minX, double maxX, double minY, double maxY, float maxRange);

    // Getters for state and position
    std::string getId() const;
    double getX() const;
    double getY() const;
    double getZ() const;
    InterceptorStatus getStatus() const;
    std::string getTargetId() const;
    double getSpeed() const;

    /**
     * @brief Checks if a set of coordinates falls within this battery's sector.
     * @param impactX Predicted landing X coordinate.
     * @param impactY Predicted landing Y coordinate.
     * @return true if the coordinates are within sector bounds.
     */
    bool isImpactInSector(double impactX, double impactY) const;

    /**
     * @brief Locks onto a threat and initiates the launch sequence.
     */
    void assignTarget(Threat* newTarget);

    /**
     * @brief Moves the interceptor towards its target based on time delta.
     */
    void updatePosition(double dt);

    /**
     * @brief Performs a distance check to determine if the target was hit.
     * @return true if a collision occurred.
     */
    bool checkCollision();

    /**
     * @brief Retrieves the maximum fire radius of the battery.
     */
    float getMaxInterceptRange() const;

};

#endif //AIRDEFENSESIM_INTERCEPTOR_H