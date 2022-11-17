#pragma once

#include "Patches\Patches.h"
#include "Overlay.h"

class m_IDirect3DDevice8 : public IDirect3DDevice8
{
private:
	LPDIRECT3DDEVICE8 ProxyInterface;
	m_IDirect3D8* m_pD3D;

	std::vector<BYTE> CachedSurfaceData;

	bool isInScene = false;

	bool AnisotropyFlag = (bool)AnisotropicFiltering;
	DWORD MaxAnisotropy = 0;

	bool IsGetFrontBufferCalled = false;
	bool IsSnapshotTextureSet = false;
	bool IsNoiseFilterVertexSet = false;

	BYTE HotelEmployeeElevatorRoomFlag = FALSE;
	bool ReplacedLastRenderTarget = false;

	// Special handling for room 54
	bool IsEnabledForCutscene54 = false;

	// For detecting fullscreen images and turning pillar boxes to black
	bool LastFrameFullscreenImage = false;
	bool DontModifyClear = false;
	float PillarBoxLeft = 0.0f;
	float PillarBoxRight = 0.0f;
	float PillarBoxTop = 0.0f;
	float PillarBoxBottom = 0.0f;

	// Fix for Room 312 pause screen
	bool InPauseMenu = false;
	UINT LastDrawPrimitiveUPStride = 0;

	typedef enum _FLDIMMODE {
		SHADOW_FADING_NONE = 0,
		SHADOW_FADING_IN = 1,
		SHADOW_FADING_OUT = 2,
		SHADOW_REFADING = 3,
	} FLDIMMODE;

	// Shadow fading when using soft shadows
	bool DrawingShadowsFlag = false;
	FLDIMMODE ShadowMode = SHADOW_FADING_NONE;
	BYTE LastFlashlightSwitch = 0;
	DWORD ShadowFadingCounter = 0;
	DWORD ShadowFadingIntensity = 0;	// 0 to 100

	// Xbox shadows to help remove self shadows in certain locations
	bool EnableXboxShadows = false;
	bool shadowVolumeFlag = false;

	// Remove Environment Flicker by skipping frames in certain cutscenes
	bool SkipSceneFlag = false;
	DWORD LastCutsceneID = 0;
	float LastCameraPos = 0;
	float LastJamesPosX = 0;
	DWORD SkipSceneCounter = 0;

	// Remove Effects Flicker by resetting texture on first frame
	DWORD EndSceneCounter = 0;
	bool OverrideTextureLoop = false;
	bool PresentFlag = false;

	IDirect3DTexture8 *pInTexture = nullptr;
	IDirect3DSurface8 *pInSurface = nullptr;
	IDirect3DSurface8 *pInRender = nullptr;

	IDirect3DTexture8 *pShrunkTexture = nullptr;
	IDirect3DSurface8 *pShrunkSurface = nullptr;

	IDirect3DTexture8 *pOutTexture = nullptr;
	IDirect3DSurface8 *pOutSurface = nullptr;

	IDirect3DTexture8 *BlankTexture = nullptr;

	IDirect3DTexture8 *silhouetteTexture = nullptr;
	IDirect3DSurface8 *silhouetteSurface = nullptr;
	IDirect3DSurface8 *silhouetteRender = nullptr;

	IDirect3DTexture8 *pCurrentRenderTexture = nullptr;
	IDirect3DTexture8 *pInitialRenderTexture = nullptr;

	struct SURFACEVECTOR
	{
		m_IDirect3DSurface8 *SourceTarget = nullptr;
		IDirect3DSurface8 *RenderTarget = nullptr;
	};

	// Store a list of surfaces
	std::vector<SURFACEVECTOR> SurfaceVector;

	struct CUSTOMVERTEX
	{
		FLOAT x, y, z, rhw;
	};

	struct CUSTOMVERTEX_DIF
	{
		FLOAT x, y, z, rhw;
		DWORD color;
	};

	struct CUSTOMVERTEX_TEX1
	{
		FLOAT x, y, z, rhw;
		FLOAT u, v;
	};

	struct CUSTOMVERTEX_DIF_TEX1
	{
		FLOAT x, y, z, rhw;
		DWORD color;
		FLOAT u, v;
	};

	struct CUSTOMVERTEX_TEX4
	{
		FLOAT x, y, z, rhw;
		FLOAT u0, v0;
		FLOAT u1, v1;
		FLOAT u2, v2;
		FLOAT u3, v3;
	};

	CUSTOMVERTEX_DIF_TEX1 FullScreenArtifact[6];

	CUSTOMVERTEX_DIF_TEX1 FullScreenFadeout[6]
	{
		{    -0.5f,  -36.5f, 0.01f, 1.0f, 0, 0.0f, 0.0f },
		{ -1919.5f,  -36.5f, 0.01f, 1.0f, 0, 1.0f, 0.0f },
		{ -1919.5f, 1115.5f, 0.01f, 1.0f, 0, 1.0f, 1.0f },
		{    -0.5f,  -36.5f, 0.01f, 1.0f, 0, 0.0f, 0.0f },
		{ -1919.5f, 1115.5f, 0.01f, 1.0f, 0, 1.0f, 1.0f },
		{    -0.5f, 1115.5f, 0.01f, 1.0f, 0, 0.0f, 1.0f }
	};

	CUSTOMVERTEX_TEX1 PillarBoxVertex[6]
	{
		{   -0.5f,  -36.5f, 0.01f, 1.0f, 0.0f, 0.0f },
		{ 1919.5f,  -36.5f, 0.01f, 1.0f, 1.0f, 0.0f },
		{ 1919.5f, 1115.5f, 0.01f, 1.0f, 1.0f, 1.0f },
		{   -0.5f,  -36.5f, 0.01f, 1.0f, 0.0f, 0.0f },
		{ 1919.5f, 1115.5f, 0.01f, 1.0f, 1.0f, 1.0f },
		{   -0.5f, 1115.5f, 0.01f, 1.0f, 0.0f, 1.0f }
	};

	CUSTOMVERTEX_TEX1 FMVVertex[4] = {};

	struct D3DSTATE
	{
		DWORD magFilter;
		DWORD minFilter;
		DWORD colorArg1;
		DWORD alphaArg1;
		DWORD alphaBlendEnable;
		DWORD alphaTestEnable;
		DWORD srcBlend;
		DWORD destBlend;
		IDirect3DBaseTexture8 *stage0 = nullptr;
		DWORD vertexShader;
		IDirect3DVertexBuffer8 *pStream0 = nullptr;
		UINT stream0Stride;
	};

	// Helper functions
	HRESULT DrawSoftShadows();
	void BackupState(D3DSTATE *state);
	void RestoreState(D3DSTATE *state);
	template <typename T>
	void ReleaseInterface(T **ppInterface, UINT ReleaseRefNum = 1);
	bool CheckSilhouetteTexture();
	DWORD GetShadowOpacity();
	DWORD GetShadowIntensity();
	void SetShadowFading();
	void CaptureScreenShot();

public:
	m_IDirect3DDevice8(LPDIRECT3DDEVICE8 pDevice, m_IDirect3D8* pD3D) : ProxyInterface(pDevice), m_pD3D(pD3D)
	{
		Logging::LogDebug() << "Creating device " << __FUNCTION__ << "(" << this << ")";

		ProxyAddressLookupTableD3d8 = new AddressLookupTableD3d8<m_IDirect3DDevice8>(this);

		// Create blank texture for white shader fix
		if (FAILED(ProxyInterface->CreateTexture(1, 1, 1, NULL, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &BlankTexture)))
		{
			BlankTexture = nullptr;
		}
	}
	~m_IDirect3DDevice8()
	{
		Logging::LogDebug() << __FUNCTION__ << "(" << this << ")" << " deleting device!";

		delete ProxyAddressLookupTableD3d8;
	}

	LPDIRECT3DDEVICE8 GetProxyInterface() { return ProxyInterface; }
	AddressLookupTableD3d8<m_IDirect3DDevice8> *ProxyAddressLookupTableD3d8;

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DDevice8 methods ***/
	STDMETHOD(TestCooperativeLevel)(THIS);
	STDMETHOD_(UINT, GetAvailableTextureMem)(THIS);
	STDMETHOD(ResourceManagerDiscardBytes)(THIS_ DWORD Bytes);
	STDMETHOD(GetDirect3D)(THIS_ IDirect3D8** ppD3D8);
	STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS8* pCaps);
	STDMETHOD(GetDisplayMode)(THIS_ D3DDISPLAYMODE* pMode);
	STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters);
	STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot, UINT YHotSpot, IDirect3DSurface8* pCursorBitmap);
	STDMETHOD_(void, SetCursorPosition)(THIS_ UINT XScreenSpace, UINT YScreenSpace, DWORD Flags);
	STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow);
	STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain8** pSwapChain);
	STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
	STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	STDMETHOD(GetBackBuffer)(THIS_ UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8** ppBackBuffer);
	STDMETHOD(GetRasterStatus)(THIS_ D3DRASTER_STATUS* pRasterStatus);
	STDMETHOD_(void, SetGammaRamp)(THIS_ DWORD Flags, CONST D3DGAMMARAMP* pRamp);
	STDMETHOD_(void, GetGammaRamp)(THIS_ D3DGAMMARAMP* pRamp);
	STDMETHOD(CreateTexture)(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8** ppTexture);
	STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture8** ppVolumeTexture);
	STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8** ppCubeTexture);
	STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8** ppVertexBuffer);
	STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8** ppIndexBuffer);
	STDMETHOD(CreateRenderTarget)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8** ppSurface);
	STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface);
	STDMETHOD(CreateImageSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8** ppSurface);
	STDMETHOD(CopyRects)(THIS_ IDirect3DSurface8* pSourceSurface, CONST RECT* pSourceRectsArray, UINT cRects, IDirect3DSurface8* pDestinationSurface, CONST POINT* pDestPointsArray);
	STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface8* pSourceSurface, IDirect3DSurface8* pDestSurface);
	STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture8* pSourceTexture, IDirect3DBaseTexture8* pDestinationTexture);
	STDMETHOD(GetFrontBuffer)(THIS_ IDirect3DSurface8* pDestSurface);
	STDMETHOD(GetFrontBufferFromGDI)(THIS_ BYTE* lpBuffer, size_t Size);
	STDMETHOD(GetFrontBufferFromDirectX)(THIS_ BYTE* lpBuffer, size_t Size);
	STDMETHOD(FakeGetFrontBuffer)(THIS_ IDirect3DSurface8* pDestSurface);
	STDMETHOD(SetRenderTarget)(THIS_ IDirect3DSurface8* pRenderTarget, IDirect3DSurface8* pNewZStencil);
	STDMETHOD(GetRenderTarget)(THIS_ IDirect3DSurface8** ppRenderTarget);
	STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface8** ppZStencilSurface);
	STDMETHOD(BeginScene)(THIS);
	STDMETHOD(EndScene)(THIS);
	STDMETHOD(Clear)(THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
	STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix);
	STDMETHOD(MultiplyTransform)(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
	STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT8* pViewport);
	STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT8* pViewport);
	STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL8* pMaterial);
	STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL8* pMaterial);
	STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT8* pLight);
	STDMETHOD(GetLight)(THIS_ DWORD Index, D3DLIGHT8*);
	STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable);
	STDMETHOD(GetLightEnable)(THIS_ DWORD Index, BOOL* pEnable);
	STDMETHOD(SetClipPlane)(THIS_ DWORD Index, CONST float* pPlane);
	STDMETHOD(GetClipPlane)(THIS_ DWORD Index, float* pPlane);
	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value);
	STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD* pValue);
	STDMETHOD(BeginStateBlock)(THIS);
	STDMETHOD(EndStateBlock)(THIS_ DWORD* pToken);
	STDMETHOD(ApplyStateBlock)(THIS_ DWORD Token);
	STDMETHOD(CaptureStateBlock)(THIS_ DWORD Token);
	STDMETHOD(DeleteStateBlock)(THIS_ DWORD Token);
	STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type, DWORD* pToken);
	STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS8* pClipStatus);
	STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS8* pClipStatus);
	STDMETHOD(GetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture8** ppTexture);
	STDMETHOD(SetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture8* pTexture);
	STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue);
	STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses);
	STDMETHOD(GetInfo)(THIS_ DWORD DevInfoID, void* pDevInfoStruct, DWORD DevInfoStructSize);
	STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber, CONST PALETTEENTRY* pEntries);
	STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber, PALETTEENTRY* pEntries);
	STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber);
	STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber);
	STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer8* pDestBuffer, DWORD Flags);
	STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pDeclaration, CONST DWORD* pFunction, DWORD* pHandle, DWORD Usage);
	STDMETHOD(SetVertexShader)(THIS_ DWORD Handle);
	STDMETHOD(GetVertexShader)(THIS_ DWORD* pHandle);
	STDMETHOD(DeleteVertexShader)(THIS_ DWORD Handle);
	STDMETHOD(SetVertexShaderConstant)(THIS_ DWORD Register, CONST void* pConstantData, DWORD ConstantCount);
	STDMETHOD(GetVertexShaderConstant)(THIS_ DWORD Register, void* pConstantData, DWORD ConstantCount);
	STDMETHOD(GetVertexShaderDeclaration)(THIS_ DWORD Handle, void* pData, DWORD* pSizeOfData);
	STDMETHOD(GetVertexShaderFunction)(THIS_ DWORD Handle, void* pData, DWORD* pSizeOfData);
	STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer8* pStreamData, UINT Stride);
	STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer8** ppStreamData, UINT* pStride);
	STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex);
	STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer8** ppIndexData, UINT* pBaseVertexIndex);
	STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction, DWORD* pHandle);
	STDMETHOD(SetPixelShader)(THIS_ DWORD Handle);
	STDMETHOD(GetPixelShader)(THIS_ DWORD* pHandle);
	STDMETHOD(DeletePixelShader)(THIS_ DWORD Handle);
	STDMETHOD(SetPixelShaderConstant)(THIS_ DWORD Register, CONST void* pConstantData, DWORD ConstantCount);
	STDMETHOD(GetPixelShaderConstant)(THIS_ DWORD Register, void* pConstantData, DWORD ConstantCount);
	STDMETHOD(GetPixelShaderFunction)(THIS_ DWORD Handle, void* pData, DWORD* pSizeOfData);
	STDMETHOD(DrawRectPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo);
	STDMETHOD(DrawTriPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo);
	STDMETHOD(DeletePatch)(THIS_ UINT Handle);

	// Extra functions
	void m_IDirect3DDevice8::AddSurfaceToVector(m_IDirect3DSurface8 *pSourceTarget, IDirect3DSurface8 *pRenderTarget);

};
