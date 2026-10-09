#ifndef UI_HUD_H
#define UI_HUD_H

#include <stdint.h>
#include <vector>
#include <string>

class UIHud {
public:
    UIHud(int screenWidth, int screenHeight);

    void drawOverlay(uint16_t* fb, float fps, size_t partCount, const std::string& currentFileName);
    void drawFileBrowser(uint16_t* fb, const std::vector<std::string>& fileList, int selectedIndex);
};

#endif // UI_HUD_H
