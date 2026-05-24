#include "../include/DefenseSystem.h"

/*
Input: Logger* logger - pointer to the system's logging utility.
Output: None (Constructor).
Description: Initializes the defense system, sets the logger, and logs the system initialization.
 */
DefenseSystem::DefenseSystem(Logger* logger) : sysLogger(logger) {
    sysLogger->logEvent("Defense System Initialized. Sector Algorithm Active.");
}

/*
Input: None (Destructor).
Output: None.
Description: Cleans up all dynamic memory by deleting threat and interceptor objects and clearing the vectors.
 */
DefenseSystem::~DefenseSystem() {
    for (auto* t : threats) {
        delete t;
    }
    for (auto* i : inventory) {
        delete i;
    }
    threats.clear();
    inventory.clear();
    sysLogger->logEvent("System memory cleaned up successfully.");
}

/*
Input: Threat* t - pointer to a new threat object.
Output: None.
Description: Adds a new threat to the system's tracking vector.
 */
void DefenseSystem::addThreat(Threat* t) { threats.push_back(t); }

/*
Input: Interceptor* i - pointer to a new interceptor battery object.
Output: None.
Description: Adds a new interceptor battery to the system's inventory.
 */
void DefenseSystem::addInterceptor(Interceptor* i) { inventory.push_back(i); }

/*
Input: None.
Output: None.
Description: The core logic of the system. It iterates over active threats, calculates their predicted impact,
             and matches them with an available battery based on sector responsibility and physical range.
 */
void DefenseSystem::runSectorAlgorithm() {
    for (auto& threat : threats) {
        if (threat->isTargeted() || threat->getStatus() != ThreatStatus::ACTIVE) {
            continue;
        }

        double impactX = threat->getImpactX();
        double impactY = threat->getImpactY();

        double currentThreatX = threat->getX();
        double currentThreatY = threat->getY();

        for (auto& battery : inventory) {
            if (battery->getStatus() != InterceptorStatus::STANDBY) continue;

            bool willLandInSector = battery->isImpactInSector(impactX, impactY);

            double dx = currentThreatX - battery->getX();
            double dy = currentThreatY - battery->getY();
            double distanceToThreat = std::sqrt(dx * dx + dy * dy);

            bool isInsideFireRange = (distanceToThreat <= battery->getMaxInterceptRange());

            if (willLandInSector && isInsideFireRange) {
                battery->assignTarget(threat);
                threat->setTargeted(true);

                std::cout << "[RADAR] Target locked and within range. Launching: "
                          << battery->getId() << std::endl;
                break;
            }
        }
    }
}


/*
Input: double dt - delta time (time passed since the last frame).
Output: None.
Description: Updates the entire simulation state. It handles scheduled launches, updates positions of
             active threats and interceptors, and checks for collisions or ground impacts.
 */
void DefenseSystem::updateWorld(double dt) {
    currentSimTime += dt;

    while (!pendingLaunches.empty() && pendingLaunches.top().launchTime <= currentSimTime) {
        PendingLaunch launch = pendingLaunches.top();
        pendingLaunches.pop();

        launch.interceptor->assignTarget(launch.target);
        sysLogger->logEvent(">>> LAUNCH: " + launch.interceptor->getId() +
                            " fired towards " + launch.target->getId() + " <<<");
    }

    for (auto* t : threats) {
        if (t->getStatus() == ThreatStatus::ACTIVE) {
            t->updatePosition(dt);
            if (t->getStatus() == ThreatStatus::IMPACTED) {
                sysLogger->logEvent(">>> CRITICAL ALERT: " + t->getId() + " has IMPACTED the ground! <<<");
            }
        }
    }

    for (auto* i : inventory) {
        if (i->getStatus() == InterceptorStatus::IN_FLIGHT) {
            i->updatePosition(dt);
            if (i->checkCollision()) {
                sysLogger->logEvent("*** HIT CONFIRMED! " + i->getId() + " destroyed target! ***");
            }
        }
    }
}

/*
Input: None.
Output: None.
Description: Completely resets the simulation state by clearing all objects, pending actions,
             and resetting the simulation clock.
 */
void DefenseSystem::reset() {
    for (auto* t : threats) delete t;
    for (auto* i : inventory) delete i;

    threats.clear();
    inventory.clear();

    while (!pendingLaunches.empty()) {
        pendingLaunches.pop();
    }

    currentSimTime = 0.0;

    sysLogger->logEvent("System state completely reset by user.");
}

/*
Input: None.
Output: None.
Description: Iterates through all threats and interceptors and triggers their respective draw functions
             to render them on the screen.
 */
void DefenseSystem::draw() const {
    for (auto* t : threats) t->draw();
    for (auto* i : inventory) i->draw();
}