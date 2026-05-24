#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <unistd.h>
#include <GLFW/glfw3.h>

#include "Threat.h"
#include "Interceptor.h"
#include "DefenseSystem.h"
#include "Logger.h"

#include "stb_image.h"

struct BatteryType {
    std::string name;
    float range;
    float speed;
};

struct ThreatType {
    std::string name;
    double vz;
};

// רשימות מוכנות מראש
std::vector<BatteryType> batteryPresets = {
    {"Iron Dome", 80.0f, 150.0f},
    {"David Sling", 180.0f, 300.0f},
    {"Arrow 3", 350.0f, 600.0f}
};

std::vector<ThreatType> threatPresets = {
    {"Grad Rocket", 60.0},
    {"Cruise Missile", 90.0},
    {"Ballistic", 150.0}
};

int selectedBatteryIdx = 0;
int selectedThreatIdx = 0;

DefenseSystem* globalSystem = nullptr;
bool isPlacingBattery = true;

bool isDraggingThreat = false;
float dragStartX = 0.0f;
float dragStartY = 0.0f;

/*
Input: std::string id, double startX/startY, double targetX/targetY, double vz.
Output: Threat* (pointer to a new Threat object).
Description: Helper function that calculates the required horizontal velocity (vx, vy)
             based on a target destination and vertical speed, then creates a targeted Threat.
 */
Threat* createTargetedThreat(std::string id, double startX, double startY, double targetX, double targetY, double vz) {
    double timeToImpact = (2.0 * vz) / 9.81;
    double vx = (targetX - startX) / timeToImpact;
    double vy = (targetY - startY) / timeToImpact;
    return new Threat(id, startX, startY, 0.0, vx, vy, vz);
}

/*
Input: GLFWwindow* window, int button, int action, int mods.
Output: None (Callback).
Description: Handles mouse clicks. It detects if a click was on a menu button (to select a unit)
             or on the map (to deploy a battery or launch an enemy threat).
 */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    float nx = (2.0f * (float)xpos / (float)width) - 1.0f;
    float ny = 1.0f - (2.0f * (float)ypos / (float)height) - 0.07f;

    float worldX = nx * 1778.0f;
    float worldY = ny * 1000.0f;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (worldX >= -1750.0f && worldX <= -1450.0f) {
            float startY_Batt = -400.0f;
            for (int i = 0; i < (int)batteryPresets.size(); i++) {
                float bMinY = startY_Batt - (i * 110.0f);
                float bMaxY = bMinY + 80.0f;
                if (worldY >= bMinY && worldY <= bMaxY) {
                    isPlacingBattery = true;
                    selectedBatteryIdx = i;
                    std::cout << "[UI] Selected: " << batteryPresets[i].name << std::endl;
                    return;
                }
            }

            float startY_Threat = -800.0f;
            for (int i = 0; i < (int)threatPresets.size(); i++) {
                float tMinY = startY_Threat - (i * 110.0f);
                float tMaxY = tMinY + 80.0f;
                if (worldY >= tMinY && worldY <= tMaxY) {
                    isPlacingBattery = false;
                    selectedThreatIdx = i;
                    std::cout << "[UI] Selected Threat: " << threatPresets[i].name << std::endl;
                    return;
                }
            }
        }

        globalSystem->simMutex.lock();
        if (isPlacingBattery) {
            auto& p = batteryPresets[selectedBatteryIdx];
            float fixedSectorRadius = 35.0f;
            globalSystem->addInterceptor(new Interceptor(
                p.name, worldX, worldY, 0, p.speed,
                worldX - fixedSectorRadius, worldX + fixedSectorRadius,
                worldY - fixedSectorRadius, worldY + fixedSectorRadius, p.range
            ));
        }
        globalSystem->simMutex.unlock();
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {

        if (action == GLFW_PRESS) {
            dragStartX = worldX;
            dragStartY = worldY;
            isDraggingThreat = true;
            std::cout << "[RADAR] Launch position set at: (" << dragStartX << ", " << dragStartY << ")" << std::endl;
        }

        else if (action == GLFW_RELEASE && isDraggingThreat) {
            isDraggingThreat = false;

            globalSystem->simMutex.lock();
            auto& t = threatPresets[selectedThreatIdx];

            Threat* newT = createTargetedThreat(t.name, dragStartX, dragStartY, worldX, worldY, t.vz);
            globalSystem->addThreat(newT);

            globalSystem->simMutex.unlock();
            std::cout << "[RADAR] Threat launched! Route: (" << dragStartX << ", " << dragStartY
                      << ") -> Target: (" << worldX << ", " << worldY << ")" << std::endl;
        }
    }
}

/*
Input: GLFWwindow* window, int key, int scancode, int action, int mods.
Output: None (Callback).
Description: Handles keyboard input. Currently used to listen for the 'R' key to reset the simulation state.
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        globalSystem->simMutex.lock();
        globalSystem->reset();
        globalSystem->simMutex.unlock();
        std::cout << "System Reset" << std::endl;
    }
}

/*
Input: None.
Output: None.
Description: Renders the tactical grid overlay for better spatial awareness on the map.
 */
void drawTacticalGrid() {
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 0.05f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for(float x = -2000; x <= 2000; x += 100) { glVertex2f(x, -1000); glVertex2f(x, 1000); }
    for(float y = -1000; y <= 1000; y += 100) { glVertex2f(-2000, y); glVertex2f(2000, y); }
    glEnd();
    glDisable(GL_BLEND);
}

/*
Input: None.
Output: None.
Description: Renders the HUD area at the top of the screen to separate simulation from UI.
 */
void drawHUDBackground() {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-2000, 850);
    glVertex2f(2000, 850);
    glVertex2f(2000, 1000);
    glVertex2f(-2000, 1000);
    glEnd();
}
/*
Input: None.
Output: None.
Description: Draws the interactive unit selection menu, highlighting the currently selected asset.
 */
void drawMenu() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_TEXTURE_2D);

    float startX = -1700.0f;
    float startY = -400.0f;
    float btnW = 250.0f;
    float btnH = 80.0f;
    float spacing = 110.0f;

    for (int i = 0; i < (int)batteryPresets.size(); i++) {
        bool selected = (i == selectedBatteryIdx && isPlacingBattery);

        if (selected) glColor3f(0.0f, 1.0f, 0.4f);
        else glColor3f(0.1f, 0.2f, 0.3f);

        glBegin(GL_QUADS);
            glVertex2f(startX, startY - (i * spacing));
            glVertex2f(startX + btnW, startY - (i * spacing));
            glVertex2f(startX + btnW, startY - (i * spacing) + btnH);
            glVertex2f(startX, startY - (i * spacing) + btnH);
        glEnd();

        glColor3f(0.8f, 0.9f, 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(startX, startY - (i * spacing));
            glVertex2f(startX + btnW, startY - (i * spacing));
            glVertex2f(startX + btnW, startY - (i * spacing) + btnH);
            glVertex2f(startX, startY - (i * spacing) + btnH);
        glEnd();
    }

    float threatStartY = startY - 400.0f;
    for (int i = 0; i < (int)threatPresets.size(); i++) {
        bool selected = (i == selectedThreatIdx && !isPlacingBattery);

        if (selected) glColor3f(1.0f, 0.2f, 0.2f);
        else glColor3f(0.3f, 0.1f, 0.1f);

        glBegin(GL_QUADS);
            glVertex2f(startX, threatStartY - (i * spacing));
            glVertex2f(startX + btnW, threatStartY - (i * spacing));
            glVertex2f(startX + btnW, threatStartY - (i * spacing) + btnH);
            glVertex2f(startX, threatStartY - (i * spacing) + btnH);
        glEnd();

        // 2. מסגרת בולטת
        glColor3f(1.0f, 0.7f, 0.7f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(startX, threatStartY - (i * spacing));
            glVertex2f(startX + btnW, threatStartY - (i * spacing));
            glVertex2f(startX + btnW, threatStartY - (i * spacing) + btnH);
            glVertex2f(startX, threatStartY - (i * spacing) + btnH);
        glEnd();
    }
}
/*
Input: const char* filename.
Output: GLuint (Texture ID).
Description: Loads an image from the disk and converts it into an OpenGL texture.
 */
GLuint loadTexture(const char* filename) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    } else {
        std::cout << "Failed to load texture: " << filename << std::endl;
    }
    return textureID;
}

/*
Input: GLuint textureID.
Output: None.
Description: Renders the world map as a background texture using global world coordinates.
 */
void drawWorldMap(GLuint textureID) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1778.0f, -1000.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1778.0f, -1000.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1778.0f, 1000.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1778.0f, 1000.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

/*
Input: None.
Output: int (Exit code).
Description: Main entry point. Initializes the logger, defense system, and OpenGL context.
             Contains the main simulation loop that updates logic and renders each frame.
 */
int main() {
    Logger myLogger("simulation_results.txt");
    DefenseSystem ironDome(&myLogger);
    globalSystem = &ironDome;

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Air Defense Command & Control", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    GLuint mapTexture = loadTexture("world_map.png");

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1778.0, 1778.0, -1000.0, 1000.0, -1.0, 1.0);

    double TIME_MULTIPLIER = 5.0;
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> realDeltaTime = currentTime - lastTime;
        lastTime = currentTime;
        double dt = realDeltaTime.count() * TIME_MULTIPLIER;

        ironDome.simMutex.lock();
        ironDome.runSectorAlgorithm();
        ironDome.updateWorld(dt);

        glClearColor(0.15f, 0.18f, 0.22f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        drawWorldMap(mapTexture);
        drawTacticalGrid();

        if (isDraggingThreat) {
            double currentMouseX, currentMouseY;
            glfwGetCursorPos(window, &currentMouseX, &currentMouseY);

            int width, height;
            glfwGetWindowSize(window, &width, &height);

            float nx = (2.0f * (float)currentMouseX / (float)width) - 1.0f;
            float ny = 1.0f - (2.0f * (float)currentMouseY / (float)height) - 0.07f;
            float mouseWorldX = nx * 1778.0f;
            float mouseWorldY = ny * 1000.0f;

            glLineWidth(2.0f);
            glColor3f(1.0f, 0.0f, 0.0f);
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x00FF); // תבנית קו מקווקו

            glBegin(GL_LINES);
            glVertex2f(dragStartX, dragStartY);
            glVertex2f(mouseWorldX, mouseWorldY);
            glEnd();

            glDisable(GL_LINE_STIPPLE);
        }

        ironDome.draw();
        drawHUDBackground();
        drawMenu();
        ironDome.draw();
        drawHUDBackground();
        drawMenu();

        ironDome.simMutex.unlock();

        glfwSwapBuffers(window);
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}