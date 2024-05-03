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

    void setCustomGlow(Player* Target, int glowable, int throughWall, bool isVisible)
    {
        /*if (Target->GlowEnable == 0 && Target->GlowThroughWall == 0 && Target->HighlightID == 0) {
			return;
		}*/
        auto handle = mem.CreateScatterHandle();
        uint64_t TargetPtr = Target->BasePointer;
        std::array<unsigned char, 4> highlightFunctionBits = {
            2,   // InsideFunction							2
            125, // OutlineFunction: HIGHLIGHT_OUTLINE_OBJECTIVE			125
            64,  // OutlineRadius: size * 255 / 8				64
            64   // (EntityVisible << 6) | State & 0x3F | (AfterPostProcess << 7) 	64
        };
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

        //uint64_t glowEnableAddress = TargetPtr + OFF_GLOW_ENABLE;
        //mem.Write<int>(glowEnableAddress, 7);

        uint64_t glowThroughWallAddress = TargetPtr + OFF_GLOW_THROUGH_WALL;
        mem.Write<int>(glowThroughWallAddress, throughWall);

        uint64_t highlightIdAddress = TargetPtr + OFF_GLOW_ENABLE;
        unsigned char value = settingIndex;
        mem.Write<unsigned char>(highlightIdAddress, value);

        uint64_t HighlightSettingsPointerAddress = mem.OFF_BASE + OFF_GLOW_HIGHLIGHTS;
        mem.AddScatterWriteRequest(handle, HighlightSettingsPointerAddress, &HighlightSettingsPointer, sizeof(uint64_t));

        uint64_t HighlightFunctionBitsAddress = HighlightSettingsPointer + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * settingIndex;
        mem.AddScatterWriteRequest(handle, HighlightFunctionBitsAddress, &highlightFunctionBits, sizeof(highlightFunctionBits));

        uint64_t HighlightparameterAddress = HighlightSettingsPointer + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * settingIndex + 0x4;
        mem.AddScatterWriteRequest(handle, HighlightparameterAddress, &glowColorRGB, sizeof(glowColorRGB));

        mem.Write<float>(TargetPtr + 0x264, 200*40.f);
        mem.ExecuteWriteScatter(handle);
        mem.CloseScatterHandle(handle);
        uint64_t glowFixAddress = mem.OFF_BASE + OFF_GLOW_FIX;
        mem.Write<int>(glowFixAddress, 1);
    }

    Vector2D DummyVector = { 0, 0 };
    void Update() {
        if (Myself->IsDead) return;

        if (ItemGlow) {
            uint64_t highlightSettingsPtr = HighlightSettingsPointer;
            if (mem.IsValidPointer(highlightSettingsPtr)) {
                uint64_t highlightSize = OFF_GLOW_HIGHLIGHT_TYPE_SIZE;
                const GlowMode newGlowMode = { 137,0,0,127 };
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
            std::cout << Target->IsVisible << "555/n";
            if (!Target->IsValid()) continue;
            if (Target->IsDummy()) continue;
            if (Target->IsLocal) continue;
            if (!Target->IsHostile) continue;
            if (GameCamera->WorldToScreen(Target->LocalOrigin.ModifyZ(30), DummyVector)) {
                setCustomGlow(Target, 1, 2, Target->IsVisible);
            }
        }
        
    }
};