#pragma once
#include <iostream>
#include <vector>
#include "Player.hpp"
#include "LocalPlayer.hpp"
#include "Offsets.hpp"
#include "Camera.hpp"
#include "GlowMode.hpp"

#include "DMALibrary/Memory/Memory.h"
#include <array>

struct Sense {
    // Variables
    Camera* GameCamera;
    LocalPlayer* Myself;
    std::vector<Player*>* Players;
    int TotalSpectators = 0;
    std::vector<std::string> Spectators;
    uint64_t HighlightSettingsPointer;

    bool ItemGlow = false;
    // 34 = White, 35 = Blue, 36 = Purple, 37 = Gold, 38 = Red
    int MinimumItemRarity = 36;
    float lastvis_aim[100];
    //Colors
    float InvisibleGlowColor[3] = { 0, 1, 0 };
    float VisibleGlowColor[3] = { 1, 0, 0 };

    Sense(std::vector<Player*>* Players, Camera* GameCamera, LocalPlayer* Myself) {
        this->Players = Players;
        this->GameCamera = GameCamera;
        this->Myself = Myself;
    }

    void Initialize() {
        // idk, nothing for now
    }

    void enableGlow(Player* Target, int setting_index, uint8_t inside_value, uint8_t outline_size, std::array<float, 3> highlight_parameter, float glow_dist) {
        const unsigned char outsidevalue = 125;
        uint64_t ptr = Target->BasePointer;
        uint64_t g_Base = mem.OFF_BASE;
        std::array<unsigned char, 4> highlightFunctionBits = {
            inside_value, // InsideFunction
            outsidevalue, // OutlineFunction: HIGHLIGHT_OUTLINE_OBJECTIVE
            outline_size, // OutlineRadius: size * 255 / 8
            64 // (EntityVisible << 6) | State & 0x3F | (AfterPostProcess << 7)
        };
        mem.Write<uint8_t>(ptr + OFF_GLOW_ENABLE, setting_index);
        mem.Write<int>(ptr + OFF_GLOW_THROUGH_WALL, 2);

        auto handle = mem.CreateScatterHandle();
        uint64_t highlightSettingsPtr = HighlightSettingsPointer;

        mem.Write<decltype(highlightFunctionBits)>(highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * setting_index + 0x0, highlightFunctionBits);
        mem.Write<decltype(highlight_parameter)>(highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * setting_index + 0x4, highlight_parameter);

        //mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * setting_index + 0x0, &highlightFunctionBits, sizeof(highlightFunctionBits));
        //mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * setting_index + 0x4, &highlight_parameter, sizeof(highlight_parameter));
        mem.Write<float>(ptr + 0x264, glow_dist);
        mem.Write<int>(g_Base + OFF_GLOW_FIX, 1);
    }

    void setCustomGlow(Player* Target, int glowable, int throughWall, bool isVisible)
    {
        if (Target->GlowEnable == 0 && Target->GlowThroughWall == 0 && Target->HighlightID == 0) {
			return;
		}
        uint64_t TargetPtr = Target->BasePointer;

        int settingIndex = 65;
        std::array<float, 3> glowColorRGB = { 0, 0, 0 };

        if (!isVisible) {
            settingIndex = 65;
            glowColorRGB = { InvisibleGlowColor[0], InvisibleGlowColor[1], InvisibleGlowColor[2] }; // Invisible Enemies
        }
        else if (isVisible) {
            settingIndex = 70;
            glowColorRGB = { VisibleGlowColor[0], VisibleGlowColor[1], VisibleGlowColor[2] }; // Visible Enemies
        }

        uint64_t glowEnableAddress = TargetPtr + OFF_GLOW_ENABLE;
        mem.Write<int>(glowEnableAddress, glowable);

        uint64_t glowThroughWallAddress = TargetPtr + OFF_GLOW_THROUGH_WALL;
        mem.Write<int>(glowThroughWallAddress, throughWall);

        uint64_t highlightIdAddress = TargetPtr + OFF_GLOW_HIGHLIGHT_ID;
        unsigned char value = settingIndex;
        mem.Write<unsigned char>(highlightIdAddress, value);

        uint64_t glowFixAddress = mem.OFF_BASE + OFF_GLOW_FIX;
        mem.Write<int>(glowFixAddress, 1);
    }

    void setHighlightSettings() {
        int InvisibleIndex = 65; // Invis
        int VisibleIndex = 70; // Vis
        std::array<unsigned char, 4> highlightFunctionBits = {
            2,   // InsideFunction							2
            64, // OutlineFunction: HIGHLIGHT_OUTLINE_OBJECTIVE			125
            64,  // OutlineRadius: size * 255 / 8				64
            64   // (EntityVisible << 6) | State & 0x3F | (AfterPostProcess << 7) 	64
        };
        std::array<float, 3> invisibleGlowColorRGB = { 0, 1, 0 };
        std::array<float, 3> visibleGlowColorRGB = { 1, 0, 0 };

        uint64_t highlightSettingsPtr = HighlightSettingsPointer;
        if (mem.IsValidPointer(highlightSettingsPtr)) {
            auto handle = mem.CreateScatterHandle();

            // Invisible
            mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * InvisibleIndex + 0x0, &highlightFunctionBits, sizeof(highlightFunctionBits));
            mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * InvisibleIndex + 0x4, &invisibleGlowColorRGB, sizeof(invisibleGlowColorRGB));

            // Visible
            mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * VisibleIndex + 0x0, &highlightFunctionBits, sizeof(highlightFunctionBits));
            mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * VisibleIndex + 0x4, &visibleGlowColorRGB, sizeof(visibleGlowColorRGB));

            mem.ExecuteWriteScatter(handle);
            mem.CloseScatterHandle(handle);
        }

    }


    void SetPlayerGlow(Player& Target, int index) {
        //int context_id = 0;
        int setting_index = 0;
        std::array<float, 3> highlight_parameter = { 0, 0, 0 };

        if (!Target.GlowEnable || (int)Target.GlowThroughWall != 1) {
            float currentEntityTime = 5000.f;
            if (!isnan(currentEntityTime) && currentEntityTime > 0.f) {
                // set glow color
                if (Target.IsKnocked || Target.IsDead) {  //倒地或者没活着
                    setting_index = 80;
                    highlight_parameter = { 0.80f, 0.78f, 0.45f };
                }
                else if (Target.LastVisibleTime > lastvis_aim[index] || (Target.LastVisibleTime < 0.f && lastvis_aim[index] > 0.f)) {
                    setting_index = 81;
                    highlight_parameter = {1,0,0};
                }
                else {
                        setting_index = 82;
                        highlight_parameter = {0,1,0};
                    }
                enableGlow(&Target,setting_index, 0,48, highlight_parameter, 250.f*40);
                }
            }
    }
    Vector2D DummyVector = { 0, 0 };
    void Update() {
        if (Myself->IsDead) return;

        if (ItemGlow) {
            uint64_t highlightSettingsPtr = HighlightSettingsPointer;
            if (mem.IsValidPointer(highlightSettingsPtr)) {
                uint64_t highlightSize = OFF_GLOW_HIGHLIGHT_TYPE_SIZE;
                const GlowMode newGlowMode = { 137,138,35,127 };
                for (int highlightId = MinimumItemRarity; highlightId < 39; highlightId++)
                {
                    const GlowMode oldGlowMode = mem.Read<GlowMode>(highlightSettingsPtr + (highlightSize * highlightId) + 0, true);
                    if (newGlowMode != oldGlowMode) {
                        mem.Write<GlowMode>(highlightSettingsPtr + (highlightSize * highlightId) + 0, newGlowMode);
                    }
                }
            }
        }

        for (int i = 0; i < Players->size(); i++) {
            Player* Target = Players->at(i);
            if (!Target->IsValid()) continue;
            if (Target->IsDummy()) continue;
            if (Target->IsLocal) continue;
            if (!Target->IsHostile) continue;
            if (GameCamera->WorldToScreen(Target->LocalOrigin.ModifyZ(30), DummyVector)) {
                setCustomGlow(Target, 7, 2, Target->IsVisible);
                std::cout << Target->IsVisible << "/n";
            }
        }
        
    }
};