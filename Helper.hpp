#pragma once
#include "DMALibrary/Memory/Memory.h"
#include "LocalPlayer.hpp"


struct ActionHelper { 
    LocalPlayer* Myself;
    
    ActionHelper(LocalPlayer* Myself) {
        this->Myself = Myself;
    }

    void superGrpple(LocalPlayer* Myself) {
        if (Myself->IsGrppleActived) {
            if (Myself->IsGrppleAttached == 1) {
                mem.Write<int>(mem.OFF_BASE + OFF_IN_JUMP + 0x08, 5);
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                mem.Write<int>(mem.OFF_BASE + OFF_IN_JUMP + 0x08, 4);
            }
        }

    }
    
    void Update() {
        if (Myself->IsDead) return;
        superGrpple(Myself);

        
    }
};