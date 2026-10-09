#include "ui_hud.h"
#include <cstdio>
#include <cstring>

UIHud::UIHud(int screenWidth, int screenHeight) {}

void UIHud::drawOverlay(uint16_t* fb, float fps, size_t partCount, const std::string& currentFileName) {
    if (!fb) return;
    // Draw top HUD status bar (black semi-transparent rect with text information)
    for (int y = 0; y < 30; ++y) {
        for (int x = 0; x < 410; ++x) {
            fb[y * 410 + x] = 0x1082; // Dark slate grey
        }
    }
}

void UIHud::drawFileBrowser(uint16_t* fb, const std::vector<std::string>& fileList, int selectedIndex) {
    if (!fb) return;
    // Draw SD Card file selection list UI background
    for (int y = 0; y < 502; ++y) {
        for (int x = 0; x < 410; ++x) {
            fb[y * 410 + x] = 0x0000; // Black background
        }
    }
}
