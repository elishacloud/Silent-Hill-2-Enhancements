#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"

constexpr DWORD vsDecl[] =
{
    D3DVSD_STREAM(0),
    D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
    D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3),
    D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2),
    D3DVSD_END()
};

/*
// Assembled with `vsa.exe -h0` from DirectX 8.1b SDK

vs.1.1
//dcl_position v0
//dcl_normal v3
//dcl_texcoord v7

mov r0, c0
mov r1, c0

mov oT1, c0

// vertex position
dp4 r0.x, v0, c39
dp4 r0.y, v0, c40
dp4 r0.z, v0, c41
dp4 r0.w, v0, c42
mov oPos, r0

// vertex fog
max r1.x, r0.w, c0.x
rcp r1.x, r1.x
mad oFog, r1.x, c46.y, c46.x

// specular highlight
dp4 oT1.x, v3, c89
dp4 oT1.y, v3, c90

*/
constexpr DWORD nurseVertexShader[] =
{
    0xfffe0101, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000001,
    0x800f0000, 0xa0e40000, 0x00000001, 0x800f0001,
    0xa0e40000, 0x00000001, 0xe00f0001, 0xa0e40000,
    0x00000009, 0x80010000, 0x90e40000, 0xa0e40027,
    0x00000009, 0x80020000, 0x90e40000, 0xa0e40028,
    0x00000009, 0x80040000, 0x90e40000, 0xa0e40029,
    0x00000009, 0x80080000, 0x90e40000, 0xa0e4002a,
    0x00000001, 0xc00f0000, 0x80e40000, 0x0000000b,
    0x80010001, 0x80ff0000, 0xa0000000, 0x00000006,
    0x80010001, 0x80000001, 0x00000004, 0xc00f0001,
    0x80000001, 0xa055002e, 0xa000002e, 0x00000009,
    0xe0010001, 0x90e40003, 0xa0e40059, 0x00000009,
    0xe0020001, 0x90e40003, 0xa0e4005a, 0x0000ffff
};

/*
// Assembled with `psa.exe -h0` from DirectX 8.1b SDK

ps.1.4
def c4, 0, 0, 0, 1  // black
texld r1, t1        // load specular highlight texture
mov r0, c4		    // make output pixel black
mul r0.rgb, r1, c3	// apply the specular highlight with tint color from c3

*/
constexpr DWORD nursePixelShader[] = {
    0xffff0104, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000051,
    0xa00f0004, 0x00000000, 0x00000000, 0x00000000,
    0x3f800000, 0x00000042, 0x800f0001, 0xb0e40001,
    0x00000001, 0x800f0000, 0xa0e40004, 0x00000005,
    0x80070000, 0x80e40001, 0xa0e40003, 0x0000ffff
};

constexpr DWORD windowVertexShader[] = {
    0xfffe0101, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000001,
    0x800f000b, 0xa0e40000, 0x00000001, 0x800f000a,
    0xa0e40000, 0x00000001, 0x800f0009, 0xa0e40000,
    0x00000001, 0x800f0008, 0xa0e40000, 0x00000001,
    0x800f0007, 0xa0e40000, 0x00000001, 0x800f0006,
    0xa0e40000, 0x00000001, 0x800f0005, 0xa0e40000,
    0x00000001, 0x800f0003, 0xa0e40000, 0x00000001,
    0x800f0002, 0xa0e40000, 0x00000001, 0x800f0001,
    0xa0e40000, 0x00000001, 0x800f0000, 0xa0e40000,
    0x00000001, 0xd00f0001, 0xa0e40000, 0x00000001,
    0xd00f0000, 0xa0e40000, 0x00000001, 0xe00f0004,
    0xa0e40000, 0x00000001, 0xe00f0003, 0xa0e40000,
    0x00000001, 0xe00f0002, 0xa0e40000, 0x00000001,
    0xe00f0001, 0xa0e40000, 0x00000001, 0xe00f0000,
    0xa0e40000, 0x00000001, 0x80070000, 0x90e40003,
    0x00000008, 0x80010002, 0x80a40000, 0xa0e40008,
    0x00000008, 0x80020002, 0x80a40000, 0xa0e40009,
    0x00000008, 0x80040002, 0x80a40000, 0xa0e4000a,
    0x00000001, 0xe0030004, 0x80e40002, 0x00000009,
    0xe0010002, 0x90e40000, 0xa0e40005, 0x00000009,
    0xe0020002, 0x90e40000, 0xa0e40004, 0x00000009,
    0x80080002, 0x90e40000, 0xa0e40006, 0x00000001,
    0xe0080002, 0x80ff0002, 0x00000001, 0xe0040003,
    0x80ff0002, 0x00000002, 0x80070005, 0xa0e4000b,
    0x91e40000, 0x00000008, 0x80010001, 0x80e40005,
    0x80e40005, 0x00000007, 0x80010001, 0x80000001,
    0x00000005, 0x80070005, 0x80a40005, 0x80000001,
    0x00000004, 0x80080002, 0x80ff0002, 0xa0000012,
    0xa0550012, 0x0000000b, 0x800f0002, 0x80e40002,
    0xa000000e, 0x00000008, 0x80010003, 0x80e40002,
    0xa0e4000f, 0x00000008, 0x80020003, 0x80e40002,
    0xa0e40010, 0x00000008, 0x80040003, 0x80e40002,
    0xa0e40011, 0x00000008, 0x80010006, 0x80a40000,
    0x80e40005, 0x0000000a, 0x80080002, 0x80ff0002,
    0xa0ff000e, 0x00000005, 0xd0080000, 0x80ff0002,
    0x80000006, 0x00000009, 0x8001000a, 0x90e40000,
    0xa0e40000, 0x00000009, 0x8002000a, 0x90e40000,
    0xa0e40001, 0x00000009, 0x8004000a, 0x90e40000,
    0xa0e40002, 0x00000009, 0x8008000a, 0x90e40000,
    0xa0e40003, 0x0000000b, 0x8008000b, 0x80ff000a,
    0xa0ff000e, 0x00000006, 0x800f000b, 0x80ff000b,
    0x00000004, 0xc00f0001, 0xa0550017, 0x8000000b,
    0xa0000017, 0x00000001, 0xe0030000, 0x90e40007,
    0x00000002, 0x80070009, 0xa0e4000d, 0x80a40005,
    0x00000008, 0x80010007, 0x80e40009, 0x80e40009,
    0x00000008, 0x80030008, 0x80e40009, 0x80a40000,
    0x00000007, 0x80080001, 0x80000007, 0x00000002,
    0x80070003, 0x80a40003, 0xa0a40013, 0x00000005,
    0x80070003, 0x80a40003, 0xa0a4001a, 0x00000002,
    0xd0070000, 0x80a40003, 0x80a40003, 0x00000001,
    0xd0070001, 0xa0a4001b, 0x00000001, 0xc00f0000,
    0x80e4000a, 0x00000005, 0xe0030001, 0x80e40008,
    0x80ff0001, 0x0000ffff
};

constexpr DWORD windowPixelShader[] = {
    0xffff0104, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000051,
    0xa00f0005, 0x3f800000, 0x00000000, 0x00000000,
    0x00000000, 0x00000051, 0xa00f0004, 0x00000000,
    0x00000000, 0x00000000, 0x3f800000, 0x00000042,
    0x800f0000, 0xb0e40000, 0x00000042, 0x800f0004,
    0xb0e40004, 0x00000050, 0x80070000, 0x80ff0004,
    0x80e40004, 0x80e40000, 0x0000ffff
};

extern DWORD nurseVsHandle;
extern DWORD nursePsHandle;

extern DWORD windowVsHandle;
extern DWORD windowPsHandle;

struct SubSubSomething
{
    unsigned __int16 off_0x00;
    unsigned __int8 off_0x02;
    unsigned __int8 off_0x03;
    unsigned __int16 off_0x04;
    unsigned __int16 off_0x06;
};

struct SubSomething
{
    DWORD off_0x00;
    DWORD off_0x04;
    IDirect3DTexture8* tex1;
    IDirect3DTexture8* tex2;
    unsigned __int8 off_0x10;
};

struct SubSomething2
{
    UINT off_0x00;
    UINT off_0x04;
    UINT off_0x08;
};

struct SubSomething3
{
    int off_0x00;
    int off_0x04;
    unsigned int off_0x08;
    SubSubSomething off_0x0C[];
};

struct VBStruct
{
    IDirect3DVertexBuffer8* vertexBuffer;
};

struct Something
{
    float unkFloats[8];
    VBStruct* vertexBufferStruct;
    IDirect3DIndexBuffer8* indexBuffer;
    SubSomething* pSubStruct;
    SubSomething2* strides;
    DWORD off_0x30;
    SubSomething3* pSubStruct3;
};

struct struct_a1_sub
{
    UINT numVertices;
    IDirect3DIndexBuffer8* indexBuffer;
    UINT dword8;
    UINT dwordC;
};

struct struct_a1
{
    unsigned __int8 byte0[4];
    BYTE gap4[4];
    DWORD dword8;
    DWORD dwordC;
    DWORD dword10;
    DWORD dword14;
    BYTE gap18[4];
    DWORD dword1C;
    DWORD dword20;
    BYTE byte24;
    BYTE byte25;
    __declspec(align(4)) BYTE cullMode;
    float tintA;
    float float30;
    float specularSize;
    struct_a1_sub* subStruct;
    BYTE gap3C[8];
    float tintR;
    float tintG;
    float tintB;
    BYTE gap50[4];
    float float54;
    float float58;
    float float5C;
    BYTE gap60[4];
    float specularR;
    float specularG;
    float specularB;
    BYTE gap70[8];
    UINT primCount;
};
static_assert(sizeof(struct_a1) == 0x7C);

struct struct_dword_1F7D4AC
{
    IDirect3DBaseTexture8* tex;
};

struct MapVsConstants
{
    D3DMATRIX mvp;
    D3DMATRIX texStage1Mat;
    float constants[20][4];
    //...
};
static_assert(sizeof(MapVsConstants) == 0x1C0);
