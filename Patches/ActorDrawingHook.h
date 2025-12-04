#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"
#include <functional>

struct ModelOffsetTable {
    int field_0;
    int field_4;
    int skeleton_points_offset;
    int skeleton_point_count;
    int skeleton_index_buffer_part_1_offset;
    int field_14;
    int skeleton_index_buffer_part_2_offset;
    int field_1C;
    int materialCount;
    unsigned int materialsOffset;
    int field_2C;
    unsigned int offset_30;
    int field_34;
    unsigned int offset_38;
    int field_3C;
    unsigned int offset_40;
    int field_44;
    int field_48;
    int field_4C;
    int field_50;
};

struct ModelGeometryData {
    UINT numVertices;
    IDirect3DIndexBuffer8* indexBuffer;
    void* vertices;
    void* unknown;
};

struct ModelMaterial {
    int materialLength;
    int reserved0;
    int unkU16Count0;
    unsigned int unkU16Array0Offset;
    int unkU16Count1;
    unsigned int unkU16Array1Offset;
    int unkU16Count2;
    unsigned int unkU16Array2Offset;
    unsigned int samplerStatesOffset;
    char materialType;
    char unkMaterialSubtype;
    char poseId;
    char unkByte0x27;
    int cullMode;
    float unkDiffuseFloat;
    float unkAmbientFloat;
    float specularHighlightScale;
    ModelGeometryData* geomData;
    int reserved2;
    float diffuseX;
    float diffuseR;
    float diffuseG;
    float diffuseB;
    float ambientX;
    float ambientR;
    float ambientG;
    float ambientB;
    float specularX;
    float specularR;
    float specularG;
    float specularB;
    int reserved3;
    int unkIndex;
    int primCount;
    int reserved4;
};

using ActorDrawTopCallbackFn = std::function<bool(ModelOffsetTable*, void*)>;

void RegisterActorDrawTopPrologue(const ActorDrawTopCallbackFn& prologueFn);
void RegisterActorDrawTopEpilogue(const ActorDrawTopCallbackFn& epilogueFn);
