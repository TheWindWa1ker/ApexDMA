#pragma once
#include <iostream>
#include "Offsets.hpp"
#include "DMALibrary/Memory/Memory.h"
#include "Vector2D.hpp"
#include "Vector3D.hpp"
/*
本地玩家指针
是否死亡
是否在射击
是否倒地
是否在瞄准
抓钩
队伍id
三维坐标向量
视角相机向量
二维视角向量

偏航角度
武器id
...
*/
struct LocalPlayer {
    uint64_t BasePointer;

    bool IsDead; // true--死了
    bool IsInAttack; //这个为什么放到base里读取
    bool IsKnocked; // true--倒地
    bool IsZooming; //true--在瞄准
    bool IsGrppleActived; //true--出勾
    bool IsGrppleAttached; //true--勾到

    int Team;
    Vector3D LocalOrigin;
    Vector3D CameraPosition;

    Vector2D ViewAngles;
    Vector2D PunchAngles;
    float ViewYaw;

    int WeaponIndex; //手持武器id
    float WeaponProjectileSpeed; //子弹射速
    float WeaponProjectileScale; //重力参数
    bool IsHoldingGrenade; //true--捏雷
    bool IsReloading; //true--换弹
    int Ammo; //当前武器使用的子弹

    float ZoomFOV; //开镜fov
    float TargetZoomFOV; //目标开镜fov??

    float wallrunStart;
    float wallrunClear;
    float TimeBase;
    int spaceFlag;              //空中状态64 蹲下67 站立65
    int skyDriveState;
    int backWardState;
    int duckState;              //向下蹲1 完全蹲下2 起身过程3 其他0
    int forewardState;          //按w时33，其他0 滚轮前进不触发
    int forceForeward;          //按下w是1 其他0
    float TraversalTimeStart;   //未使用 sg可用
    float TraversalTimeProgress; //未使用

    void ResetPointer() {
        BasePointer = 0;
    }

    bool ValidPosition() {
		if (LocalOrigin.x != 0.0f && LocalOrigin.y != 0.0f && LocalOrigin.z != 0.0f) {
            return true;
		} else {
			return false;
		}
	}
    //本地玩家实体信息在结构体read方法中使用分散读取统一读取
    void Read() {
        if (!mem.IsValidPointer(BasePointer)) return;

        auto handle = mem.CreateScatterHandle();

        // Scatter read request for IsDead
        uint64_t isDeadAddress = BasePointer + OFF_LIFE_STATE;
        mem.AddScatterReadRequest(handle, isDeadAddress, &IsDead, sizeof(bool));

        // Scatter read request for IsKnocked
        uint64_t isKnockedAddress = BasePointer + OFF_BLEEDOUT_STATE;
        mem.AddScatterReadRequest(handle, isKnockedAddress, &IsKnocked, sizeof(bool));

        // Scatter read request for IsZooming
        uint64_t isZoomingAddress = BasePointer + OFF_ZOOMING;
        mem.AddScatterReadRequest(handle, isZoomingAddress, &IsZooming, sizeof(bool));

        // Scatter read request for TimeBase
        uint64_t baseTimeAddress = BasePointer + OFF_TIME_BASE;
        mem.AddScatterReadRequest(handle, baseTimeAddress, &TimeBase, sizeof(float));

        // Scatter read request for spaceflag
        uint64_t spaceFlagAddress = BasePointer + OFF_FLAGS;
        mem.AddScatterReadRequest(handle, spaceFlagAddress, &spaceFlag, sizeof(int));

        // Scatter read request for Team
        uint64_t teamAddress = BasePointer + OFF_TEAM_NUMBER;
		mem.AddScatterReadRequest(handle, teamAddress, &Team, sizeof(int));

        // Scatter read request for LocalOrigin
        uint64_t localOriginAddress = BasePointer + OFF_LOCAL_ORIGIN;
		mem.AddScatterReadRequest(handle, localOriginAddress, &LocalOrigin, sizeof(Vector3D));

        // Scatter read request for CameraPosition
        uint64_t cameraPositionAddress = BasePointer + OFF_CAMERAORIGIN;
		mem.AddScatterReadRequest(handle, cameraPositionAddress, &CameraPosition, sizeof(Vector3D));

        // Scatter read request for ViewAngles
        uint64_t viewAnglesAddress = BasePointer + OFF_VIEW_ANGLES;
		mem.AddScatterReadRequest(handle, viewAnglesAddress, &ViewAngles, sizeof(Vector2D));

        // Scatter read request for ViewYaw
        uint64_t viewYawAddress = BasePointer + OFF_YAW;
        mem.AddScatterReadRequest(handle, viewYawAddress, &ViewYaw, sizeof(float));

        // Scatter read request for WeaponHandle  最新的主要武器和最新的非副手武器的区别？
        uint64_t WeaponHandle;
        uint64_t weaponHandleAddress = BasePointer + OFF_WEAPON_HANDLE;
		mem.AddScatterReadRequest(handle, weaponHandleAddress, &WeaponHandle, sizeof(uint64_t));

        // Scatter read request for OffhandWeaponID
        int OffHandWeaponID;
        uint64_t offhandWeaponIDAddress = BasePointer + OFF_OFFHAND_WEAPON;
		mem.AddScatterReadRequest(handle, offhandWeaponIDAddress, &OffHandWeaponID, sizeof(int));

        // Execute the scatter read
        mem.ExecuteReadScatter(handle);

        // Close the scatter handle
        mem.CloseScatterHandle(handle);

        if (!IsDead && !IsKnocked) {
            auto handle = mem.CreateScatterHandle();

            uint64_t wallrunstartAddress = BasePointer + OFF_WALLRUNSTART;
            mem.AddScatterReadRequest(handle, wallrunstartAddress, &wallrunStart, sizeof(float));

            uint64_t wallrunclearAddress = BasePointer + OFF_WALLRUNCLEAR;
            mem.AddScatterReadRequest(handle, wallrunclearAddress, &wallrunClear, sizeof(float));

            uint64_t skydrivestateAddress = BasePointer + OFF_SKYDRIVESTATE;
            mem.AddScatterReadRequest(handle, skydrivestateAddress, &skyDriveState, sizeof(int));

            uint64_t inbackwardAddress = mem.OFF_BASE + OFF_IN_BACKWARD;
            mem.AddScatterReadRequest(handle, inbackwardAddress, &backWardState, sizeof(int));

            uint64_t induckstateAddress = BasePointer + OFF_IN_DUCKSTATE;
            mem.AddScatterReadRequest(handle, induckstateAddress, &duckState, sizeof(int));

            uint64_t inforwardAddress = mem.OFF_BASE + OFF_IN_FORWARD;
            mem.AddScatterReadRequest(handle, inforwardAddress, &forewardState, sizeof(int));

            uint64_t inforceforwardAddress = mem.OFF_BASE + OFF_IN_FORWARD + 0x8;
            mem.AddScatterReadRequest(handle, inforceforwardAddress, &forceForeward, sizeof(int));
            //抓钩
            uint64_t grppleActivedAddress = BasePointer + OFF_GRAPPLE_ACTIVE;
            mem.AddScatterReadRequest(handle, grppleActivedAddress, &IsGrppleActived, sizeof(bool));

            uint64_t grppleStateAddress = BasePointer + OFF_GRAPPLE + OFF_GRAPPLE_ATTACHED;
            mem.AddScatterReadRequest(handle, grppleStateAddress, &IsGrppleAttached, sizeof(bool));
            // Execute the scatter read
            mem.ExecuteReadScatter(handle);
            // Close the scatter handle
            mem.CloseScatterHandle(handle);
        }
        if (!IsDead && !IsKnocked && WeaponHandle) {
            uint64_t WeaponHandleMasked = WeaponHandle & 0xffff;
            uint64_t WeaponEntity = mem.Read<uint64_t>(mem.OFF_BASE + OFF_ENTITY_LIST + (WeaponHandleMasked << 5), true);

            IsHoldingGrenade = OffHandWeaponID == -251 ? true : false;

            auto handle = mem.CreateScatterHandle();

            // Scatter read request for ZoomFOV
            uint64_t zoomFOVAddress = WeaponEntity + OFF_CURRENTZOOMFOV;
            mem.AddScatterReadRequest(handle, zoomFOVAddress, &ZoomFOV, sizeof(float));

            // Scatter read request for TargetZoomFOV
            uint64_t targetZoomFOVAddress = WeaponEntity + OFF_TARGETZOOMFOV;
            mem.AddScatterReadRequest(handle, targetZoomFOVAddress, &TargetZoomFOV, sizeof(float));

            // Scatter read request for WeaponIndex
            uint64_t weaponIndexAddress = WeaponEntity + OFF_WEAPON_INDEX;
            mem.AddScatterReadRequest(handle, weaponIndexAddress, &WeaponIndex, sizeof(int));

            // Scatter read request for WeaponProjectileSpeed
            uint64_t weaponProjectileSpeedAddress = WeaponEntity + OFF_PROJECTILESPEED;
            mem.AddScatterReadRequest(handle, weaponProjectileSpeedAddress, &WeaponProjectileSpeed, sizeof(float));

            // Scatter read request for WeaponProjectileScale
            uint64_t weaponProjectileScaleAddress = WeaponEntity + OFF_PROJECTILESCALE;
            mem.AddScatterReadRequest(handle, weaponProjectileScaleAddress, &WeaponProjectileScale, sizeof(float));

            // Scatter read request for Weapon Ammo
            uint64_t weaponAmmoAddress = WeaponEntity + OFF_WEAPON_AMMO;
            mem.AddScatterReadRequest(handle, weaponAmmoAddress, &Ammo, sizeof(int));

            // Scatter read request for IsReloading
            uint64_t isReloadingAddress = WeaponEntity + OFF_RELOADING;
            mem.AddScatterReadRequest(handle, isReloadingAddress, &IsReloading, sizeof(bool));

            // Execute the scatter read
            mem.ExecuteReadScatter(handle);

            // Close the scatter handle
            mem.CloseScatterHandle(handle);
        }
    }

    bool IsValid() {
        return mem.IsValidPointer(BasePointer);
    }

    bool IsCombatReady() {
        if (!mem.IsValidPointer(BasePointer)) return false;
        if (IsDead) return false;
        if (IsKnocked) return false;
        return true;
    }
};