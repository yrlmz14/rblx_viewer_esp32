#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <vector>
#include <string>

#include "pin_config.h"
#include "display_driver.h"
#include "touch_handler.h"
#include "renderer3d.h"
#include "r3d_parser.h"
#include "ui_hud.h"

DisplayDriver display;
TouchHandler touch;
Renderer3D renderer(SCREEN_WIDTH, SCREEN_HEIGHT);
UIHud hud(SCREEN_WIDTH, SCREEN_HEIGHT);

std::vector<R3DPart> currentParts;
uint16_t* frameBuffer = nullptr;
std::string loadedFileName = "No SD File";

float camRotX = 0.4f;
float camRotY = 0.6f;
float camZoom = 25.0f;

void loadModelFromSD() {
    File root = SD.open("/");
    if (!root) return;

    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            std::string filename = file.name();
            if (filename.find(".r3d") != std::string::npos || filename.find(".R3D") != std::string::npos) {
                std::string fullPath = "/" + filename;
                if (R3DParser::parseFromFile(fullPath.c_str(), currentParts) && !currentParts.empty()) {
                    loadedFileName = filename;
                    file.close();
                    root.close();
                    return;
                }
            }
        }
        file = root.openNextFile();
    }
    file.close();
    root.close();
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Roblox AMOLED Viewer...");

    display.init();
    touch.init();

    // Allocate frame buffer in PSRAM
    frameBuffer = (uint16_t*)ps_malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
    if (!frameBuffer) {
        frameBuffer = (uint16_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
    }

    // Initialize SD Card
    SPI.begin(SD_SPI_SCK, SD_SPI_MISO, SD_SPI_MOSI, SD_SPI_CS);
    if (!SD.begin(SD_SPI_CS)) {
        Serial.println("SD Card initialization failed!");
    } else {
        Serial.println("SD Card initialized successfully.");
        loadModelFromSD();
    }

    // Load fallback demo part if no model loaded from SD
    if (currentParts.empty()) {
        R3DPart demoPart;
        demoPart.shape = R3D_SHAPE_BLOCK;
        demoPart.posX = 0.0f; demoPart.posY = 0.0f; demoPart.posZ = 0.0f;
        demoPart.sizeX = 4.0f; demoPart.sizeY = 1.2f; demoPart.sizeZ = 2.0f;
        for(int i=0; i<9; ++i) demoPart.rot[i] = (i%4==0)?1.0f:0.0f;
        demoPart.colorRGB565 = 0x07E0; // Bright Green
        demoPart.transparency = 0.0f;

        currentParts.push_back(demoPart);
        loadedFileName = "Demo Block";
    }
}

void loop() {
    static uint32_t lastFrameTime = millis();
    uint32_t now = millis();
    float delta = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;
    float fps = (delta > 0.0001f) ? (1.0f / delta) : 60.0f;

    // Handle touch controls
    touch.updateCameraState(camRotX, camRotY, camZoom);

    // Clear frame buffer
    if (frameBuffer) {
        memset(frameBuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));

        renderer.setCamera(camRotX, camRotY, camZoom);
        renderer.renderParts(currentParts, frameBuffer);

        hud.drawOverlay(frameBuffer, fps, currentParts.size(), loadedFileName);

        // Push frame to AMOLED display
        display.setAddrWindow(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
        display.writePixels(frameBuffer, SCREEN_WIDTH * SCREEN_HEIGHT);
    }

    delay(10);
}
