#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"

extern DWORD windowVsHandle;
extern DWORD windowPsHandle;
extern DWORD vcolorVsHandle;

extern DWORD hospitalDoorVsHandle;
extern DWORD hospitalDoorPsHandles[4];

HRESULT DrawFlashlightReflection(LPDIRECT3DDEVICE8 ProxyInterface, D3DPRIMITIVETYPE Type, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);

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
// Assembled with `vsa.exe -h0` from DirectX 8.1b SDK

vs.1.1

// v0 - position
// v3 - normal
// v7 - texcoord

// sh2ee regs:
// c20..c23 - VP
// c92..c95 - invViewMat

// c90 - flashLightPos

mov r7, v0
mov r8, v3

// move vertex back to world space
dp4 r0.x, r7, c92
dp4 r0.y, r7, c93
dp4 r0.z, r7, c94
dp4 r0.w, r7, c95

// and to clip space
dp4 r1.x, r0, c20
dp4 r1.y, r0, c21
dp4 r1.z, r0, c22
dp4 r1.w, r0, c23

// linear fog
max r3.x, r1.w, c0.x
rcp r3.x, r3.x
mad oFog, r3.x, c46.y, c46.x

// output position
mov oPos, r1

// project pos onto flashlight texture
dp4 oT2.x, r0, c5
dp4 oT2.y, r0, c4
dp4 r2.w, r0, c6
mov oT2.w, r2.w
mov oT3.z, r2.w

// transform normal to ... something space
dp3 r1.x, r8, c47
dp3 r1.y, r8, c48
dp3 r1.z, r8, c49
dp3 r1.w, r8, c50

// have no idea, but looks like transposed transform
max r1, r1, c0.x
mul r2, r1.x, c55
mad r2, r1.y, c56, r2
mad r2, r1.z, c57, r2
mad r2, r1.w, c58, r2

// directions from something to vertex
add r3, r7, -c65
add r4, r7, -c67

// and normalize them
dp3 r9.x, r3, r3
dp3 r9.y, r4, r4
rsq r9.x, r9.x
rsq r9.y, r9.y
mul r3.xyz, r3.xyzz, r9.x
mul r4.xyz, r4.xyzz, r9.y

dp3 r11.x, r8, r3
dp3 r11.y, r8, r4
mov r10.xy, c63.xyyy
mad r10.xy, r10.xyyy, r9.xyyy, c64.xyyy
min r10.xy, r10.xyyy, c0.y
min r11.xy, r11.xyyy, c0.y
max r10.xy, r10.xyyy, c0.x
max r11.xy, r11.xyyy, c0.x
mul r9.xy, r10.xyyy, r11.xyyy
mad r2, r9.x, c66, r2
mad r2, r9.y, c68, r2
mul r2, r2, c43
add r2, r2, c44
mul oD0, r2, c1.x

// pass specular color to pixel shader
mov oD1.xyz, c27.xyzz

// some direction that we calculate the diffuse lighing against
add r3, r7, -c77
dp3 r1.x, r3, r3
rsq r1.x, r1.x
mul oT5, r3.xyzz, r1.x

// texcoord passthrough
mov oT0.xy, v7

// calculate dir to flashlight
// r5 = normalize(flashLightPos - vertexPos)
//add r5.xyz, c90, -r7
add r5.xyz, r7, -c90
dp3 r1.x, r5, r5
rsq r1.x, r1.x
mul r5.xyz, r5.xyzz, r1.x

// r9 = normalize(cameraPos - vertexPos)
//mov r9.xyz, -r7
mov r9.xyz, r7
dp3 r1.x, r9, r9
rsq r1.y, r1.x
mul r9, r9.xyzz, r1.y

// halfVec = normalize(dirToLight + dirToCamera)
add r9, r9, r5.xyzz
dp3 r11.x, r9, r9
rsq r11.y, r11.x
mul r9, r9, r11.y

// pass half vec and normal to pixel shader
mov oT1, r9
mov oT4, r8


*/
constexpr DWORD hospitalDoorVertexShader[] = {
    0xfffe0101, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000001,
    0x800f0007, 0x90e40000, 0x00000001, 0x800f0008,
    0x90e40003, 0x00000009, 0x80010000, 0x80e40007,
    0xa0e4005c, 0x00000009, 0x80020000, 0x80e40007,
    0xa0e4005d, 0x00000009, 0x80040000, 0x80e40007,
    0xa0e4005e, 0x00000009, 0x80080000, 0x80e40007,
    0xa0e4005f, 0x00000009, 0x80010001, 0x80e40000,
    0xa0e40014, 0x00000009, 0x80020001, 0x80e40000,
    0xa0e40015, 0x00000009, 0x80040001, 0x80e40000,
    0xa0e40016, 0x00000009, 0x80080001, 0x80e40000,
    0xa0e40017, 0x0000000b, 0x80010003, 0x80ff0001,
    0xa0000000, 0x00000006, 0x80010003, 0x80000003,
    0x00000004, 0xc00f0001, 0x80000003, 0xa055002e,
    0xa000002e, 0x00000001, 0xc00f0000, 0x80e40001,
    0x00000009, 0xe0010002, 0x80e40000, 0xa0e40005,
    0x00000009, 0xe0020002, 0x80e40000, 0xa0e40004,
    0x00000009, 0x80080002, 0x80e40000, 0xa0e40006,
    0x00000001, 0xe0080002, 0x80ff0002, 0x00000001,
    0xe0040003, 0x80ff0002, 0x00000008, 0x80010001,
    0x80e40008, 0xa0e4002f, 0x00000008, 0x80020001,
    0x80e40008, 0xa0e40030, 0x00000008, 0x80040001,
    0x80e40008, 0xa0e40031, 0x00000008, 0x80080001,
    0x80e40008, 0xa0e40032, 0x0000000b, 0x800f0001,
    0x80e40001, 0xa0000000, 0x00000005, 0x800f0002,
    0x80000001, 0xa0e40037, 0x00000004, 0x800f0002,
    0x80550001, 0xa0e40038, 0x80e40002, 0x00000004,
    0x800f0002, 0x80aa0001, 0xa0e40039, 0x80e40002,
    0x00000004, 0x800f0002, 0x80ff0001, 0xa0e4003a,
    0x80e40002, 0x00000002, 0x800f0003, 0x80e40007,
    0xa1e40041, 0x00000002, 0x800f0004, 0x80e40007,
    0xa1e40043, 0x00000008, 0x80010009, 0x80e40003,
    0x80e40003, 0x00000008, 0x80020009, 0x80e40004,
    0x80e40004, 0x00000007, 0x80010009, 0x80000009,
    0x00000007, 0x80020009, 0x80550009, 0x00000005,
    0x80070003, 0x80a40003, 0x80000009, 0x00000005,
    0x80070004, 0x80a40004, 0x80550009, 0x00000008,
    0x8001000b, 0x80e40008, 0x80e40003, 0x00000008,
    0x8002000b, 0x80e40008, 0x80e40004, 0x00000001,
    0x8003000a, 0xa054003f, 0x00000004, 0x8003000a,
    0x8054000a, 0x80540009, 0xa0540040, 0x0000000a,
    0x8003000a, 0x8054000a, 0xa0550000, 0x0000000a,
    0x8003000b, 0x8054000b, 0xa0550000, 0x0000000b,
    0x8003000a, 0x8054000a, 0xa0000000, 0x0000000b,
    0x8003000b, 0x8054000b, 0xa0000000, 0x00000005,
    0x80030009, 0x8054000a, 0x8054000b, 0x00000004,
    0x800f0002, 0x80000009, 0xa0e40042, 0x80e40002,
    0x00000004, 0x800f0002, 0x80550009, 0xa0e40044,
    0x80e40002, 0x00000005, 0x800f0002, 0x80e40002,
    0xa0e4002b, 0x00000002, 0x800f0002, 0x80e40002,
    0xa0e4002c, 0x00000005, 0xd00f0000, 0x80e40002,
    0xa0000001, 0x00000001, 0xd0070001, 0xa0a4001b,
    0x00000002, 0x800f0003, 0x80e40007, 0xa1e4004d,
    0x00000008, 0x80010001, 0x80e40003, 0x80e40003,
    0x00000007, 0x80010001, 0x80000001, 0x00000005,
    0xe00f0005, 0x80a40003, 0x80000001, 0x00000001,
    0xe0030000, 0x90e40007, 0x00000002, 0x80070005,
    0x80e40007, 0xa1e4005a, 0x00000008, 0x80010001,
    0x80e40005, 0x80e40005, 0x00000007, 0x80010001,
    0x80000001, 0x00000005, 0x80070005, 0x80a40005,
    0x80000001, 0x00000001, 0x80070009, 0x80e40007,
    0x00000008, 0x80010001, 0x80e40009, 0x80e40009,
    0x00000007, 0x80020001, 0x80000001, 0x00000005,
    0x800f0009, 0x80a40009, 0x80550001, 0x00000002,
    0x800f0009, 0x80e40009, 0x80a40005, 0x00000008,
    0x8001000b, 0x80e40009, 0x80e40009, 0x00000007,
    0x8002000b, 0x8000000b, 0x00000005, 0x800f0009,
    0x80e40009, 0x8055000b, 0x00000001, 0xe00f0001,
    0x80e40009, 0x00000001, 0xe00f0004, 0x80e40008,
    0x0000ffff
};

/*
// how to compile
// first we preprocess it to 4 different shaders with Visual Studio's cl:
// cl /EP /C /DSTAGE_0=1 shader_src.hlsl > shader_stage0.hlsl
// cl /EP /C /DSTAGE_1=1 shader_src.hlsl > shader_stage1.hlsl
// cl /EP /C /DSTAGE_2=1 shader_src.hlsl > shader_stage2.hlsl
// cl /EP /C /DSTAGE_3=1 shader_src.hlsl > shader_stage3.hlsl
//
// then we compile them using psa.exe from DirectX 8 SDK
// psa -h0 shader_stage0.hlsl
// psa -h0 shader_stage1.hlsl
// psa -h0 shader_stage2.hlsl
// psa -h0 shader_stage3.hlsl

  ps.1.4
  def c0, 0.5, 0.5, 0.5, 0.5
  def c4, 0.0, 0.0, 0.0, 0.0
  def c5, 0.0, 0.0, 1.0, 0.0

  // sample flashlight func texture
  texld r2, t2_dw.xyw

  // load vector "depth in light's coordinates"
  texcrd r0.xyz, t3

  // load half vector and normal from texcoords
  texcrd r4.xyz, t1
  texcrd r5.xyz, t4

  // check if looking towards pixel
  dp3 r0, r0, c5

  // normalization approximation by Nvidia (using normalization cube somehow looks like shit)
  // half vector
  mov r4.w, c4
  mul r1, r4, c0
  dp3 r3, r4, r4
  mad r4, 1-r3, r1, r4

  // angle between half vector and normal
  dp3_sat r1, r4, r5

  // if looking towards -> take flashlight, else - take black (c4)
  cnd r4.xyz, r0.x, r2, c4

  // diffuse lighting
  mov r3, c1

phase

  // load albedo
  texld r0, t0
  // load specular func
  texld r1, r1
  // load diffuse light dir
  texcrd r2.xyz, t5

  // flashlight calc
  dp3_sat r3, r2, r3_bx2
  // diffuse light ?
  dp3_sat r2.xyz, r2, r5
+ sub r3.w, r3, c2.w

#if defined(STAGE_3)
  mul_x2_sat r3.xyz, r3.w, r2
#else
  mul_sat r3.xyz, r3.w, r2
#endif

  // specular * albedo alpha
  mul r2.xyz, r1.w, 1-r0.w
  // and mask by flashlight's func
  mul r2.xyz, r2, r4

  // flashligh * c2 + diffuse color ?
#if defined(STAGE_3) || defined(STAGE_2)
  mad_x4_sat r3.xyz, r3, c2, v0
#elif defined(STAGE_1)
  mad_x2_sat r3.xyz, r3, c2, v0
#else // STAGE_0
  mad_sat r3.xyz, r3, c2, v0
#endif

  // now all this * albedo
  mul r3.xyz, r3, r0
  // copy alpha from albedo
+ mov r3.w, r0.w

  // specular * spec_color + diffuse
  mad r0.xyz, r2, v1, r3

*/
constexpr DWORD hospitalDoorPixelShader_stage0[] = {
    0xffff0104, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000051,
    0xa00f0000, 0x3f000000, 0x3f000000, 0x3f000000,
    0x3f000000, 0x00000051, 0xa00f0004, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000051,
    0xa00f0005, 0x00000000, 0x00000000, 0x3f800000,
    0x00000000, 0x00000042, 0x800f0002, 0xbaf40002,
    0x00000040, 0x80070000, 0xb0e40003, 0x00000040,
    0x80070004, 0xb0e40001, 0x00000040, 0x80070005,
    0xb0e40004, 0x00000008, 0x800f0000, 0x80e40000,
    0xa0e40005, 0x00000001, 0x80080004, 0xa0e40004,
    0x00000005, 0x800f0001, 0x80e40004, 0xa0e40000,
    0x00000008, 0x800f0003, 0x80e40004, 0x80e40004,
    0x00000004, 0x800f0004, 0x86e40003, 0x80e40001,
    0x80e40004, 0x00000008, 0x801f0001, 0x80e40004,
    0x80e40005, 0x00000050, 0x80070004, 0x80000000,
    0x80e40002, 0xa0e40004, 0x00000001, 0x800f0003,
    0xa0e40001, 0x0000fffd, 0x00000042, 0x800f0000,
    0xb0e40000, 0x00000042, 0x800f0001, 0x80e40001,
    0x00000040, 0x80070002, 0xb0e40005, 0x00000008,
    0x801f0003, 0x80e40002, 0x84e40003, 0x00000008,
    0x80170002, 0x80e40002, 0x80e40005, 0x40000003,
    0x80080003, 0x80e40003, 0xa0ff0002, 0x00000005,
    0x80170003, 0x80ff0003, 0x80e40002, 0x00000005,
    0x80070002, 0x80ff0001, 0x86ff0000, 0x00000005,
    0x80070002, 0x80e40002, 0x80e40004, 0x00000004,
    0x80170003, 0x80e40003, 0xa0e40002, 0x90e40000,
    0x00000005, 0x80070003, 0x80e40003, 0x80e40000,
    0x40000001, 0x80080003, 0x80ff0000, 0x00000004,
    0x80070000, 0x80e40002, 0x90e40001, 0x80e40003,
    0x0000ffff
};
constexpr DWORD hospitalDoorPixelShader_stage1[] = {
    0xffff0104, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000051,
    0xa00f0000, 0x3f000000, 0x3f000000, 0x3f000000,
    0x3f000000, 0x00000051, 0xa00f0004, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000051,
    0xa00f0005, 0x00000000, 0x00000000, 0x3f800000,
    0x00000000, 0x00000042, 0x800f0002, 0xbaf40002,
    0x00000040, 0x80070000, 0xb0e40003, 0x00000040,
    0x80070004, 0xb0e40001, 0x00000040, 0x80070005,
    0xb0e40004, 0x00000008, 0x800f0000, 0x80e40000,
    0xa0e40005, 0x00000001, 0x80080004, 0xa0e40004,
    0x00000005, 0x800f0001, 0x80e40004, 0xa0e40000,
    0x00000008, 0x800f0003, 0x80e40004, 0x80e40004,
    0x00000004, 0x800f0004, 0x86e40003, 0x80e40001,
    0x80e40004, 0x00000008, 0x801f0001, 0x80e40004,
    0x80e40005, 0x00000050, 0x80070004, 0x80000000,
    0x80e40002, 0xa0e40004, 0x00000001, 0x800f0003,
    0xa0e40001, 0x0000fffd, 0x00000042, 0x800f0000,
    0xb0e40000, 0x00000042, 0x800f0001, 0x80e40001,
    0x00000040, 0x80070002, 0xb0e40005, 0x00000008,
    0x801f0003, 0x80e40002, 0x84e40003, 0x00000008,
    0x80170002, 0x80e40002, 0x80e40005, 0x40000003,
    0x80080003, 0x80e40003, 0xa0ff0002, 0x00000005,
    0x80170003, 0x80ff0003, 0x80e40002, 0x00000005,
    0x80070002, 0x80ff0001, 0x86ff0000, 0x00000005,
    0x80070002, 0x80e40002, 0x80e40004, 0x00000004,
    0x81170003, 0x80e40003, 0xa0e40002, 0x90e40000,
    0x00000005, 0x80070003, 0x80e40003, 0x80e40000,
    0x40000001, 0x80080003, 0x80ff0000, 0x00000004,
    0x80070000, 0x80e40002, 0x90e40001, 0x80e40003,
    0x0000ffff
};
constexpr DWORD hospitalDoorPixelShader_stage2[] = {
    0xffff0104, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000051,
    0xa00f0000, 0x3f000000, 0x3f000000, 0x3f000000,
    0x3f000000, 0x00000051, 0xa00f0004, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000051,
    0xa00f0005, 0x00000000, 0x00000000, 0x3f800000,
    0x00000000, 0x00000042, 0x800f0002, 0xbaf40002,
    0x00000040, 0x80070000, 0xb0e40003, 0x00000040,
    0x80070004, 0xb0e40001, 0x00000040, 0x80070005,
    0xb0e40004, 0x00000008, 0x800f0000, 0x80e40000,
    0xa0e40005, 0x00000001, 0x80080004, 0xa0e40004,
    0x00000005, 0x800f0001, 0x80e40004, 0xa0e40000,
    0x00000008, 0x800f0003, 0x80e40004, 0x80e40004,
    0x00000004, 0x800f0004, 0x86e40003, 0x80e40001,
    0x80e40004, 0x00000008, 0x801f0001, 0x80e40004,
    0x80e40005, 0x00000050, 0x80070004, 0x80000000,
    0x80e40002, 0xa0e40004, 0x00000001, 0x800f0003,
    0xa0e40001, 0x0000fffd, 0x00000042, 0x800f0000,
    0xb0e40000, 0x00000042, 0x800f0001, 0x80e40001,
    0x00000040, 0x80070002, 0xb0e40005, 0x00000008,
    0x801f0003, 0x80e40002, 0x84e40003, 0x00000008,
    0x80170002, 0x80e40002, 0x80e40005, 0x40000003,
    0x80080003, 0x80e40003, 0xa0ff0002, 0x00000005,
    0x80170003, 0x80ff0003, 0x80e40002, 0x00000005,
    0x80070002, 0x80ff0001, 0x86ff0000, 0x00000005,
    0x80070002, 0x80e40002, 0x80e40004, 0x00000004,
    0x82170003, 0x80e40003, 0xa0e40002, 0x90e40000,
    0x00000005, 0x80070003, 0x80e40003, 0x80e40000,
    0x40000001, 0x80080003, 0x80ff0000, 0x00000004,
    0x80070000, 0x80e40002, 0x90e40001, 0x80e40003,
    0x0000ffff
};
constexpr DWORD hospitalDoorPixelShader_stage3[] = {
    0xffff0104, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000051,
    0xa00f0000, 0x3f000000, 0x3f000000, 0x3f000000,
    0x3f000000, 0x00000051, 0xa00f0004, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000051,
    0xa00f0005, 0x00000000, 0x00000000, 0x3f800000,
    0x00000000, 0x00000042, 0x800f0002, 0xbaf40002,
    0x00000040, 0x80070000, 0xb0e40003, 0x00000040,
    0x80070004, 0xb0e40001, 0x00000040, 0x80070005,
    0xb0e40004, 0x00000008, 0x800f0000, 0x80e40000,
    0xa0e40005, 0x00000001, 0x80080004, 0xa0e40004,
    0x00000005, 0x800f0001, 0x80e40004, 0xa0e40000,
    0x00000008, 0x800f0003, 0x80e40004, 0x80e40004,
    0x00000004, 0x800f0004, 0x86e40003, 0x80e40001,
    0x80e40004, 0x00000008, 0x801f0001, 0x80e40004,
    0x80e40005, 0x00000050, 0x80070004, 0x80000000,
    0x80e40002, 0xa0e40004, 0x00000001, 0x800f0003,
    0xa0e40001, 0x0000fffd, 0x00000042, 0x800f0000,
    0xb0e40000, 0x00000042, 0x800f0001, 0x80e40001,
    0x00000040, 0x80070002, 0xb0e40005, 0x00000008,
    0x801f0003, 0x80e40002, 0x84e40003, 0x00000008,
    0x80170002, 0x80e40002, 0x80e40005, 0x40000003,
    0x80080003, 0x80e40003, 0xa0ff0002, 0x00000005,
    0x81170003, 0x80ff0003, 0x80e40002, 0x00000005,
    0x80070002, 0x80ff0001, 0x86ff0000, 0x00000005,
    0x80070002, 0x80e40002, 0x80e40004, 0x00000004,
    0x82170003, 0x80e40003, 0xa0e40002, 0x90e40000,
    0x00000005, 0x80070003, 0x80e40003, 0x80e40000,
    0x40000001, 0x80080003, 0x80ff0000, 0x00000004,
    0x80070000, 0x80e40002, 0x90e40001, 0x80e40003,
    0x0000ffff
};
