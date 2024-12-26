#include "ModelGLTF.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#define TINYGLTF_USE_CPP14
#include <tinygltf/tiny_gltf.h>

#include "GfxUtils.h"
#include <filesystem>

static bool ExtensionEqual(const char* filePath, const char* extToCompare) {
    const size_t len = strnlen_s(filePath, 1024);
    return tolower(filePath[len - 3]) == tolower(extToCompare[0]) &&
           tolower(filePath[len - 2]) == tolower(extToCompare[1]) &&
           tolower(filePath[len - 1]) == tolower(extToCompare[2]);
}

template <typename K, typename T>
static bool map_contains(const std::map<K, T>& m, const K& key) {
    return m.find(key) != m.end();
}

static uint32_t EncodeVertexColor(const float r, const float g, const float b, const float a) {
    const uint8_t rb = static_cast<uint8_t>(std::clamp(r * 255.0f, 0.0f, 255.0f));
    const uint8_t gb = static_cast<uint8_t>(std::clamp(g * 255.0f, 0.0f, 255.0f));
    const uint8_t bb = static_cast<uint8_t>(std::clamp(b * 255.0f, 0.0f, 255.0f));
    const uint8_t ab = static_cast<uint8_t>(std::clamp(a * 255.0f, 0.0f, 255.0f));

    return D3DCOLOR_ARGB(ab, rb, gb, bb);
}

static void DecodeGLTFVertexColor(ModelGLTF::Vertex_PNCT& v, const tinygltf::Accessor* colorAcc, const tinygltf::BufferView* colorView, const tinygltf::Buffer* colorBuff, const size_t idx) {
    const uint8_t* colorPtrRaw = colorBuff->data.data() + colorAcc->byteOffset + colorView->byteOffset;

    switch (colorAcc->componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            const uint8_t* colorPtr = colorPtrRaw + (idx * 4);
            v.color = EncodeVertexColor(colorPtr[0] / 255.0f, colorPtr[1] / 255.0f, colorPtr[2] / 255.0f, colorPtr[3] / 255.0f);
        } break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            const uint16_t* colorPtr = reinterpret_cast<const uint16_t*>(colorPtrRaw) + (idx * 4);
            v.color = EncodeVertexColor(colorPtr[0] / 65535.0f, colorPtr[1] / 65535.0f, colorPtr[2] / 65535.0f, colorPtr[3] / 65535.0f);
        } break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT: {
            const float* colorPtr = reinterpret_cast<const float*>(colorPtrRaw) + (idx * 4);
            v.color = EncodeVertexColor(colorPtr[0], colorPtr[1], colorPtr[2], colorPtr[3]);
        } break;
    }
}

//TINYGLTF_COMPONENT_TYPE_FLOAT
template <bool normalized>
static size_t UnpackComponent(const void* srcData, float& dstData, const int componentType) {
    switch (componentType) {
        case TINYGLTF_COMPONENT_TYPE_BYTE: {
            if constexpr (normalized) {
                dstData = fmaxf(*reinterpret_cast<const int8_t*>(srcData) / 127.0f, -1.0f);
            } else {
                dstData = *reinterpret_cast<const int8_t*>(srcData);
            }
            return 1;
        } break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            if constexpr (normalized) {
                dstData = *reinterpret_cast<const uint8_t*>(srcData) / 255.0f;
            } else {
                dstData = *reinterpret_cast<const uint8_t*>(srcData);
            }
            return 1;
        } break;
        case TINYGLTF_COMPONENT_TYPE_SHORT: {
            if constexpr (normalized) {
                dstData = fmaxf(*reinterpret_cast<const int16_t*>(srcData) / 32767.0f, -1.0f);
            } else {
                dstData = *reinterpret_cast<const int16_t*>(srcData);
            }
            return 2;
        } break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            if constexpr (normalized) {
                dstData = *reinterpret_cast<const uint16_t*>(srcData) / 65535.0f;
            } else {
                dstData = *reinterpret_cast<const uint16_t*>(srcData);
            }
            return 2;
        } break;

        default: {
            dstData = *reinterpret_cast<const float*>(srcData);
            return 4;
        }
    }
}

template <size_t vecSize, bool normalized>
static size_t UnpackVec(const void* srcData, float* dstData, const int componentType) {
    size_t offset = UnpackComponent<normalized>(srcData, dstData[0], componentType);
    offset += UnpackComponent<normalized>(reinterpret_cast<const uint8_t*>(srcData) + offset, dstData[1], componentType);
    if constexpr (vecSize > 2) {
        offset += UnpackComponent<normalized>(reinterpret_cast<const uint8_t*>(srcData) + offset, dstData[2], componentType);
        if constexpr (vecSize > 3) {
            offset += UnpackComponent<normalized>(reinterpret_cast<const uint8_t*>(srcData) + offset, dstData[3], componentType);
        }
    }

    return offset;
}


static void ReadGLTFTransformation(const tinygltf::Node& srcNode, ModelGLTF::SceneNode& dstNode) {
    if (srcNode.matrix.size() == 16) {
        D3DXMATRIX result = {};
        for (size_t y = 0; y < 4; ++y) {
            for (size_t x = 0; x < 4; ++x) {
                // transpose on read (from OpenGL -> Direct3D)
                reinterpret_cast<float*>(&result)[x + y * 4] = static_cast<float>(srcNode.matrix[y + x * 4]);
            }
        }
        assert(false && "Implement matrix decompose !!!");
        //D3DXMatrixDecompose()
    } else {
        if (srcNode.translation.size() == 3) {
            dstNode.position.x = static_cast<float>(srcNode.translation[0]);
            dstNode.position.y = static_cast<float>(srcNode.translation[1]);
            dstNode.position.z = static_cast<float>(srcNode.translation[2]);
        } else {
            dstNode.position = { 0.0f, 0.0f, 0.0f };
        }

        if (srcNode.rotation.size() == 4) {
            dstNode.rotation.x = static_cast<float>(srcNode.rotation[0]);
            dstNode.rotation.y = static_cast<float>(srcNode.rotation[1]);
            dstNode.rotation.z = static_cast<float>(srcNode.rotation[2]);
            dstNode.rotation.w = static_cast<float>(srcNode.rotation[3]);
        } else {
            D3DXQuaternionIdentity(&dstNode.rotation);
        }

        if (srcNode.scale.size() == 3) {
            dstNode.scale.x = static_cast<float>(srcNode.scale[0]);
            dstNode.scale.y = static_cast<float>(srcNode.scale[1]);
            dstNode.scale.z = static_cast<float>(srcNode.scale[2]);
        } else {
            dstNode.scale = { 1.0f, 1.0f, 1.0f };
        }
    }
}

static void MakeNodeXForm(const D3DXVECTOR3& position, const D3DXQUATERNION& rotation, const D3DXVECTOR3& scale, D3DXMATRIX& xform) {
    D3DXMATRIX S, R, T;
    D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
    D3DXMatrixRotationQuaternion(&R, &rotation);
    D3DXMatrixTranslation(&T, position.x, position.y, position.z);

    xform = S * R * T;
}


ModelGLTF::ModelGLTF(const VertexType vtype, const bool uploadToGPU)
    : mVertexType(vtype)
    , mUploadToGPU(uploadToGPU)
    , mAnimTime(0.0f)
{
}
ModelGLTF::~ModelGLTF() {
}

bool ModelGLTF::LoadFromFile(const std::string& filePath, IDirect3DDevice8* device) {
    std::error_code errorCode{};
    if (!std::filesystem::exists(filePath, errorCode)) {
        return false;
    }

    const bool isBinary = ExtensionEqual(filePath.c_str(), "glb");

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string errors, warnings;

    loader.SetImageLoader([](tinygltf::Image* image, const int /*image_idx*/, std::string* /*err*/,
        std::string* /*warn*/, int /*req_width*/, int /*req_height*/,
        const unsigned char* bytes, int size, void* /*user_data*/)->bool {
            // just read image as is for now
            image->width = image->height = image->component = -1;
            image->bits = image->pixel_type = -1;
            image->image.resize(static_cast<size_t>(size));
            std::copy(bytes, bytes + size, image->image.begin());
            return true;
        }, nullptr);

    bool loaded = false;
    if (isBinary) {
        loaded = loader.LoadBinaryFromFile(&model, &errors, &warnings, filePath);
    } else {
        loaded = loader.LoadASCIIFromFile(&model, &errors, &warnings, filePath);
    }

    if (!loaded) {
        return false;
    }

    // now collect our meshes
    mMeshes.resize(model.meshes.size());

    mVertices.clear();
    mIndices.clear();

    const size_t vertexSize = (mVertexType == VertexType::PosNormalTexcoord) ? sizeof(Vertex_PNT) : sizeof(Vertex_PNCT);

    for (size_t meshIdx = 0, numMeshes = model.meshes.size(); meshIdx < numMeshes; ++meshIdx) {
        const tinygltf::Mesh& srcMesh = model.meshes[meshIdx];
        Mesh& dstMesh = mMeshes[meshIdx];
        dstMesh.sections.resize(srcMesh.primitives.size());

        for (size_t i = 0, n = srcMesh.primitives.size(); i < n; ++i) {
            const tinygltf::Primitive& prim = srcMesh.primitives[i];
            assert(prim.mode == TINYGLTF_MODE_TRIANGLES);

            Section& section = dstMesh.sections[i];
            section.skinningOffset = ~0u;

            const int posIdx = map_contains(prim.attributes, std::string("POSITION")) ? prim.attributes.at("POSITION") : -1;
            const int normIdx = map_contains(prim.attributes, std::string("NORMAL")) ? prim.attributes.at("NORMAL") : -1;
            const int colorIdx = map_contains(prim.attributes, std::string("COLOR_0")) ? prim.attributes.at("COLOR_0") : -1;
            const int uvIdx = map_contains(prim.attributes, std::string("TEXCOORD_0")) ? prim.attributes.at("TEXCOORD_0") : -1;

            // skinning
            const int bonesIdx = map_contains(prim.attributes, std::string("JOINTS_0")) ? prim.attributes.at("JOINTS_0") : -1;
            const int weightsIdx = map_contains(prim.attributes, std::string("WEIGHTS_0")) ? prim.attributes.at("WEIGHTS_0") : -1;

            const tinygltf::Accessor& posAcc = model.accessors[posIdx];
            const tinygltf::Accessor& normAcc = model.accessors[normIdx];
            const tinygltf::Accessor* colorAcc = (colorIdx != -1) ? &model.accessors[colorIdx] : nullptr;
            const tinygltf::Accessor& uvAcc = model.accessors[uvIdx];

            const tinygltf::Accessor* bonesAcc = (bonesIdx != -1) ? &model.accessors[bonesIdx] : nullptr;
            const tinygltf::Accessor* weightsAcc = (weightsIdx != -1) ? &model.accessors[weightsIdx] : nullptr;

            assert(posAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && posAcc.type == TINYGLTF_TYPE_VEC3);
            assert(normAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && normAcc.type == TINYGLTF_TYPE_VEC3);
            if (colorAcc) {
                assert((colorAcc->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT || colorAcc->componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT || colorAcc->componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) && colorAcc->type == TINYGLTF_TYPE_VEC4);
            }
            assert(uvAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && uvAcc.type == TINYGLTF_TYPE_VEC2);

            assert(posAcc.count == normAcc.count && posAcc.count == uvAcc.count);

            if (bonesAcc && weightsAcc) {
                assert((bonesAcc->componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE || bonesAcc->componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) && bonesAcc->type == TINYGLTF_TYPE_VEC4);
                assert(weightsAcc->type == TINYGLTF_TYPE_VEC4);

                assert(posAcc.count == bonesAcc->count && posAcc.count == weightsAcc->count);
            }

            const tinygltf::BufferView& posView = model.bufferViews[posAcc.bufferView];
            const tinygltf::BufferView& normView = model.bufferViews[normAcc.bufferView];
            const tinygltf::BufferView* colorView = colorAcc ? &model.bufferViews[colorAcc->bufferView] : nullptr;
            const tinygltf::BufferView& uvView = model.bufferViews[uvAcc.bufferView];

            const tinygltf::BufferView* bonesView = bonesAcc ? &model.bufferViews[bonesAcc->bufferView] : nullptr;
            const tinygltf::BufferView* weightsView = weightsAcc ? &model.bufferViews[weightsAcc->bufferView] : nullptr;

            const tinygltf::Buffer& posBuff = model.buffers[posView.buffer];
            const tinygltf::Buffer& normBuff = model.buffers[normView.buffer];
            const tinygltf::Buffer* colorBuff = colorView ? &model.buffers[colorView->buffer] : nullptr;
            const tinygltf::Buffer& uvBuff = model.buffers[uvView.buffer];

            const tinygltf::Buffer* bonesBuff = bonesView ? &model.buffers[bonesView->buffer] : nullptr;
            const tinygltf::Buffer* weightsBuff = weightsView ? &model.buffers[weightsView->buffer] : nullptr;

            const size_t vertsOffset = mVertices.size() / vertexSize;
            mVertices.resize(mVertices.size() + (posAcc.count * vertexSize));
            for (size_t j = 0; j < posAcc.count; ++j) {
                const float* posPtr = reinterpret_cast<const float*>(posBuff.data.data() + posAcc.byteOffset + posView.byteOffset) + (j * 3);
                const float* normPtr = reinterpret_cast<const float*>(normBuff.data.data() + normAcc.byteOffset + normView.byteOffset) + (j * 3);
                const float* uvPtr = reinterpret_cast<const float*>(uvBuff.data.data() + uvAcc.byteOffset + uvView.byteOffset) + (j * 2);

                if (mVertexType == VertexType::PosNormalTexcoord) {
                    Vertex_PNT& v = reinterpret_cast<Vertex_PNT*>(mVertices.data())[vertsOffset + j];
                    v.pos.x     = posPtr[0];  v.pos.y    = posPtr[1];  v.pos.z    = posPtr[2];
                    v.normal.x  = normPtr[0]; v.normal.y = normPtr[1]; v.normal.z = normPtr[2];
                    v.uv.x      = uvPtr[0];   v.uv.y     = uvPtr[1];
                } else {
                    Vertex_PNCT& v = reinterpret_cast<Vertex_PNCT*>(mVertices.data())[vertsOffset + j];
                    v.pos.x     = posPtr[0];  v.pos.y    = posPtr[1];  v.pos.z    = posPtr[2];
                    v.normal.x  = normPtr[0]; v.normal.y = normPtr[1]; v.normal.z = normPtr[2];
                    if (colorAcc) {
                        DecodeGLTFVertexColor(v, colorAcc, colorView, colorBuff, j);
                    } else {
                        v.color = ~0u;
                    }
                    v.uv.x      = uvPtr[0];   v.uv.y     = uvPtr[1];
                }
            }

            if (bonesBuff && weightsBuff) {
                const size_t skinVertsOffset = mSkinVertices.size();
                mSkinVertices.resize(mSkinVertices.size() + posAcc.count);
                const bool bonesAreU8 = bonesAcc->componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
                const uint8_t* weightsPtr = weightsBuff->data.data() + weightsAcc->byteOffset + weightsView->byteOffset;
                for (size_t j = 0, weightsOff = 0; j < posAcc.count; ++j) {
                    const uint16_t* bonesPtrU16 = reinterpret_cast<const uint16_t*>(bonesBuff->data.data() + bonesAcc->byteOffset + bonesView->byteOffset) + (j * 4);
                    const uint8_t* bonesPtrU8 = bonesBuff->data.data() + bonesAcc->byteOffset + bonesView->byteOffset + (j * 4);

                    Vertex_Skin& vs = mSkinVertices[skinVertsOffset + j];
                    vs.bones[0] = bonesAreU8 ? bonesPtrU8[0] : static_cast<uint8_t>(bonesPtrU16[0]);
                    vs.bones[1] = bonesAreU8 ? bonesPtrU8[1] : static_cast<uint8_t>(bonesPtrU16[1]);
                    vs.bones[2] = bonesAreU8 ? bonesPtrU8[2] : static_cast<uint8_t>(bonesPtrU16[2]);
                    vs.bones[3] = bonesAreU8 ? bonesPtrU8[3] : static_cast<uint8_t>(bonesPtrU16[3]);
                    weightsOff += UnpackVec<4, true>(weightsPtr + weightsOff, vs.weights, weightsAcc->componentType);
                }

                section.skinningOffset = static_cast<uint32_t>(skinVertsOffset);
            }

            const tinygltf::Accessor& idxAcc = model.accessors[prim.indices];
            assert((idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT || idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) && idxAcc.type == TINYGLTF_TYPE_SCALAR);

            const tinygltf::BufferView& idxView = model.bufferViews[idxAcc.bufferView];
            const tinygltf::Buffer& idxBuff = model.buffers[idxView.buffer];

            const size_t indicesOffset = mIndices.size();
            mIndices.resize(mIndices.size() + idxAcc.count);
            if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                std::memcpy(mIndices.data() + indicesOffset, idxBuff.data.data() + idxAcc.byteOffset + idxView.byteOffset, idxAcc.count * sizeof(uint16_t));
            } else {
                const uint32_t* srcIndices = reinterpret_cast<const uint32_t*>(idxBuff.data.data() + idxAcc.byteOffset + idxView.byteOffset);
                for (size_t j = 0; j < idxAcc.count; ++j) {
                    mIndices[indicesOffset + j] = static_cast<uint16_t>(srcIndices[j]);
                }
            }

            section.numVertices = static_cast<uint32_t>(posAcc.count);
            section.numIndices = static_cast<uint32_t>(idxAcc.count);
            section.vbOffset = static_cast<uint32_t>(vertsOffset);
            section.ibOffset = static_cast<uint32_t>(indicesOffset);
            const int albedoIdx = model.materials[prim.material].pbrMetallicRoughness.baseColorTexture.index;
            section.textureIdx = static_cast<uint32_t>(model.textures[albedoIdx].source);
        }
    }

    if (mUploadToGPU) {
        const DWORD vbSizeBytes = static_cast<DWORD>(mVertices.size());
        HRESULT hr = device->CreateVertexBuffer(vbSizeBytes, 0u, 0u, D3DPOOL_MANAGED, mVB.ReleaseAndGetAddressOf());
        if (FAILED(hr)) {
            return false;
        } else {
            BYTE* lockedPtr = nullptr;
            mVB->Lock(0, vbSizeBytes, &lockedPtr, D3DLOCK_DISCARD);
            std::memcpy(lockedPtr, mVertices.data(), vbSizeBytes);
            mVB->Unlock();
        }

        const DWORD ibSizeBytes = static_cast<DWORD>(mIndices.size() * sizeof(uint16_t));
        hr = device->CreateIndexBuffer(ibSizeBytes, 0u, D3DFMT_INDEX16, D3DPOOL_MANAGED, mIB.ReleaseAndGetAddressOf());
        if (FAILED(hr)) {
            return false;
        } else {
            BYTE* lockedPtr = nullptr;
            mIB->Lock(0, ibSizeBytes, &lockedPtr, D3DLOCK_DISCARD);
            std::memcpy(lockedPtr, mIndices.data(), ibSizeBytes);
            mIB->Unlock();
        }

        mVertices.clear();
        mIndices.clear();
    } else {
        mXFormedVertices.resize(mVertices.size());
        std::memcpy(mXFormedVertices.data(), mVertices.data(), mVertices.size());
    }

    // now collect textures
    mTextures.resize(model.images.size());
    for (size_t i = 0, n = model.images.size(); i < n; ++i) {
        const tinygltf::Image& img = model.images[i];
        IUnknownPtr<IDirect3DTexture8>& texture = mTextures[i];

        HRESULT hr = GfxCreateTextureFromFileInMem(device, (void*)img.image.data(), img.image.size(), texture.ReleaseAndGetAddressOf());
        if (FAILED(hr)) {
            return false;
        }
    }

    mSceneNodes.resize(model.nodes.size());
    for (size_t i = 0, end = model.nodes.size(); i < end; ++i) {
        const tinygltf::Node& srcNode = model.nodes[i];
        SceneNode& dstNode = mSceneNodes[i];

        dstNode.meshIdx = srcNode.mesh;
        dstNode.skinIdx = srcNode.skin;
        ReadGLTFTransformation(srcNode, dstNode);
        dstNode.children = srcNode.children;

        MakeNodeXForm(dstNode.position, dstNode.rotation, dstNode.scale, dstNode.xform);
    }

    assert(!model.scenes.empty());
    const tinygltf::Scene& scene = *model.scenes.begin();
    mRootNode.position = { 0.0f, 0.0f, 0.0f };
    D3DXQuaternionIdentity(&mRootNode.rotation);
    mRootNode.scale = { 1.0f, 1.0f, 1.0f };
    mRootNode.children = scene.nodes;

    mAnimatedNodesXForms.resize(model.nodes.size());

    if (!model.animations.empty()) {
        this->CollectAnimation(&model);
    }

    if (!model.skins.empty()) {
        this->CollectSkinning(&model);
    }

    mAnimTime = 0.0f;

    for (size_t i = 0, end = mSceneNodes.size(); i < end; ++i) {
        const SceneNode& node = mSceneNodes[i];
        mAnimatedNodesXForms[i] = node.xform;
    }

    return true;
}

void ModelGLTF::Update(const float deltaInSeconds, const D3DXMATRIX& globalXForm, float* customTimer) {
    if (!mAnimTimeline.empty()) {
        float& timer = (customTimer == nullptr) ? mAnimTime : *customTimer;

        timer += deltaInSeconds;
        while (timer > mAnimTimeline.back()) {
            timer -= mAnimTimeline.back();
        }

        auto it = std::upper_bound(mAnimTimeline.begin(), mAnimTimeline.end(), timer);
        const size_t idxB = std::distance(mAnimTimeline.begin(), it);
        const size_t idxA = idxB > 0 ? idxB - 1 : 0;

        const float t = (timer - mAnimTimeline[idxA]) / (mAnimTimeline[idxB] - mAnimTimeline[idxA]);

        for (AnimTrack& track : mAnimTracks) {
            D3DXVECTOR3 offset;
            if (!track.offsets.empty()) {
                D3DXVec3Lerp(&offset, &track.offsets[idxA], &track.offsets[idxB], t);
            } else {
                offset = mSceneNodes[track.target].position;
            }

            D3DXQUATERNION rotation;
            D3DXQuaternionIdentity(&rotation);
            if (!track.rotations.empty()) {
                D3DXQuaternionSlerp(&rotation, &track.rotations[idxA], &track.rotations[idxB], t);
            } else {
                rotation = mSceneNodes[track.target].rotation;
            }

            D3DXVECTOR3 scale;
            if (!track.scales.empty()) {
                D3DXVec3Lerp(&scale, &track.scales[idxA], &track.scales[idxB], t);
            } else {
                scale = mSceneNodes[track.target].scale;
            }

            MakeNodeXForm(offset, rotation, scale, mAnimatedNodesXForms[track.target]);
        }
    }

    D3DXMATRIX rootXForm;
    D3DXMatrixIdentity(&rootXForm);
    for (const int idx : mRootNode.children) {
        this->RecursiveBuildChildrenXForm(idx, rootXForm);
    }

    if (!mUploadToGPU) {
        if (mVertexType == VertexType::PosNormalTexcoord) {
            this->XFormVertices<Vertex_PNT>(globalXForm, reinterpret_cast<Vertex_PNT*>(mXFormedVertices.data()));
        } else {
            this->XFormVertices<Vertex_PNCT>(globalXForm, reinterpret_cast<Vertex_PNCT*>(mXFormedVertices.data()));
        }
    }
}

HRESULT ModelGLTF::Draw(IDirect3DDevice8* device) {
    const DWORD vertexSize = (mVertexType == VertexType::PosNormalTexcoord) ? sizeof(Vertex_PNT) : sizeof(Vertex_PNCT);

    HRESULT hr = S_OK;

    IUnknownPtr<IDirect3DBaseTexture8> savedTexture;
    hr = device->GetTexture(0, savedTexture.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    const uint8_t* vertices = mXFormedVertices.data();
    const uint16_t* indices = mIndices.data();

    uint32_t lastTextureIdx = ~0u;
    for (const ModelGLTF::Mesh& mesh : mMeshes) {
        for (const ModelGLTF::Section& section : mesh.sections) {
            if (section.textureIdx != lastTextureIdx) {
                device->SetTexture(0, mTextures[section.textureIdx].GetPtr());
                lastTextureIdx = section.textureIdx;
            }

            hr = device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, section.numVertices, section.numIndices / 3u, indices + section.ibOffset, D3DFMT_INDEX16, vertices + section.vbOffset * vertexSize, vertexSize);
            if (FAILED(hr)) {
                return hr;
            }
        }
    }

    hr = device->SetTexture(0, savedTexture.GetPtr());
    return hr;
}

size_t ModelGLTF::GetNumMeshes() const {
    return mMeshes.size();
}

const ModelGLTF::Mesh& ModelGLTF::GetMesh(const size_t idx) const {
    return mMeshes[idx];
}

const size_t ModelGLTF::GetNumCPUVertices() const {
    const size_t vertexSize = (mVertexType == VertexType::PosNormalTexcoord) ? sizeof(Vertex_PNT) : sizeof(Vertex_PNCT);
    return mVertices.size() / vertexSize;
}

const size_t ModelGLTF::GetNumCPUIndices() const {
    return mIndices.size();
}

const uint8_t* ModelGLTF::GetCPUVertices() const {
    return mVertices.data();
}

const uint16_t* ModelGLTF::GetCPUIndices() const {
    return mIndices.data();
}

const uint8_t* ModelGLTF::GetCPUXFormedVertices() const {
    return mXFormedVertices.data();
}

IDirect3DVertexBuffer8* ModelGLTF::GetGPUVertices() const {
    return mVB.GetPtr();
}

IDirect3DIndexBuffer8* ModelGLTF::GetGPUIndices() const {
    return mIB.GetPtr();
}

size_t ModelGLTF::GetNumTextures() const {
    return mTextures.size();
}

IDirect3DTexture8* ModelGLTF::GetTexture(const size_t idx) const {
    return mTextures[idx].GetPtr();
}

const ModelGLTF::SceneNode* ModelGLTF::GetRootNode() const {
    return &mRootNode;
}

size_t ModelGLTF::GetNumSceneNodes() const {
    return mSceneNodes.size();
}

const ModelGLTF::SceneNode* ModelGLTF::GetSceneNode(const size_t idx) const {
    return &mSceneNodes[idx];
}

const D3DXMATRIX& ModelGLTF::GetAnimatedSceneNodeXForm(const size_t idx) const {
    return mAnimatedNodesXForms[idx];
}


void ModelGLTF::CollectAnimation(const void* gltfModel) {
    const tinygltf::Model* model = reinterpret_cast<const tinygltf::Model*>(gltfModel);

    const tinygltf::Animation& anim = model->animations.front();
    // iOrange: I assume only one input sampler for simplicity
    const int timelineSampler = anim.samplers[0].input;

    const tinygltf::Accessor& timelineAcc = model->accessors[timelineSampler];
    const tinygltf::BufferView& timelineView = model->bufferViews[timelineAcc.bufferView];
    const tinygltf::Buffer& timelineBuff = model->buffers[timelineView.buffer];

    assert(timelineAcc.type == TINYGLTF_TYPE_SCALAR && timelineAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
    const float* timelinePtr = reinterpret_cast<const float*>(timelineBuff.data.data() + timelineAcc.byteOffset + timelineView.byteOffset);
    mAnimTimeline.resize(timelineAcc.count);
    std::memcpy(mAnimTimeline.data(), timelinePtr, timelineAcc.count * sizeof(float));

    for (const tinygltf::AnimationChannel& channel : anim.channels) {
        const tinygltf::AnimationSampler& sampler = anim.samplers[channel.sampler];
        assert(sampler.input == timelineSampler);
        const int targetNode = channel.target_node;

        auto it = std::find_if(mAnimTracks.begin(), mAnimTracks.end(), [targetNode](const AnimTrack& track)->bool {
            return track.target == targetNode;
        });

        AnimTrack* track;
        if (it == mAnimTracks.end()) {
            mSceneNodes[targetNode].animTrackIdx = static_cast<int>(mAnimTracks.size());

            mAnimTracks.push_back({});
            track = &mAnimTracks.back();
            track->target = targetNode;
        } else {
            track = &(*it);
        }

        const tinygltf::Accessor& channelAcc = model->accessors[sampler.output];
        const tinygltf::BufferView& channelView = model->bufferViews[channelAcc.bufferView];
        const tinygltf::Buffer& channelBuff = model->buffers[channelView.buffer];

        assert(channelAcc.count == timelineAcc.count);

        if (channel.target_path == "rotation") {
            assert(channelAcc.type == TINYGLTF_TYPE_VEC4);
            assert(track->rotations.empty());

            track->rotations.resize(channelAcc.count);
            const uint8_t* srcData = channelBuff.data.data() + channelAcc.byteOffset + channelView.byteOffset;
            for (size_t i = 0, offset = 0; i < channelAcc.count; ++i) {
                offset += UnpackVec<4, true>(srcData + offset, track->rotations[i], channelAcc.componentType);
            }
        } else if (channel.target_path == "translation") {
            assert(channelAcc.type == TINYGLTF_TYPE_VEC3 && channelAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
            assert(track->offsets.empty());

            const D3DXVECTOR3* offsetsPtr = reinterpret_cast<const D3DXVECTOR3*>(channelBuff.data.data() + channelAcc.byteOffset + channelView.byteOffset);
            track->offsets.resize(channelAcc.count);
            std::memcpy(track->offsets.data(), offsetsPtr, channelAcc.count * sizeof(D3DXVECTOR3));
        } else if (channel.target_path == "scale") {
            assert(channelAcc.type == TINYGLTF_TYPE_VEC3 && channelAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
            assert(track->scales.empty());

            const D3DXVECTOR3* scalesPtr = reinterpret_cast<const D3DXVECTOR3*>(channelBuff.data.data() + channelAcc.byteOffset + channelView.byteOffset);
            track->scales.resize(channelAcc.count);
            std::memcpy(track->scales.data(), scalesPtr, channelAcc.count * sizeof(D3DXVECTOR3));
        }
    }
}

void ModelGLTF::CollectSkinning(const void* gltfModel) {
    const tinygltf::Model* model = reinterpret_cast<const tinygltf::Model*>(gltfModel);

    const size_t numSkins = model->skins.size();
    mSkins.resize(numSkins);

    for (size_t skinIdx = 0; skinIdx < numSkins; ++skinIdx) {
        const tinygltf::Skin& srcSkin = model->skins.front();
        Skin& dstSkin = mSkins[skinIdx];

        dstSkin.joints = srcSkin.joints;

        const tinygltf::Accessor& invBindMatsAcc = model->accessors[srcSkin.inverseBindMatrices];
        const tinygltf::BufferView& invBindMatsView = model->bufferViews[invBindMatsAcc.bufferView];
        const tinygltf::Buffer& invBindMatsBuff = model->buffers[invBindMatsView.buffer];

        assert(invBindMatsAcc.type == TINYGLTF_TYPE_MAT4 && invBindMatsAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

        dstSkin.invBindMatrices.resize(invBindMatsAcc.count);

        const float* invBindMatsPtr = reinterpret_cast<const float*>(invBindMatsBuff.data.data() + invBindMatsAcc.byteOffset + invBindMatsView.byteOffset);
        std::memcpy(dstSkin.invBindMatrices.data(), invBindMatsPtr, invBindMatsAcc.count * sizeof(D3DXMATRIX));
    }
}

void ModelGLTF::RecursiveBuildChildrenXForm(const int nodeIdx, const D3DXMATRIX& parentXForm) {
    const SceneNode& node = mSceneNodes[nodeIdx];
    if (node.animTrackIdx < 0) {
        mAnimatedNodesXForms[nodeIdx] = node.xform;
    }
    mAnimatedNodesXForms[nodeIdx] *= parentXForm;
    for (const int childIdx : node.children) {
        this->RecursiveBuildChildrenXForm(childIdx, mAnimatedNodesXForms[nodeIdx]);
    }
}

template <typename T>
void ModelGLTF::XFormVertices(const D3DXMATRIX& globalXForm, T* dstVertices) {
    const T* srcVertices = reinterpret_cast<const T*>(mVertices.data());

    std::vector<D3DXMATRIX> jointXForms;
    const D3DXVECTOR3* vpos, * vnorm;
    D3DXVECTOR3 skinnedPos, skinnedNorm;

    for (size_t i = 0, numNodes = mSceneNodes.size(); i < numNodes; ++i) {
        const ModelGLTF::SceneNode& node = mSceneNodes[i];
        if (node.meshIdx < 0) {
            continue;
        }

        const D3DXMATRIX& nodeXForm = mAnimatedNodesXForms[i];

        D3DXMATRIX fullXForm = nodeXForm * globalXForm;
        D3DXMATRIX fullXFormI, fullXFormTI;
        D3DXMatrixInverse(&fullXFormI, nullptr, &fullXForm);
        D3DXMatrixTranspose(&fullXFormTI, &fullXFormI);

        const ModelGLTF::Mesh& mesh = mMeshes[node.meshIdx];
        const Skin* skin = (node.skinIdx >= 0) ? &mSkins[node.skinIdx] : nullptr;

        if (skin) {
            const size_t numJoints = skin->invBindMatrices.size();
            jointXForms.resize((std::max)(jointXForms.size(), numJoints));

            D3DXMATRIX nodeXFormI;
            D3DXMatrixInverse(&nodeXFormI, nullptr, &nodeXForm);

            for (size_t j = 0; j < numJoints; ++j) {
                jointXForms[j] = skin->invBindMatrices[j] * mAnimatedNodesXForms[skin->joints[j]] * nodeXFormI;
            }
        }

        for (const ModelGLTF::Section& section : mesh.sections) {
            for (size_t j = 0; j < section.numVertices; ++j) {
                if (skin) {
                    D3DXMATRIX skinMat;
                    std::memset(&skinMat, 0, sizeof(skinMat));
                    const Vertex_Skin& skinVert = mSkinVertices[section.skinningOffset + j];
                    for (size_t k = 0; k < 4; ++k) {
                        if (skinVert.weights[k] > 0.0f) {
                            skinMat += (jointXForms[skinVert.bones[k]] * skinVert.weights[k]);
                        }
                    }

                    D3DXVECTOR4 temp;
                    D3DXVec3Transform(&temp, &srcVertices[section.vbOffset + j].pos, &skinMat);
                    D3DXVec3TransformNormal(&skinnedNorm, &srcVertices[section.vbOffset + j].normal, &skinMat);

                    skinnedPos.x = temp.x;
                    skinnedPos.y = temp.y;
                    skinnedPos.z = temp.z;

                    vpos = &skinnedPos;
                    vnorm = &skinnedNorm;
                } else {
                    vpos = &srcVertices[section.vbOffset + j].pos;
                    vnorm = &srcVertices[section.vbOffset + j].normal;
                }

                D3DXVECTOR4 xformedV;
                D3DXVECTOR3 xformedN;
                D3DXVec3Transform(&xformedV, vpos, &fullXForm);
                D3DXVec3TransformNormal(&xformedN, vnorm, &fullXFormTI);

                dstVertices[section.vbOffset + j].pos.x = xformedV.x;
                dstVertices[section.vbOffset + j].pos.y = xformedV.y;
                dstVertices[section.vbOffset + j].pos.z = xformedV.z;
                dstVertices[section.vbOffset + j].normal = xformedN;
            }
        }
    }
}
