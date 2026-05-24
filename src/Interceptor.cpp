#include "../include/Interceptor.h"

/*
Input: std::string interceptorId, double startX/startY/startZ, double startSpeed,
       double minX/maxX/minY/maxY (Sector boundaries), float maxRange.
Output: None (Constructor).
Description: Initializes an Interceptor battery with its location, speed,
             assigned geographic sector responsibility, and maximum interception range.
 */
Interceptor::Interceptor(std::string interceptorId, double startX, double startY, double startZ, double startSpeed,
                         double minX, double maxX, double minY, double maxY, float maxRange)
    : id(interceptorId), baseX(startX), baseY(startY), x(startX), y(startY), z(startZ),
      speed(startSpeed), minSectorX(minX), maxSectorX(maxX), minSectorY(minY), maxSectorY(maxY),
      maxInterceptRange(maxRange), status(InterceptorStatus::STANDBY), target(nullptr) {
}


/*
Input: None.
Output: std::string (ID), double (X, Y, Z, Speed), InterceptorStatus (Status).
Description: Getter functions to retrieve the current state and attributes of the interceptor.
 */
std::string Interceptor::getId() const { return id; }
double Interceptor::getX() const { return x; }
double Interceptor::getY() const { return y; }
double Interceptor::getZ() const { return z; }
InterceptorStatus Interceptor::getStatus() const { return status; }
std::string Interceptor::getTargetId() const {
    if (target != nullptr) {
        return target->getId();
    }
    return "Unknown";
}
double Interceptor::getSpeed() const { return speed; }

/*
Input: double impactX, double impactY.
Output: bool.
Description: Checks if a predicted impact point falls within the battery's
             pre-defined rectangular geographic sector.
 */
bool Interceptor::isImpactInSector(double impactX, double impactY) const {
    return (impactX >= minSectorX && impactX <= maxSectorX &&
            impactY >= minSectorY && impactY <= maxSectorY);
}

/*
Input: Threat* newTarget.
Output: None.
Description: Pairs the battery with a threat and changes its status to IN_FLIGHT.
             Resets the interceptor's position to its base coordinates for launch.
 */
void Interceptor::assignTarget(Threat* newTarget) {
    if (status == InterceptorStatus::IN_FLIGHT) return;

    target = newTarget;
    status = InterceptorStatus::IN_FLIGHT;

    x = baseX;
    y = baseY;
    z = 0;
}

/*
Input: double dt (Delta time).
Output: None.
Description: Updates the 3D position of the interceptor in flight,
             moving it towards its target's current position based on its speed.
 */
void Interceptor::updatePosition(double dt) {
    if (status == InterceptorStatus::IN_FLIGHT && target != nullptr && target->getStatus() == ThreatStatus::ACTIVE) {
        double dx = target->getX() - x;
        double dy = target->getY() - y;
        double dz = target->getZ() - z;
        double distance = std::sqrt(dx*dx + dy*dy + dz*dz);

        if (distance > 0) {
            double step = speed * dt;
            if (step > distance) step = distance;

            x += (dx / distance) * step;
            y += (dy / distance) * step;
            z += (dz / distance) * step;
        }
    }
}

/*
Input: None.
Output: bool.
Description: Checks if the interceptor is close enough to its target to confirm a hit (within 15 units).
             If a hit occurs, it marks the target as destroyed and resets the battery to standby.
 */
bool Interceptor::checkCollision() {
    if (!target) return false;

    double dx = x - target->getX();
    double dy = y - target->getY();
    double dz = z - target->getZ();
    double distance = std::sqrt(dx*dx + dy*dy + dz*dz);

    if (distance < 15.0) {
        target->markAsDestroyed();
        status = InterceptorStatus::HIT_TARGET;

        x = baseX;
        y = baseY;
        z = 0;
        status = InterceptorStatus::STANDBY;

        return true;
    }
    return false;
}

/*
Input: None.
Output: None.
Description: Renders the battery, its rectangular sector, and its circular interception
             range using OpenGL. Also draws the interceptor and its trajectory if in flight.
 */
void Interceptor::draw() const {
    glLineWidth(1.0f);
    glColor3f(0.0f, 0.8f, 0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(minSectorX, minSectorY);
    glVertex2f(maxSectorX, minSectorY);
    glVertex2f(maxSectorX, maxSectorY);
    glVertex2f(minSectorX, maxSectorY);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 1.0f, 0.0f, 0.4f);
    glLineWidth(1.5f); // קו מעט עבה יותר לעיגול

    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < 64; i++) {
        float theta = 2.0f * 3.14159f * float(i) / 64.0f;
        glVertex2f(baseX + maxInterceptRange * cos(theta), baseY + maxInterceptRange * sin(theta));
    }
    glEnd();
    glDisable(GL_BLEND);

    if (status == InterceptorStatus::IN_FLIGHT) {
        glColor3f(1.0f, 1.0f, 0.0f);
        glPointSize(6.0f);
        glBegin(GL_POINTS);
        glVertex2f(x, y);
        glEnd();

        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glVertex2f(x, y);
        glVertex2f(baseX, baseY);
        glEnd();
    }

    glColor3f(0.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(baseX - 5, baseY - 5);
    glVertex2f(baseX + 5, baseY - 5);
    glVertex2f(baseX + 5, baseY + 5);
    glVertex2f(baseX - 5, baseY + 5);
    glEnd();
}

/*
Input: None.
Output: float.
Description: Returns the maximum radius within which the battery can successfully engage a target.
 */
float Interceptor::getMaxInterceptRange() const {
    return maxInterceptRange;
}