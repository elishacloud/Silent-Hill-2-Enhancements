#pragma once
#include <string>
#include "Wrappers\d3d8\d3d8wrapper.h"
#include "IUnknownPtr.h"

class ModelGLTF {
public:
    enum class VertexType {
        PosNormalTexcoord,
        PosNormalColorTexcoord
    };

    struct Vertex_PNT {
        D3DXVECTOR3 pos;
        D3DXVECTOR3 normal;
        D3DXVECTOR2 uv;
    };
    static_assert(sizeof(Vertex_PNT) == 32);

    struct Vertex_PNCT {
        D3DXVECTOR3 pos;
        D3DXVECTOR3 normal;
        uint32_t    color;
        D3DXVECTOR2 uv;
    };
    static_assert(sizeof(Vertex_PNCT) == 36);

    struct Vertex_Skin {
        uint8_t  bones[4];
        float    weights[4];
    };
    static_assert(sizeof(Vertex_Skin) == 20);

    struct Section {
        uint32_t numVertices;
        uint32_t numIndices;
        uint32_t vbOffset;
        uint32_t ibOffset;
        uint32_t textureIdx;
        uint32_t skinningOffset;
    };

    struct Mesh {
        std::vector<Section> sections;
    };

    struct SceneNode {
        int              meshIdx = -1;      // index into mMeshes
        int              animTrackIdx = -1; // index into mAnimTracks
        int              skinIdx = -1;      // index into mSkins
        D3DXVECTOR3      position;
        D3DXQUATERNION   rotation;
        D3DXVECTOR3      scale;
        D3DXMATRIX       xform;
        std::vector<int> children;
    };

    struct AnimTrack {
        int                         target; // index into mSceneNodes
        std::vector<D3DXVECTOR3>    offsets;
        std::vector<D3DXQUATERNION> rotations;
        std::vector<D3DXVECTOR3>    scales;
    };

    struct Skin {
        std::vector<int>        joints;
        std::vector<D3DXMATRIX> invBindMatrices;
    };

    struct Texture {
        IUnknownPtr<IDirect3DTexture8>  texture;
        BOOL                            transparent;
    };

public:
    ModelGLTF() = delete;
    ModelGLTF(const VertexType vtype, const bool uploadToGPU);
    ~ModelGLTF();

    bool                                LoadFromFile(const std::string& filePath, IDirect3DDevice8* device);
    void                                Update(const float deltaInSeconds, const D3DXMATRIX& globalXForm, float* customTimer = nullptr);
    HRESULT                             Draw(IDirect3DDevice8* device, BOOL enableTransparency = FALSE);

    size_t                              GetNumMeshes() const;
    const Mesh&                         GetMesh(const size_t idx) const;

    const size_t                        GetNumCPUVertices() const;
    const size_t                        GetNumCPUIndices() const;
    const uint8_t*                      GetCPUVertices() const;
    const uint16_t*                     GetCPUIndices() const;

    const uint8_t*                      GetCPUXFormedVertices() const;

    IDirect3DVertexBuffer8*             GetGPUVertices() const;
    IDirect3DIndexBuffer8*              GetGPUIndices() const;

    size_t                              GetNumTextures() const;
    IDirect3DTexture8*                  GetTexture(const size_t idx) const;

    const SceneNode*                    GetRootNode() const;
    size_t                              GetNumSceneNodes() const;
    const SceneNode*                    GetSceneNode(const size_t idx) const;
    const D3DXMATRIX&                   GetAnimatedSceneNodeXForm(const size_t idx) const;

private:
    void                                CollectAnimation(const void* gltfModel);
    void                                CollectSkinning(const void* gltfModel);
    void                                RecursiveBuildChildrenXForm(const int nodeIdx, const D3DXMATRIX& parentXForm);

    template <typename T>
    void                                XFormVertices(const D3DXMATRIX& globalXForm, T* dstVertices);

private:
    VertexType                          mVertexType;
    bool                                mUploadToGPU;

    std::vector<uint8_t>                mVertices;
    std::vector<uint16_t>               mIndices;

    IUnknownPtr<IDirect3DVertexBuffer8> mVB;
    IUnknownPtr<IDirect3DIndexBuffer8>  mIB;

    std::vector<uint8_t>                mXFormedVertices;

    std::vector<Mesh>                   mMeshes;

    using TexturesArray = std::vector<Texture>;
    TexturesArray                       mTextures;

    // scene
    SceneNode                           mRootNode;
    std::vector<SceneNode>              mSceneNodes;

    // animation
    std::vector<float>                  mAnimTimeline;
    std::vector<AnimTrack>              mAnimTracks;
    std::vector<D3DXMATRIX>             mAnimatedNodesXForms;
    float                               mAnimTime;

    // skinning
    std::vector<Vertex_Skin>            mSkinVertices;
    std::vector<Skin>                   mSkins;
};
