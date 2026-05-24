#include "../include/Threat.h"

/*
Input: std::string threatId, double sX/sY/sZ (Start Pos), double vX/vY/vZ (Velocity).
Output: None (Constructor).
Description: Initializes a ballistic threat with its starting position and velocity vectors.
             Immediately calls calculateImpactPosition to predict its landing coordinates.
 */
Threat::Threat(std::string threatId, double sX, double sY, double sZ, double vX, double vY, double vZ)
    : id(threatId), x(sX), y(sY), z(sZ), startX(sX), startY(sY), startZ(sZ),
      vx(vX), vy(vY), vz(vZ), status(ThreatStatus::ACTIVE), targeted(false), time(0.0) {

    calculateImpactPosition();
}

/*
Input: None.
Output: std::string (ID), double (X, Y, Z, ImpactX, ImpactY, Time), ThreatStatus (Status), bool (IsTargeted).
Description: Getter functions to retrieve the threat's current state, position, and predicted landing point.
 */
std::string Threat::getId() const { return id; }
double Threat::getX() const { return x; }
double Threat::getY() const { return y; }
double Threat::getZ() const { return z; }

double Threat::getImpactX() const { return impactX; }
double Threat::getImpactY() const { return impactY; }

ThreatStatus Threat::getStatus() const { return status; }
bool Threat::isTargeted() const { return targeted; }

double Threat::getTime() const { return time; }

/*
Input: bool val.
Output: None.
Description: Sets the targeted status of the threat to prevent multiple batteries from locking on.
 */
void Threat::setTargeted(bool val) { targeted = val; }

/*
Input: None.
Output: None.
Description: Updates the threat's status to DESTROYED, typically called when an interceptor hits it.
 */
void Threat::markAsDestroyed() {
    status = ThreatStatus::DESTROYED;
}

/*
Input: None.
Output: None.
Description: Uses the quadratic formula based on physics equations (Gravity) to solve for time-to-impact
             and calculate the final (X, Y) coordinates where the threat will hit the ground.
 */
void Threat::calculateImpactPosition() {
    double a = -0.5 * GRAVITY;
    double b = vz;
    double c = startZ;

    double discriminant = (b * b) - (4 * a * c);

    if (discriminant >= 0) {
        double t1 = (-b + std::sqrt(discriminant)) / (2 * a);
        double t2 = (-b - std::sqrt(discriminant)) / (2 * a);
        double timeToImpact = std::max(t1, t2);

        impactX = startX + (vx * timeToImpact);
        impactY = startY + (vy * timeToImpact);
    } else {
        impactX = startX;
        impactY = startY;
    }
}

/*
Input: double dt (Delta time).
Output: None.
Description: Updates the threat's 3D position based on its velocity and gravity over time.
             If the Z-coordinate reaches 0, the status is updated to IMPACTED.
 */
void Threat::updatePosition(double dt) {
    if (status == ThreatStatus::ACTIVE) {
        time += dt;

        x = startX + (vx * time);
        y = startY + (vy * time);
        z = startZ + (vz * time) - (0.5 * GRAVITY * time * time);

        if (z <= 0) {
            z = 0;
            status = ThreatStatus::IMPACTED;
        }
    }
}


/*
Input: double futureTime, double &outX, double &outY, double &outZ (outputs).
Output: None.
Description: Calculates and returns the predicted 3D position of the threat at a specific future time.
 */
void Threat::getFuturePosition(double futureTime, double& outX, double& outY, double& outZ) const {
    outX = startX + (vx * futureTime);
    outY = startY + (vy * futureTime);
    outZ = startZ + (vz * futureTime) - (0.5 * GRAVITY * futureTime * futureTime);
}

/*
Input: None.
Output: None.
Description: Renders the threat as a red polygon on the screen using OpenGL if it is currently ACTIVE.
 */
void Threat::draw() const {
    if (status == ThreatStatus::ACTIVE) {
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_POLYGON);
        for(int i = 0; i < 20; i++) {
            float theta = 2.0f * 3.14159f * float(i) / 20.0f;
            glVertex2f(x + 10.0f * cos(theta), y + 10.0f * sin(theta));
        }
        glEnd();
    }
}
