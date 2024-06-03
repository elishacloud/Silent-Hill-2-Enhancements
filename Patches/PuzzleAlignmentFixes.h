#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"

struct StyleBits
{
    unsigned short textured        : 1; // 0 = flat shaded, 1 = textured
    unsigned short unk02           : 1; // affects the vertices, draws larger
    unsigned short useTexCoords    : 1; // 0 = draw entire texture, 1 = draw portion given by texCoords
    unsigned short unk08           : 1;
    unsigned short unk10           : 1;
    unsigned short useTransparency : 1;
    unsigned short unk40           : 1;
    unsigned short fourCorners     : 1; // 0 = quad is specified by top left and bottom right vertices. 1 = vertices are positioned independently.
    unsigned short unk100          : 1;
};

struct UIElement
{
    IDirect3DBaseTexture8* texture;
    DWORD dword4; // probably another texture
    DWORD id;     // avoid trusting this value, the game never uses it and it differs between region/versions

    struct {
        short x;
        short y;
    } verts[4];   // If style.fourCorners == 0, only verts[0] and verts[1] are used

    struct {
        float u1;
        float v1;
        float u2;
        float v2;
    } texCoords;

    WORD word2C;
    StyleBits style;
    WORD word30;
    WORD word32;

    WORD width;
    WORD height;

    struct {
        unsigned char b;
        unsigned char g;
        unsigned char r;
        unsigned char a;
    } color;

    DWORD reserved3C;
    DWORD reserved40;
};
static_assert(sizeof(UIElement) == 0x44);

void PatchPuzzleAlignmentFixes();
