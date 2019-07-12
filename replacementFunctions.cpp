#include "replacementFunctions.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"
#include "Logging\Logging.h"

int& screenWidth_932AD0 = *reinterpret_cast<int*>(0x932AD0);
int& screenHeight_932AD4 = *reinterpret_cast<int*>(0x932AD4);

void(__cdecl *sub_445670)() = (void(__cdecl *)())0x445670;
void(__cdecl *sub_442AC0)(uint32_t*, uint32_t*, uint32_t*, uint32_t*) = (void(__cdecl *)(uint32_t*, uint32_t*, uint32_t*, uint32_t*))0x442AC0;
uint32_t*(__cdecl *sub_4F5490)() = (uint32_t*(__cdecl *)())0x4F5490;

IDirect3DTexture8 *pInTexture = NULL;
IDirect3DSurface8 *pInSurface = NULL, *pBackBuffer = NULL, *pStencilBuffer = NULL;

IDirect3DTexture8 *pOutTexture = NULL;
IDirect3DSurface8 *pOutSurface = NULL;

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
	IDirect3DBaseTexture8 *stage0;
	DWORD vertexShader;
};

constexpr int SHADOW_SCALE = 4;
constexpr int SHADOW_OPACITY = 128;

void backupState(D3DSTATE *state);
void restoreState(D3DSTATE *state);

//sh2pc.00445380 (NA v1.0)
void parentShadowFunc(uint32_t arg1)
{
	IDirect3DDevice8 *pD3d8Device = *reinterpret_cast<IDirect3DDevice8**>(0xA32894);

	uint32_t(*sub_4F5450)(void) = (uint32_t(*)(void))0x4F5450;
	uint32_t& pWidth = *reinterpret_cast<uint32_t*>(0x932AD0);
	pWidth = sub_4F5450();

	uint32_t(*sub_4F5460)(void) = (uint32_t(*)(void))0x4F5460;
	uint32_t& pHeight = *reinterpret_cast<uint32_t*>(0x932AD4);
	pHeight = sub_4F5460();

	uint32_t(*sub_4F5490)(void) = (uint32_t(*)(void))0x4F5490;
	uint32_t local_1 = sub_4F5490();

	uint32_t& global_1 = *reinterpret_cast<uint32_t*>(0x932AD8);
	global_1 = pWidth;

	uint32_t& global_2 = *reinterpret_cast<uint32_t*>(0x932ADC);
	global_2 = pHeight;

	uint32_t& global_3 = *reinterpret_cast<uint32_t*>(0x932AF0);
	uint32_t& global_4 = *reinterpret_cast<uint32_t*>(0x932AF4);
	uint32_t& global_5 = *reinterpret_cast<uint32_t*>(0x932AF8);
	global_3 = 0;
	global_4 = 0;
	global_5 = 0;

	pD3d8Device->SetRenderState(D3DRS_FOGENABLE, FALSE);
	pD3d8Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	pD3d8Device->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR);
	pD3d8Device->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_INCR);
	pD3d8Device->SetRenderState(D3DRS_STENCILREF, 0);
	pD3d8Device->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
	pD3d8Device->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
	pD3d8Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
	pD3d8Device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	pD3d8Device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	pD3d8Device->SetRenderState(D3DRS_ZENABLE, TRUE);
	pD3d8Device->SetTexture(0, 0);
	pD3d8Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	//Not sure about this
	struct ZBIAS_VALUES
	{
		uint32_t zbias1;
		uint32_t zbias2;
		uint32_t unk2;
		uint32_t unk3;
	};

	uint32_t& index = *reinterpret_cast<uint32_t*>(0xA333C4);
	ZBIAS_VALUES *zbias_values = (ZBIAS_VALUES *)0x6BCE48;
	pD3d8Device->SetRenderState(D3DRS_ZBIAS, zbias_values[index].zbias2);

	uint32_t(*sub_441EA0)(void) = (uint32_t(*)(void))0x441EA0;
	sub_441EA0();

	void(*sub_4451D0)(uint32_t) = (void(*)(uint32_t))0x4451D0;
	sub_4451D0(arg1);

	//void(*sub_442B70)(void) = (void(*)(void))0x442B70;
	//sub_442B70();
	drawDynamicShadows();

	void(*sub_442010)(void) = (void(*)(void))0x442010;
	sub_442010();

	pD3d8Device->SetRenderState(D3DRS_FOGENABLE, TRUE);
	pD3d8Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pD3d8Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NEVER);
	pD3d8Device->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	pD3d8Device->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	pD3d8Device->SetRenderState(D3DRS_STENCILREF, 0);
	pD3d8Device->SetRenderState(D3DRS_STENCILMASK, 0);
	pD3d8Device->SetRenderState(D3DRS_STENCILWRITEMASK, 0);
	pD3d8Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

	pD3d8Device->SetRenderState(D3DRS_ZBIAS, zbias_values[index].zbias1);
}

//sh2pc.00442B70 (NA v1.0)
void __cdecl drawDynamicShadows()
{
	IDirect3DDevice8 *pD3d8Device = *reinterpret_cast<IDirect3DDevice8**>(0xA32894);

	sub_445670(); //Sets up some globals

	pD3d8Device->SetRenderState(D3DRS_ZENABLE, FALSE);
	pD3d8Device->SetRenderState(D3DRS_STENCILENABLE, TRUE);
	pD3d8Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pD3d8Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	pD3d8Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	pD3d8Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
	pD3d8Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	pD3d8Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pD3d8Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	pD3d8Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	pD3d8Device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	pD3d8Device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	pD3d8Device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	pD3d8Device->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	pD3d8Device->SetRenderState(D3DRS_STENCILREF, 129);
	pD3d8Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);

	//Original function calls these, but never uses returned values, no side-effects?
	uint32_t v4, v3;
	sub_442AC0(&v4, &v3, nullptr, nullptr);
	sub_4F5490();

	CUSTOMVERTEX shadowRect[] =
	{
		{0.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(128, 0, 0, 0)},
		{0.0f, (float)screenHeight_932AD4, 0.0f, 1.0f, D3DCOLOR_ARGB(128, 0, 0, 0)},
		{(float)screenWidth_932AD0, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(128, 0, 0, 0)},
		{(float)screenWidth_932AD0, (float)screenHeight_932AD4, 0.0f, 1.0f, D3DCOLOR_ARGB(128, 0, 0, 0)}
	};

	pD3d8Device->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	softShadows(screenWidth_932AD0, screenHeight_932AD4); //pD3d8Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, shadowRect, 20);

	pD3d8Device->SetRenderState(D3DRS_ZENABLE, TRUE);
	pD3d8Device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	pD3d8Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	pD3d8Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_ZERO);
	pD3d8Device->SetRenderState(D3DRS_FOGENABLE, TRUE);
	pD3d8Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void softShadows(int screenW, int screenH)
{
	IDirect3DDevice8 *pD3d8Device = *reinterpret_cast<IDirect3DDevice8**>(0xA32894);

	//Original geometry vanilla game uses
	CUSTOMVERTEX shadowRectDiffuse[] =
	{
		{0.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(SHADOW_OPACITY, 0, 0, 0)},
		{0.0f, (float)screenH, 0.0f, 1.0f, D3DCOLOR_ARGB(SHADOW_OPACITY, 0, 0, 0)},
		{(float)screenW, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(SHADOW_OPACITY, 0, 0, 0)},
		{(float)screenW, (float)screenH, 0.0f, 1.0f, D3DCOLOR_ARGB(SHADOW_OPACITY, 0, 0, 0)}
	};

	//No need for diffuse color, used to render from texture, requires texture coords
	CUSTOMVERTEX_UV shadowRectUV[] =
	{
		{0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, (float)screenH, 0.0f, 1.0f, 0.0f, 1.0f},
		{(float)screenW, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
		{(float)screenW, (float)screenH, 0.0f, 1.0f, 1.0f, 1.0f}
	};

	//Create our intermediate render targets/textures only once
	if (!pInTexture) {
		pD3d8Device->CreateTexture(screenW, screenH, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pInTexture);
		pInTexture->GetSurfaceLevel(0, &pInSurface);
	}

	if (!pOutTexture) {
		pD3d8Device->CreateTexture(screenW, screenH, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pOutTexture);
		pOutTexture->GetSurfaceLevel(0, &pOutSurface);
	}

	//Backup current state
	D3DSTATE state;
	backupState(&state);

	//Textures will be scaled bi-linearly
	pD3d8Device->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	pD3d8Device->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);

	//Turn off alpha blending
	pD3d8Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pD3d8Device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	//Back up current render target (backbuffer) and stencil buffer
	pD3d8Device->GetRenderTarget(&pBackBuffer);
	pD3d8Device->GetDepthStencilSurface(&pStencilBuffer);

	//Swap to new render target, maintain old stencil buffer and draw shadows
	pD3d8Device->SetRenderTarget(pInSurface, pStencilBuffer);
	pD3d8Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	pD3d8Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, shadowRectDiffuse, 20);

	//TODO: Would be more efficient to draw to a scaled down buffer here first and blur that

	pD3d8Device->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX1);
	pD3d8Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pD3d8Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//Create 4 full-screen quads, each offset a tiny amount diagonally in each direction
	CUSTOMVERTEX_UV blurUpLeft[] =
	{
		{   0.0f,    0.0f, 0.0f, 1.0f,    0.0f - (0.5f / screenW), 0.0f - (0.5f / screenH)},
		{   0.0f, screenH, 0.0f, 1.0f,    0.0f - (0.5f / screenW), 1.0f - (0.5f / screenH)},
		{screenW,    0.0f, 0.0f, 1.0f,    1.0f - (0.5f / screenW), 0.0f - (0.5f / screenH)},
		{screenW, screenH, 0.0f, 1.0f,    1.0f - (0.5f / screenW), 1.0f - (0.5f / screenH)}
	};

	CUSTOMVERTEX_UV blurDownLeft[] =
	{
		{   0.0f,    0.0f, 0.0f, 1.0f,    0.0f - (0.5f / screenW), 0.0f + (0.5f / screenH)},
		{   0.0f, screenH, 0.0f, 1.0f,    0.0f - (0.5f / screenW), 1.0f + (0.5f / screenH)},
		{screenW,    0.0f, 0.0f, 1.0f,    1.0f - (0.5f / screenW), 0.0f + (0.5f / screenH)},
		{screenW, screenH, 0.0f, 1.0f,    1.0f - (0.5f / screenW), 1.0f + (0.5f / screenH)}
	};

	CUSTOMVERTEX_UV blurUpRight[] =
	{
		{   0.0f,    0.0f, 0.0f, 1.0f,    0.0f + (0.5f / screenW), 0.0f - (0.5f / screenH)},
		{   0.0f, screenH, 0.0f, 1.0f,    0.0f + (0.5f / screenW), 1.0f - (0.5f / screenH)},
		{screenW,    0.0f, 0.0f, 1.0f,    1.0f + (0.5f / screenW), 0.0f - (0.5f / screenH)},
		{screenW, screenH, 0.0f, 1.0f,    1.0f + (0.5f / screenW), 1.0f - (0.5f / screenH)}
	};

	CUSTOMVERTEX_UV blurDownRight[] =
	{
		{   0.0f,    0.0f, 0.0f, 1.0f,    0.0f + (0.5f / screenW), 0.0f + (0.5f / screenH)},
		{   0.0f, screenH, 0.0f, 1.0f,    0.0f + (0.5f / screenW), 1.0f + (0.5f / screenH)},
		{screenW,    0.0f, 0.0f, 1.0f,    1.0f + (0.5f / screenW), 0.0f + (0.5f / screenH)},
		{screenW, screenH, 0.0f, 1.0f,    1.0f + (0.5f / screenW), 1.0f + (0.5f / screenH)}
	};

	//Bias coords to align correctly to screen space
	for (int j = 0; j < 4; j++)
	{
		blurUpLeft[j].x -= 0.5f;
		blurUpLeft[j].y -= 0.5f;

		blurDownLeft[j].x -= 0.5f;
		blurDownLeft[j].y -= 0.5f;

		blurUpRight[j].x -= 0.5f;
		blurUpRight[j].y -= 0.5f;

		blurDownRight[j].x -= 0.5f;
		blurDownRight[j].y -= 0.5f;
	}

	//Peform fixed function blur
	int PASSES = 16;
	for (int i = 0; i < PASSES; i++)
	{
		pD3d8Device->SetRenderTarget(pOutSurface, NULL);
		pD3d8Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		pD3d8Device->SetTexture(0, pInTexture);

		//Should probably be combined into one
		pD3d8Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, blurUpLeft, 24);
		pD3d8Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, blurDownLeft, 24);
		pD3d8Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, blurUpRight, 24);
		pD3d8Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, blurDownRight, 24);

		pD3d8Device->SetRenderTarget(pInSurface, NULL);
		pD3d8Device->SetTexture(0, pOutTexture);

		pD3d8Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		pD3d8Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, shadowRectUV, 24);
	}

	//Return to backbuffer but without stencil buffer
	pD3d8Device->SetRenderTarget(pBackBuffer, NULL);
	pD3d8Device->SetTexture(0, pInTexture);

	//Set up alpha-blending for final draw back to scene
	pD3d8Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pD3d8Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3d8Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	//Bias coords to align correctly to screen space
	for (int i = 0; i < 4; i++)
	{
		shadowRectUV[i].x -= 0.5f;
		shadowRectUV[i].y -= 0.5f;
	}

	pD3d8Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, shadowRectUV, 24);

	//Return to original render target and stencil buffer
	pD3d8Device->SetRenderTarget(pBackBuffer, pStencilBuffer);

	restoreState(&state);
}

void backupState(D3DSTATE *state)
{
	IDirect3DDevice8 *pD3d8Device = *reinterpret_cast<IDirect3DDevice8**>(0xA32894);

	pD3d8Device->GetTextureStageState(0, D3DTSS_MAGFILTER, &state->magFilter);
	pD3d8Device->GetTextureStageState(0, D3DTSS_MINFILTER, &state->minFilter);
	pD3d8Device->GetTextureStageState(0, D3DTSS_COLORARG1, &state->colorArg1);
	pD3d8Device->GetTextureStageState(0, D3DTSS_ALPHAARG1, &state->alphaArg1);

	pD3d8Device->GetRenderState(D3DRS_ALPHABLENDENABLE, &state->alphaBlendEnable); //Doesn't really need to be backed up
	pD3d8Device->GetRenderState(D3DRS_ALPHATESTENABLE, &state->alphaTestEnable);
	pD3d8Device->GetRenderState(D3DRS_SRCBLEND, &state->srcBlend);
	pD3d8Device->GetRenderState(D3DRS_DESTBLEND, &state->destBlend);

	pD3d8Device->GetTexture(0, &state->stage0); //Could use a later stage instead of backing up

	pD3d8Device->GetVertexShader(&state->vertexShader);
}

void restoreState(D3DSTATE *state)
{
	IDirect3DDevice8 *pD3d8Device = *reinterpret_cast<IDirect3DDevice8**>(0xA32894);

	pD3d8Device->SetTextureStageState(0, D3DTSS_MAGFILTER, state->magFilter);
	pD3d8Device->SetTextureStageState(0, D3DTSS_MINFILTER, state->minFilter);
	pD3d8Device->SetTextureStageState(0, D3DTSS_COLORARG1, state->colorArg1);
	pD3d8Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, state->alphaArg1);

	pD3d8Device->SetRenderState(D3DRS_ALPHABLENDENABLE, state->alphaBlendEnable);
	pD3d8Device->SetRenderState(D3DRS_ALPHATESTENABLE, state->alphaTestEnable);
	pD3d8Device->SetRenderState(D3DRS_SRCBLEND, state->srcBlend);
	pD3d8Device->SetRenderState(D3DRS_DESTBLEND, state->destBlend);

	pD3d8Device->SetTexture(0, state->stage0);

	pD3d8Device->SetVertexShader(state->vertexShader);
}
