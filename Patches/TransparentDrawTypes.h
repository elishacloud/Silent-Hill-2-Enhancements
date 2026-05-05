#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"

enum class DrawCallType {
    Nop = 0x7FFFFFFF,
    Custom = 0x2C,
    type2D = 0x2D,
    type2E = 0x2E,
    type22 = 22,
    type33 = 33
};

// Draws nothing
struct DrawCallTypeNop
{
};

// Just calls func with the provided arg (For fully custom rendering logic?)
// If a specific local var in drawTransparent is non-zero (probably a bitfield) it may also run setup and other render state logic
struct DrawCallTypeCustom
{
    void(__cdecl* func)(DWORD);
    DWORD arg;
    DWORD pad; // Unused
    void(*setup)();
};

struct DrawCallType2D
{
    IDirect3DBaseTexture8* texture;
    IDirect3DBaseTexture8* texture2; // May actually be a function pointer
    IDirect3DVertexBuffer8* vertexBuffer;
    DWORD startVertex;

    // Affect VS constants somehow
    DWORD vsConstantMod1;
    DWORD vsConstantMod2;

    USHORT stride;
    BYTE primitiveCount; // +2 for some reason
    BYTE unk_23; // Related to material selection
};

// Uses DrawPrimitiveUP with D3DPT_TRIANGLEFAN
struct DrawCallType2E
{
    IDirect3DBaseTexture8* texture;
    DWORD pad[2];
    IDirect3DBaseTexture8* texture2;
    USHORT primitiveCount;
    USHORT unk_1A;
    void* vertices;
};

struct DrawCallType33
{
    IDirect3DBaseTexture8* texture;
    DWORD vsHandle;
    USHORT stride;
    USHORT primitiveCount;

    //0 = D3DTOP_SELECTARG2, 1 = D3DTOP_SUBTRACT, 2 = D3DTOP_MODULATE, 3 = D3DTOP_MODULATE2X, 4 = D3DTOP_MODULATE4X
    BYTE colorOpIndex;
    BYTE alphaOpIndex;

    USHORT pad;
    void* vertices;
    D3DMATRIX matrix; // Not used in types less than 33
};

// A tagged union of possible draw call variants
struct DrawCallNode
{
    DrawCallNode* pNext;
    DrawCallType type;
    union {
        DrawCallTypeNop typeNop;
        DrawCallTypeCustom typeCustom;
        DrawCallType2D type2D;
        DrawCallType2E type2E;
        DrawCallType33 type33;
    };
};

struct DrawCalls
{
    void* unknown_00;
    void* unknown_04;
    void* unknown_08;
    DrawCallNode* head;
};
