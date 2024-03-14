/**
* Copyright (C) 2024 Murugo
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"

namespace
{
    constexpr BYTE kAnimIndexReady = 0x1A;
    constexpr BYTE kAnimIndexAttacking = 0x1C;

    constexpr BYTE kWeaponWoodenPlank = 0x05;
    constexpr BYTE kWeaponGreatKnife = 0x08;
    constexpr BYTE kWeaponChineseCleaver = 0x0A;

    // Maximum duration to buffer a light attack after quickly tapping the attack button.
    constexpr float kLightAttackBufferTimerMax = 0.3f;
    // Maximum duration the attack button can be held before triggering a heavy attack.
    constexpr float kHeavyAttackHoldTimerMax = 0.2f;

    // Variables for ASM
    float HeavyAttackHoldTimer = 0.0f;
    float LightAttackBufferTimer = 0.0f;
    BYTE QueueAttackIndex = 0;

    DWORD* PlayerRootAddr = 0;
    BYTE* AttackButtonHeldAddr = nullptr;
    BYTE* AttackButtonDownAddr = nullptr;
    BYTE* AttackButtonReleasedAddr = nullptr;
    BYTE* CurrentWeaponIndexAddr = nullptr;
    BYTE* PlayerAnimIndexUAddr = nullptr;
    BYTE* IsPlayerRunningAddr = nullptr;
    BYTE* CurrentAttackIndexAddr = nullptr;
    BYTE* RequestAttackIndexAddr = nullptr;
    BYTE* IsAttackRepeatingAddr = nullptr;

    DWORD* HandleAttackInputReturnAddr = nullptr;
    DWORD* RequestMeleeAttackJumpAddr = nullptr;
    DWORD* RequestAttackReturnAddr = nullptr;

    typedef float(__cdecl* GetDeltaTimeProc)();
    GetDeltaTimeProc GetDeltaTime = nullptr;
}

bool IsMeleeWeapon(char weaponIndex) {
    return (weaponIndex >= kWeaponWoodenPlank && weaponIndex <= kWeaponGreatKnife) || weaponIndex == kWeaponChineseCleaver;
}

// ASM that returns the correct attack index for the current weapon given a
// custom attack button sensitivity value.
BYTE RequestMeleeAttack(BYTE sensitivity)
{
    __asm
    {
        mov bl, sensitivity
        mov edi, 0x00
        mov esi, dword ptr ds : [PlayerRootAddr]
        push esi
        call RequestMeleeAttackAux
        pop esi
        movzx eax, byte ptr ds : [esi + 0x493]
        jmp ExitAsm

    RequestMeleeAttackAux:
        sub esp, 0x0C
        // Jump to the switch statement which requests an attack for the current weapon.
        jmp RequestMeleeAttackJumpAddr

    ExitAsm:
    }
}

BYTE GetAttackIndex(bool heavy)
{
    const BYTE preserveRequestedAttack = *RequestAttackIndexAddr;
    const BYTE preserveIsAttackRepeating = *IsAttackRepeatingAddr;
    *IsAttackRepeatingAddr = 0x00;

    RequestMeleeAttack(/*sensitivity=*/heavy ? 0xFF : 0x01);

    const BYTE newAttackIndex = *RequestAttackIndexAddr;
    *RequestAttackIndexAddr = preserveRequestedAttack;
    *IsAttackRepeatingAddr = preserveIsAttackRepeating;

    // Do not request a heavy attack if any attack is active.
    if (newAttackIndex == 0x04 && *CurrentAttackIndexAddr != 0)
        return 0;
    return newAttackIndex;
}

void HandleHeavyAttack(float deltaTime)
{
    HeavyAttackHoldTimer += deltaTime;
    if (HeavyAttackHoldTimer < kHeavyAttackHoldTimerMax)
        return;

    const BYTE attackIndex = GetAttackIndex(/*heavy=*/true);
    if (attackIndex != 0)
        QueueAttackIndex = attackIndex;
}

void HandleLightAttack()
{
    if (*AttackButtonReleasedAddr == 0)
        return;
    // Do not queue a light attack if a heavy attack is active.
    if (*CurrentAttackIndexAddr == 0x04)
        return;

    LightAttackBufferTimer = kLightAttackBufferTimerMax;
    const BYTE attackIndex = GetAttackIndex(/*heavy=*/false);
    if (attackIndex != 0)
        QueueAttackIndex = attackIndex;
}

void HandleAttackInput() {
    const float deltaTime = GetDeltaTime();
    LightAttackBufferTimer -= deltaTime;
    // Stop queueing an attack if the buffer timer expires, or if kicking/stomping an enemy.
    if (LightAttackBufferTimer <= 0.0f || *CurrentAttackIndexAddr > 5)
    {
        LightAttackBufferTimer = 0.0f;
        QueueAttackIndex = 0;
    }
    if (*AttackButtonDownAddr != 0)
        HeavyAttackHoldTimer = 0;

    if (IsMeleeWeapon(*CurrentWeaponIndexAddr) &&
        (*PlayerAnimIndexUAddr == kAnimIndexReady ||
            *PlayerAnimIndexUAddr == kAnimIndexAttacking))
    {
        const bool forceLightAttack = (*CurrentWeaponIndexAddr == kWeaponWoodenPlank || *CurrentWeaponIndexAddr == kWeaponChineseCleaver) && *IsPlayerRunningAddr != 0;
        if (*AttackButtonHeldAddr != 0 && !forceLightAttack)
            HandleHeavyAttack(deltaTime);
        else
            HandleLightAttack();
    }
}

// ASM boilerplate to handle attack input.
__declspec(naked) void __stdcall HandleAttackInputASM()
{
    __asm
    {
        call HandleAttackInput
        mov ecx, dword ptr ds : [PlayerRootAddr]
        mov ecx, dword ptr ds : [ecx]
        mov eax, esi
        shr eax, 0x12
        jmp HandleAttackInputReturnAddr
    }
}

// ASM which requests a queued attack index if one is set. Applies only to
// melee attacks.
__declspec(naked) void __stdcall RequestAttackASM()
{
    __asm
    {
        movzx eax, byte ptr ds : [esi + 0x48C]
        push eax
        call IsMeleeWeapon
        add esp, 0x04
        test eax, eax
        jz ExitAsm

        mov al, byte ptr ds : [QueueAttackIndex]
        test al, al
        jz Done
        mov edi, [esp + 0x14]
        mov byte ptr ds : [esi + edi + 0x493], al
        mov byte ptr ds : [QueueAttackIndex], 0x00
        mov dword ptr ds : [LightAttackBufferTimer], 0x00
        mov dword ptr ds : [HeavyAttackHoldTimer], 0x00

    Done:
        pop edi
        pop esi
        pop ebx
        ret

    ExitAsm:
        mov edi, [esp + 0x14]
        mov al, [esi + edi + 0x493]
        jmp RequestAttackReturnAddr
    }
}

// ASM which skips clearing the requested attack if holding a melee weapon
// and the attack button is not held down. This is necessary for queueing
// light attacks in between button taps.
__declspec(naked) void __stdcall SkipClearRequestAttackASM()
{
    __asm
    {
        mov eax, dword ptr ds : [PlayerRootAddr]
        movzx eax, byte ptr ds : [eax + 0x48C]
        push eax
        call IsMeleeWeapon
        add esp, 0x04
        test eax, eax
        jz ExitAsm
        mov al, 0x01
        ret

    ExitAsm:
        mov eax, dword ptr ds : [PlayerRootAddr]
        mov al, byte ptr ds : [eax + 0x28E]
        ret
    }
}

// ASM which automatically readies a melee weapon on Beginner or Easy action
// difficulty. Invoked only when an enemy is nearby.
__declspec(naked) void __stdcall AutoReadyWeaponASM()
{
    __asm
    {
        mov ecx, dword ptr ds : [PlayerRootAddr]
        movzx eax, byte ptr ds : [ecx + 0x48C]
        push eax
        call IsMeleeWeapon
        add esp, 0x04
        test eax, eax
        jz ReadyWeapon
        
        // If the attack button is held down, hold guard.
        mov al, byte ptr ds : [ecx + 0x28E]
        test al, al
        jnz ReadyWeapon

        // If a kick or stomp attack is active, don't hold guard.
        mov al, byte ptr ds : [ecx + 0x492]
        cmp al, 0x05
        ja ExitAsm

        // If any other attack is active, hold guard.
        test al, al
        jnz ReadyWeapon

        // If an attack is already queued, hold guard.
        mov al, byte ptr ds : [QueueAttackIndex]
        test al, al
        jz ExitAsm

    ReadyWeapon:
        mov byte ptr ds : [ecx + 0x292], 0x04

    ExitAsm:
        ret
    }
}

void PatchSwapLightHeavyAttack()
{
    // Get address to inject logic to queue melee attacks based on player input.
    constexpr BYTE SearchBytesHandleInput[]{ 0x8B, 0xC6, 0xC1, 0xE8, 0x12, 0x24, 0x03 };
    DWORD HandleAttackInputAddr = SearchAndGetAddresses(0x0052EB83, 0x0052EEB3, 0x0052E7D3, SearchBytesHandleInput, sizeof(SearchBytesHandleInput), -0x06, __FUNCTION__);
    if (!HandleAttackInputAddr)
    {
        Logging::Log() << __FUNCTION__ << "Error: failed to find memory address!";
        return;
    }
    HandleAttackInputReturnAddr = (DWORD*)(HandleAttackInputAddr + 0x06);
    memcpy(&PlayerRootAddr, (void*)(HandleAttackInputAddr + 0x02), sizeof(DWORD*));

    // Set addresses for player state.
    AttackButtonHeldAddr = (BYTE*)(PlayerRootAddr) + 0x28E;
    AttackButtonDownAddr = (BYTE*)(PlayerRootAddr) + 0x290;
    AttackButtonReleasedAddr = (BYTE*)(PlayerRootAddr) + 0x2AE;
    CurrentWeaponIndexAddr = (BYTE*)(PlayerRootAddr) + 0x48C;
    PlayerAnimIndexUAddr = (BYTE*)(PlayerRootAddr) + 0x64;
    IsPlayerRunningAddr = (BYTE*)(PlayerRootAddr) + 0x488;
    CurrentAttackIndexAddr = (BYTE*)(PlayerRootAddr) + 0x492;
    RequestAttackIndexAddr = (BYTE*)(PlayerRootAddr) + 0x493;
    IsAttackRepeatingAddr = (BYTE*)(PlayerRootAddr) + 0x499;

    // Get address to inject logic to request a queued melee attack.
    constexpr BYTE SearchBytesRequestAttack[]{ 0x8B, 0x7C, 0x24, 0x14, 0x8A, 0x84, 0x3E, 0x93, 0x04, 0x00, 0x00 };
    DWORD RequestAttackAddr = SearchAndGetAddresses(0x005343B7, 0x005346E7, 0x00534007, SearchBytesRequestAttack, sizeof(SearchBytesRequestAttack), 0x00, __FUNCTION__);
    if (!RequestAttackAddr)
    {
        Logging::Log() << __FUNCTION__ << "Error: failed to find memory address!";
        return;
    }
    RequestAttackReturnAddr = (DWORD*)(RequestAttackAddr + 0x0B);
    RequestMeleeAttackJumpAddr = (DWORD*)(RequestAttackAddr + 0x25);

    DWORD* GetDeltaTimeFuncPtr = GetDeltaTimeFunctionPointer();
    if (!GetDeltaTimeFuncPtr)
    {
        Logging::Log() << __FUNCTION__ << "Error: failed to find memory address for GetDeltaTime!";
        return;
    }
    GetDeltaTime = (GetDeltaTimeProc)GetDeltaTimeFuncPtr;

    Logging::Log() << "Enabling Swap Light and Heavy Melee Attack...";
    WriteJMPtoMemory((BYTE*)HandleAttackInputAddr, *HandleAttackInputASM, 0x05);
    WriteJMPtoMemory((BYTE*)RequestAttackAddr, *RequestAttackASM, 0x0B);
    WriteCalltoMemory((BYTE*)(HandleAttackInputAddr - 0x57), *SkipClearRequestAttackASM, 0x05);
    WriteCalltoMemory((BYTE*)(HandleAttackInputAddr - 0x17), *AutoReadyWeaponASM, 0x07);
}
