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

constexpr DWORD vsDeclVColor[] =
{
    D3DVSD_STREAM(0),
    D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
    D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3),
    D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR),
    D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2),
    D3DVSD_END()
};

/*
// Assembled with `vsa.exe -h0` from DirectX 8.1b SDK

vs.1.1

//v0 - position
//v3 - normal
//v7 - texcoord

// this is pretty much the original shader, I just calculate the per-vertex direction to the camera
// and pass it + half vector to pixel shader, while the orogonal shader was calculating angle here
// and then was supposed to sample some texture in pixel shader
// that sucs and results in poor specular effect due to per-vertex calculations

// sh2ee regs:
// c90 - flashLightPos
// c91 - cameraPos

// this seems to be the flashlight's world matrix (transposed)
// c8, c9, c10, c11 - matrix (float4x4)
// load and transform normal
mov r0.xyz, v3
dp3 r2.x, r0.xyzz, c8
dp3 r2.y, r0.xyzz, c9
dp3 r2.z, r0.xyzz, c10

// this seems to be flashlight's MVP matric
// we "project" our vertex using it to calculate flashlight's texture uvs
// c4, c5, c6 - float4x3
dp4 oT2.x, v0, c5
dp4 oT2.y, v0, c4
dp4 r2.w, v0, c6
mov oT2.w, r2.w
mov oT3.z, r2.w

// calculate dir to flashlight
// r5 = normalize(c11 - pos)
add r5.xyz, c11, -v0
dp3 r1.x, r5, r5
rsq r1.x, r1.x
mul r5.xyz, r5.xyzz, r1.x

// c14 = 0.000 0.500 255.000 1.000
// c15 = 0.000 0.013 0.002 0.000
// c16 = 0.000 0.013 0.002 0.000
// c17 = 0.000 0.013 0.002 0.000
// c18 = -0.000 1.491 10.893 -9.893
mad r2.w, r2.w, c18.x, c18.y
max r2, r2, c14.x
dp3 r3.x, r2, c15
dp3 r3.y, r2, c16
dp3 r3.z, r2, c17
dp3 r6.x, r0.xyzz, r5
min r2.w, r2.w, c14.w
// diffuse alpha is based on vertex projected depth in light's space remapped to whatev bullshit
mul oD0.w, r2.w, r6.x

// now calc clip space pos
// c0, c1, c2, c3 - matrix (float4x4)
// r10 = mul(IN.position, ModelViewProjection)
dp4 r10.x, v0, c0
dp4 r10.y, v0, c1
dp4 r10.z, v0, c2
dp4 r10.w, v0, c3

// c14 = 0.000 0.500 255.000 1.000
// c23 = -0.698 5237.045 0.000 0.000
// simple linear fog
max r11.w, r10.w, c14.w
rcp r11, r11.w
mad oFog, c23.y, r11.x, c23.x

// texcoord passthrough
mov oT0.xy, v7

// half vector between vec to camera and vec to light
// c91 - camera pos
// r9 = normalize(cameraPos - vertexPos)
add r9.xyz, c91, -v0
dp3 r11.x, r9, r9
rsq r11.y, r11.x
mul r9, r9.xyzz, r11.y

// halfVec = normalize(dirToLight + dirToCamera)
add r9, r9, r5.xyzz
dp3 r11.x, r9, r9
rsq r11.y, r11.x
mul r9, r9, r11.y

// pass half vec and normal to pixel shader
mov oT1, r9
mov oT4, r0.xyzz

// some weird mumbo-jumbo, have no idea tbh, calc diffuse color
// c19 = -0.000 1.491 10.893 -9.893
// c26 = 0.698 0.698 0.698 1.000
add r3.xyz, r3.xyzz, c19.xyzz
mul r3.xyz, r3.xyzz, c26.xyzz
add oD0.xyz, r3.xyzz, r3.xyzz

// pass specular color to pixel shader
mov oD1.xyz, c27.xyzz

// finally write the pos
mov oPos, r10

*/
constexpr DWORD windowVertexShader[] = {
    0xfffe0101, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000001,
    0x80070000, 0x90e40003, 0x00000008, 0x80010002,
    0x80a40000, 0xa0e40008, 0x00000008, 0x80020002,
    0x80a40000, 0xa0e40009, 0x00000008, 0x80040002,
    0x80a40000, 0xa0e4000a, 0x00000009, 0xe0010002,
    0x90e40000, 0xa0e40005, 0x00000009, 0xe0020002,
    0x90e40000, 0xa0e40004, 0x00000009, 0x80080002,
    0x90e40000, 0xa0e40006, 0x00000001, 0xe0080002,
    0x80ff0002, 0x00000001, 0xe0040003, 0x80ff0002,
    0x00000002, 0x80070005, 0xa0e4000b, 0x91e40000,
    0x00000008, 0x80010001, 0x80e40005, 0x80e40005,
    0x00000007, 0x80010001, 0x80000001, 0x00000005,
    0x80070005, 0x80a40005, 0x80000001, 0x00000004,
    0x80080002, 0x80ff0002, 0xa0000012, 0xa0550012,
    0x0000000b, 0x800f0002, 0x80e40002, 0xa000000e,
    0x00000008, 0x80010003, 0x80e40002, 0xa0e4000f,
    0x00000008, 0x80020003, 0x80e40002, 0xa0e40010,
    0x00000008, 0x80040003, 0x80e40002, 0xa0e40011,
    0x00000008, 0x80010006, 0x80a40000, 0x80e40005,
    0x0000000a, 0x80080002, 0x80ff0002, 0xa0ff000e,
    0x00000005, 0xd0080000, 0x80ff0002, 0x80000006,
    0x00000009, 0x8001000a, 0x90e40000, 0xa0e40000,
    0x00000009, 0x8002000a, 0x90e40000, 0xa0e40001,
    0x00000009, 0x8004000a, 0x90e40000, 0xa0e40002,
    0x00000009, 0x8008000a, 0x90e40000, 0xa0e40003,
    0x0000000b, 0x8008000b, 0x80ff000a, 0xa0ff000e,
    0x00000006, 0x800f000b, 0x80ff000b, 0x00000004,
    0xc00f0001, 0xa0550017, 0x8000000b, 0xa0000017,
    0x00000001, 0xe0030000, 0x90e40007, 0x00000002,
    0x80070009, 0xa0e4005b, 0x91e40000, 0x00000008,
    0x8001000b, 0x80e40009, 0x80e40009, 0x00000007,
    0x8002000b, 0x8000000b, 0x00000005, 0x800f0009,
    0x80a40009, 0x8055000b, 0x00000002, 0x800f0009,
    0x80e40009, 0x80a40005, 0x00000008, 0x8001000b,
    0x80e40009, 0x80e40009, 0x00000007, 0x8002000b,
    0x8000000b, 0x00000005, 0x800f0009, 0x80e40009,
    0x8055000b, 0x00000001, 0xe00f0001, 0x80e40009,
    0x00000001, 0xe00f0004, 0x80a40000, 0x00000002,
    0x80070003, 0x80a40003, 0xa0a40013, 0x00000005,
    0x80070003, 0x80a40003, 0xa0a4001a, 0x00000002,
    0xd0070000, 0x80a40003, 0x80a40003, 0x00000001,
    0xd0070001, 0xa0a4001b, 0x00000001, 0xc00f0000,
    0x80e4000a, 0x0000ffff
};

/*
// Assembled with `psa.exe -h0` from DirectX 8.1b SDK

ps.1.4
  def c0, 0.5, 0.5, 0.5, 0.5
  def c3, 0.0, 0.0, 0.0, 0.0
  def c5, 0.0, 0.0, 1.0, 0.0

  texcrd r3.xyz, t3

  // load vector and normal from texcoords
  texcrd r4.xyz, t1
  texcrd r5.xyz, t4

  // normalization approximation by Nvidia (using normalization cube somehow looks like shit)
  // half vector
  mov r4.w, c3
  mul r1, r4, c0
  dp3 r2, r4, r4
  mad r4, 1-r2, r1, r4

  // normal vector
  // not enough instructions, but normals are very close to be unit, so it is "good enough"
  //mul r1, r5, c0
  //dp3 r2, r5, r5
  //mad r5, 1-r2, r1, r5

  // angle between half vector and normal
  dp3_sat r1, r4, r5

  // check if looking towards pixel
  dp3 r5, r3, c5

phase

  texld r0, t0
  texld r1, r1
  texld r2, t2_dw.xyw

  // save albedo
  mov r4, r0

  // specular color = specular function (from texture 1) * specular color from vertex shader
  mul r1.xyz, r1.w, v1
  // also modulate specular color by the albedo's alpha channel ("roughness", means we have to invert the value)
  mul r1.xyz, r1, 1-r0.w

  // r1 = albedo texel * ambient color (???) + specular
  // r2.w = flashlight's func * albedo alpha
  mad r1.xyz, r4, c1, r1
+ mul r2.w, r2.w, v0.w

  // r2 = combine ambient, albedo, specular and flashlight
  mul_x2 r2.xyz, r1, r2.w

  // if looking towards -> take t2, else - take black (c3)
  cnd r0.xyz, r5.x, r2, c3

  // diffuse color from vertex * albedo + lighted texel (???)
  // and simply copy alpha from albedo to output
  mad r0.xyz, v0, r4, r0

*/
constexpr DWORD windowPixelShader[] = {
    0xffff0104, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000051,
    0xa00f0000, 0x3f000000, 0x3f000000, 0x3f000000,
    0x3f000000, 0x00000051, 0xa00f0003, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000051,
    0xa00f0005, 0x00000000, 0x00000000, 0x3f800000,
    0x00000000, 0x00000040, 0x80070003, 0xb0e40003,
    0x00000040, 0x80070004, 0xb0e40001, 0x00000040,
    0x80070005, 0xb0e40004, 0x00000001, 0x80080004,
    0xa0e40003, 0x00000005, 0x800f0001, 0x80e40004,
    0xa0e40000, 0x00000008, 0x800f0002, 0x80e40004,
    0x80e40004, 0x00000004, 0x800f0004, 0x86e40002,
    0x80e40001, 0x80e40004, 0x00000008, 0x801f0001,
    0x80e40004, 0x80e40005, 0x00000008, 0x800f0005,
    0x80e40003, 0xa0e40005, 0x0000fffd, 0x00000042,
    0x800f0000, 0xb0e40000, 0x00000042, 0x800f0001,
    0x80e40001, 0x00000042, 0x800f0002, 0xbaf40002,
    0x00000001, 0x800f0004, 0x80e40000, 0x00000005,
    0x80070001, 0x80ff0001, 0x90e40001, 0x00000005,
    0x80070001, 0x80e40001, 0x86ff0000, 0x00000004,
    0x80070001, 0x80e40004, 0xa0e40001, 0x80e40001,
    0x40000005, 0x80080002, 0x80ff0002, 0x90ff0000,
    0x00000005, 0x81070002, 0x80e40001, 0x80ff0002,
    0x00000050, 0x80070000, 0x80000005, 0x80e40002,
    0xa0e40003, 0x00000004, 0x80070000, 0x90e40000,
    0x80e40004, 0x80e40000, 0x0000ffff
};

/*
// Assembled with `vsa.exe -h0` from DirectX 8.1b SDK

vs.1.1

// v0 - position
// v3 - normal
// v5 - color
// v7 - texcoord

// sh2ee regs:
// c90 - flashLightPos
// c91 - cameraPos

// this seems to be the flashlight's world matrix (transposed)
// c8, c9, c10, c11 - matrix (float4x4)
// load and transform normal
mov r0.xyz, v3
dp3 r2.x, r0.xyzz, c8
dp3 r2.y, r0.xyzz, c9
dp3 r2.z, r0.xyzz, c10

// this seems to be flashlight's MVP matric
// we "project" our vertex using it to calculate flashlight's texture uvs
// c4, c5, c6 - float4x3
dp4 oT2.x, v0, c5
dp4 oT2.y, v0, c4
dp4 r2.w, v0, c6
mov oT2.w, r2.w
mov oT3.z, r2.w

// calculate dir to flashlight
// r5 = normalize(c11 - pos)
add r5.xyz, c11, -v0
dp3 r1.x, r5, r5
rsq r1.x, r1.x
mul r5.xyz, r5.xyzz, r1.x

// c14 = 0.000 0.500 255.000 1.000
// c15 = 0.000 0.013 0.002 0.000
// c16 = 0.000 0.013 0.002 0.000
// c17 = 0.000 0.013 0.002 0.000
// c18 = -0.000 1.491 10.893 -9.893
mad r2.w, r2.w, c18.x, c18.y
max r2, r2, c14.x
dp3 r3.x, r2, c15
dp3 r3.y, r2, c16
dp3 r3.z, r2, c17
dp3 r6.x, r0.xyzz, r5
min r2.w, r2.w, c14.w
// diffuse alpha is based on vertex projected depth in light's space remapped to whatev bullshit
mul oD0.w, r2.w, r6.x

// now calc clip space pos
// c0, c1, c2, c3 - matrix (float4x4)
// r10 = mul(IN.position, ModelViewProjection)
dp4 r10.x, v0, c0
dp4 r10.y, v0, c1
dp4 r10.z, v0, c2
dp4 r10.w, v0, c3

// c14 = 0.000 0.500 255.000 1.000
// c23 = -0.698 5237.045 0.000 0.000
// simple linear fog
max r11.w, r10.w, c14.w
rcp r11, r11.w
mad oFog, c23.y, r11.x, c23.x

// texcoord passthrough
mov oT0.xy, v7

// half vector between vec to camera and vec to light
// c91 - camera pos
// r9 = normalize(cameraPos - vertexPos)
add r9.xyz, c91, -v0
dp3 r11.x, r9, r9
rsq r11.y, r11.x
mul r9, r9.xyzz, r11.y

// halfVec = normalize(dirToLight + dirToCamera)
add r9, r9, r5.xyzz
dp3 r11.x, r9, r9
rsq r11.y, r11.x
mul r9, r9, r11.y

// pass half vec and normal to pixel shader
mov oT1, r9
mov oT4, r0.xyzz

// some weird mumbo-jumbo, have no idea tbh, calc diffuse color (mixes in vertex color)
add r3.xyz, r3.xyzz, c21.xyzz
mul r4.xyz, v5.xyzz, c22.xyzz
mad r3.xyz, r3.xyzz, c20.xyzz, r4.xyzz
add oD0.xyz, r3.xyzz, r3.xyzz

// moves specular color from c27 to output specular color
mov oD1.xyz, c27.xyzz

// finally write the pos
mov oPos, r10

*/
constexpr DWORD vcolorVertexShader[] = {
    0xfffe0101, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000001,
    0x80070000, 0x90e40003, 0x00000008, 0x80010002,
    0x80a40000, 0xa0e40008, 0x00000008, 0x80020002,
    0x80a40000, 0xa0e40009, 0x00000008, 0x80040002,
    0x80a40000, 0xa0e4000a, 0x00000009, 0xe0010002,
    0x90e40000, 0xa0e40005, 0x00000009, 0xe0020002,
    0x90e40000, 0xa0e40004, 0x00000009, 0x80080002,
    0x90e40000, 0xa0e40006, 0x00000001, 0xe0080002,
    0x80ff0002, 0x00000001, 0xe0040003, 0x80ff0002,
    0x00000002, 0x80070005, 0xa0e4000b, 0x91e40000,
    0x00000008, 0x80010001, 0x80e40005, 0x80e40005,
    0x00000007, 0x80010001, 0x80000001, 0x00000005,
    0x80070005, 0x80a40005, 0x80000001, 0x00000004,
    0x80080002, 0x80ff0002, 0xa0000012, 0xa0550012,
    0x0000000b, 0x800f0002, 0x80e40002, 0xa000000e,
    0x00000008, 0x80010003, 0x80e40002, 0xa0e4000f,
    0x00000008, 0x80020003, 0x80e40002, 0xa0e40010,
    0x00000008, 0x80040003, 0x80e40002, 0xa0e40011,
    0x00000008, 0x80010006, 0x80a40000, 0x80e40005,
    0x0000000a, 0x80080002, 0x80ff0002, 0xa0ff000e,
    0x00000005, 0xd0080000, 0x80ff0002, 0x80000006,
    0x00000009, 0x8001000a, 0x90e40000, 0xa0e40000,
    0x00000009, 0x8002000a, 0x90e40000, 0xa0e40001,
    0x00000009, 0x8004000a, 0x90e40000, 0xa0e40002,
    0x00000009, 0x8008000a, 0x90e40000, 0xa0e40003,
    0x0000000b, 0x8008000b, 0x80ff000a, 0xa0ff000e,
    0x00000006, 0x800f000b, 0x80ff000b, 0x00000004,
    0xc00f0001, 0xa0550017, 0x8000000b, 0xa0000017,
    0x00000001, 0xe0030000, 0x90e40007, 0x00000002,
    0x80070009, 0xa0e4005b, 0x91e40000, 0x00000008,
    0x8001000b, 0x80e40009, 0x80e40009, 0x00000007,
    0x8002000b, 0x8000000b, 0x00000005, 0x800f0009,
    0x80a40009, 0x8055000b, 0x00000002, 0x800f0009,
    0x80e40009, 0x80a40005, 0x00000008, 0x8001000b,
    0x80e40009, 0x80e40009, 0x00000007, 0x8002000b,
    0x8000000b, 0x00000005, 0x800f0009, 0x80e40009,
    0x8055000b, 0x00000001, 0xe00f0001, 0x80e40009,
    0x00000001, 0xe00f0004, 0x80a40000, 0x00000002,
    0x80070003, 0x80a40003, 0xa0a40015, 0x00000005,
    0x80070004, 0x90a40005, 0xa0a40016, 0x00000004,
    0x80070003, 0x80a40003, 0xa0a40014, 0x80a40004,
    0x00000002, 0xd0070000, 0x80a40003, 0x80a40003,
    0x00000001, 0xd0070001, 0xa0a4001b, 0x00000001,
    0xc00f0000, 0x80e4000a, 0x0000ffff
};

/*
// Assembled with `psa.exe -h0` from DirectX 8.1b SDK

    ps.1.4
    def c0, 1.0, 0.0, 1.0, 1.0
    mov r0, c0
*/
constexpr DWORD magentaPixelShader[] = {
    0xffff0104, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000051,
    0xa00f0000, 0x3f800000, 0x00000000, 0x3f800000,
    0x3f800000, 0x00000001, 0x800f0000, 0xa0e40000,
    0x0000ffff
};

extern DWORD windowVsHandle;
extern DWORD windowPsHandle;
extern DWORD vcolorVsHandle;

extern DWORD magentaPsHandle;

struct SubSubSomething
{
    unsigned __int16 off_0x00;
    unsigned __int8 off_0x02;
    unsigned __int8 off_0x03;
    unsigned __int16 off_0x04;
    unsigned __int16 off_0x06;
};

struct MapMaterial
{
    DWORD materialColor_0x00;
    DWORD overlayColor_0x04;
    IDirect3DTexture8* tex1;
    IDirect3DTexture8* tex2;
    unsigned __int8 mode_0x10;
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
    MapMaterial* mapMaterial;
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
