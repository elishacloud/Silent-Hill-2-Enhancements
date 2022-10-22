/*
	Adapted from
	http://filmicworlds.com/blog/minimal-color-grading-tools/

	Work-in-progress.
*/
// #region Preprocessor

//-------------------------------------------------------------
// BEGIN "ReShadeUI.fxh"
//
#if !defined(__RESHADE__) || __RESHADE__ < 30000
#error "ReShade 3.0+ is required to use this header file"
#endif

#define RESHADE_VERSION(major,minor,build) (10000 * (major) + 100 * (minor) + (build))
#define SUPPORTED_VERSION(major,minor,build) (__RESHADE__ >= RESHADE_VERSION(major,minor,build))

// Since 3.0.0
// Commit current in-game user interface status
// https://github.com/crosire/reshade/commit/302bacc49ae394faedc2e29a296c1cebf6da6bb2#diff-82cf230afdb2a0d5174111e6f17548a5R1183
// Added various GUI related uniform variable annotations
// https://reshade.me/forum/releases/2341-3-0
#define __UNIFORM_INPUT_ANY    ui_type = "input";

#define __UNIFORM_INPUT_BOOL1  __UNIFORM_INPUT_ANY // It is unsupported on all version
#define __UNIFORM_INPUT_BOOL2  __UNIFORM_INPUT_ANY // It is unsupported on all version
#define __UNIFORM_INPUT_BOOL3  __UNIFORM_INPUT_ANY // It is unsupported on all version
#define __UNIFORM_INPUT_BOOL4  __UNIFORM_INPUT_ANY // It is unsupported on all version
#define __UNIFORM_INPUT_INT1   __UNIFORM_INPUT_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_INPUT_INT2   __UNIFORM_INPUT_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_INPUT_INT3   __UNIFORM_INPUT_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_INPUT_INT4   __UNIFORM_INPUT_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_INPUT_FLOAT1 __UNIFORM_INPUT_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_INPUT_FLOAT2 __UNIFORM_INPUT_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_INPUT_FLOAT3 __UNIFORM_INPUT_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_INPUT_FLOAT4 __UNIFORM_INPUT_ANY // If it was not supported in someday or now, please add information

// Since 4.0.1
// Change slider widget to be used with new "slider" instead of a "drag" type annotation
// https://github.com/crosire/reshade/commit/746229f31cd6f311a3e72a543e4f1f23faa23f11#diff-59405a313bd8cbfb0ca6dd633230e504R1701
// Changed slider widget to be used with < ui_type = "slider"; > instead of < ui_type = "drag"; >
// https://reshade.me/forum/releases/4772-4-0
#if SUPPORTED_VERSION(4,0,1)
#define __UNIFORM_DRAG_ANY    ui_type = "drag";

// Since 4.0.0
// Rework statistics tab and add drag widgets back
// https://github.com/crosire/reshade/commit/1b2c38795f00efd66c007da1f483f1441b230309
// Changed drag widget to a slider widget (old one is still available via < ui_type = "drag2"; >)
// https://reshade.me/forum/releases/4772-4-0
#elif SUPPORTED_VERSION(4,0,0)
#define __UNIFORM_DRAG_ANY    ui_type = "drag2";

// Since 3.0.0
// Commit current in-game user interface status
// https://github.com/crosire/reshade/commit/302bacc49ae394faedc2e29a296c1cebf6da6bb2#diff-82cf230afdb2a0d5174111e6f17548a5R1187
// Added various GUI related uniform variable annotations
// https://reshade.me/forum/releases/2341-3-0
#else
#define __UNIFORM_DRAG_ANY    ui_type = "drag";
#endif

#define __UNIFORM_DRAG_BOOL1  __UNIFORM_DRAG_ANY // It is unsupported on all version
#define __UNIFORM_DRAG_BOOL2  __UNIFORM_DRAG_ANY // It is unsupported on all version
#define __UNIFORM_DRAG_BOOL3  __UNIFORM_DRAG_ANY // It is unsupported on all version
#define __UNIFORM_DRAG_BOOL4  __UNIFORM_DRAG_ANY // It is unsupported on all version
#define __UNIFORM_DRAG_INT1   __UNIFORM_DRAG_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_DRAG_INT2   __UNIFORM_DRAG_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_DRAG_INT3   __UNIFORM_DRAG_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_DRAG_INT4   __UNIFORM_DRAG_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_DRAG_FLOAT1 __UNIFORM_DRAG_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_DRAG_FLOAT2 __UNIFORM_DRAG_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_DRAG_FLOAT3 __UNIFORM_DRAG_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_DRAG_FLOAT4 __UNIFORM_DRAG_ANY // If it was not supported in someday or now, please add information

// Since 4.0.1
// Change slider widget to be used with new "slider" instead of a "drag" type annotation
// https://github.com/crosire/reshade/commit/746229f31cd6f311a3e72a543e4f1f23faa23f11#diff-59405a313bd8cbfb0ca6dd633230e504R1699
// Changed slider widget to be used with < ui_type = "slider"; > instead of < ui_type = "drag"; >
// https://reshade.me/forum/releases/4772-4-0
#if SUPPORTED_VERSION(4,0,1)
#define __UNIFORM_SLIDER_ANY    ui_type = "slider";

// Since 4.0.0
// Rework statistics tab and add drag widgets back
// https://github.com/crosire/reshade/commit/1b2c38795f00efd66c007da1f483f1441b230309
// Changed drag widget to a slider widget (old one is still available via < ui_type = "drag2"; >)
// https://reshade.me/forum/releases/4772-4-0
#elif SUPPORTED_VERSION(4,0,0)
#define __UNIFORM_SLIDER_ANY    ui_type = "drag";
#else
#define __UNIFORM_SLIDER_ANY    __UNIFORM_DRAG_ANY
#endif

#define __UNIFORM_SLIDER_BOOL1  __UNIFORM_SLIDER_ANY // It is unsupported on all version
#define __UNIFORM_SLIDER_BOOL2  __UNIFORM_SLIDER_ANY // It is unsupported on all version
#define __UNIFORM_SLIDER_BOOL3  __UNIFORM_SLIDER_ANY // It is unsupported on all version
#define __UNIFORM_SLIDER_BOOL4  __UNIFORM_SLIDER_ANY // It is unsupported on all version
#define __UNIFORM_SLIDER_INT1   __UNIFORM_SLIDER_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_SLIDER_INT2   __UNIFORM_SLIDER_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_SLIDER_INT3   __UNIFORM_SLIDER_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_SLIDER_INT4   __UNIFORM_SLIDER_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_SLIDER_FLOAT1 __UNIFORM_SLIDER_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_SLIDER_FLOAT2 __UNIFORM_SLIDER_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_SLIDER_FLOAT3 __UNIFORM_SLIDER_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_SLIDER_FLOAT4 __UNIFORM_SLIDER_ANY // If it was not supported in someday or now, please add information

// Since 3.0.0
// Add combo box display type for uniform variables and fix displaying of integer variable under Direct3D 9
// https://github.com/crosire/reshade/commit/b025bfae5f7343509ec0cacf6df0cff537c499f2#diff-82cf230afdb2a0d5174111e6f17548a5R1631
// Added various GUI related uniform variable annotations
// https://reshade.me/forum/releases/2341-3-0
#define __UNIFORM_COMBO_ANY    ui_type = "combo";

//      __UNIFORM_COMBO_BOOL1
#define __UNIFORM_COMBO_BOOL2  __UNIFORM_COMBO_ANY // It is unsupported on all version
#define __UNIFORM_COMBO_BOOL3  __UNIFORM_COMBO_ANY // It is unsupported on all version
#define __UNIFORM_COMBO_BOOL4  __UNIFORM_COMBO_ANY // It is unsupported on all version
#define __UNIFORM_COMBO_INT1   __UNIFORM_COMBO_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_COMBO_INT2   __UNIFORM_COMBO_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_COMBO_INT3   __UNIFORM_COMBO_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_COMBO_INT4   __UNIFORM_COMBO_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_COMBO_FLOAT1 __UNIFORM_COMBO_ANY // It is unsupported on all version
#define __UNIFORM_COMBO_FLOAT2 __UNIFORM_COMBO_ANY // It is unsupported on all version
#define __UNIFORM_COMBO_FLOAT3 __UNIFORM_COMBO_ANY // It is unsupported on all version
#define __UNIFORM_COMBO_FLOAT4 __UNIFORM_COMBO_ANY // It is unsupported on all version

// Since 4.0.0 (but the ui_items force set "Off\0On\0"), and if less than it force converted to checkbox
// Add option to display boolean values as combo box instead of checkbox
// https://github.com/crosire/reshade/commit/aecb757c864c9679e77edd6f85a1521c49e489c1#diff-59405a313bd8cbfb0ca6dd633230e504R1147
// https://github.com/crosire/reshade/blob/v4.0.0/source/gui.cpp
// Added option to display boolean values as combo box instead of checkbox (via < ui_type = "combo"; >)
// https://reshade.me/forum/releases/4772-4-0
#define __UNIFORM_COMBO_BOOL1  __UNIFORM_COMBO_ANY

// Since 4.0.0
// Cleanup GUI code and rearrange some widgets
// https://github.com/crosire/reshade/commit/6751f7bd50ea7c0556cf0670f10a4b4ba912ee7d#diff-59405a313bd8cbfb0ca6dd633230e504R1711
// Added radio button widget (via < ui_type = "radio"; ui_items = "Button 1\0Button 2\0...\0"; >)
// https://reshade.me/forum/releases/4772-4-0
#if SUPPORTED_VERSION(4,0,0)
#define __UNIFORM_RADIO_ANY    ui_type = "radio";
#else
#define __UNIFORM_RADIO_ANY    __UNIFORM_COMBO_ANY
#endif

#define __UNIFORM_RADIO_BOOL1  __UNIFORM_RADIO_ANY // It is unsupported on all version
#define __UNIFORM_RADIO_BOOL2  __UNIFORM_RADIO_ANY // It is unsupported on all version
#define __UNIFORM_RADIO_BOOL3  __UNIFORM_RADIO_ANY // It is unsupported on all version
#define __UNIFORM_RADIO_BOOL4  __UNIFORM_RADIO_ANY // It is unsupported on all version
#define __UNIFORM_RADIO_INT1   __UNIFORM_RADIO_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_RADIO_INT2   __UNIFORM_RADIO_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_RADIO_INT3   __UNIFORM_RADIO_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_RADIO_INT4   __UNIFORM_RADIO_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_RADIO_FLOAT1 __UNIFORM_RADIO_ANY // It is unsupported on all version
#define __UNIFORM_RADIO_FLOAT2 __UNIFORM_RADIO_ANY // It is unsupported on all version
#define __UNIFORM_RADIO_FLOAT3 __UNIFORM_RADIO_ANY // It is unsupported on all version
#define __UNIFORM_RADIO_FLOAT4 __UNIFORM_RADIO_ANY // It is unsupported on all version

// Since 4.1.0
// Fix floating point uniforms with unknown "ui_type" not showing up in UI
// https://github.com/crosire/reshade/commit/50e5bf44dfc84bc4220c2b9f19d5f50c7a0fda66#diff-59405a313bd8cbfb0ca6dd633230e504R1788
// Fixed floating point uniforms with unknown "ui_type" not showing up in UI
// https://reshade.me/forum/releases/5021-4-1
#define __UNIFORM_COLOR_ANY    ui_type = "color";

// Since 3.0.0
// Move technique list to preset configuration file
// https://github.com/crosire/reshade/blob/84bba3aa934c1ebe4c6419b69dfe1690d9ab9d34/source/runtime.cpp#L1328
// Added various GUI related uniform variable annotations
// https://reshade.me/forum/releases/2341-3-0

// If empty, these versions before 4.1.0 are decide that the type is color from the number of components

#define __UNIFORM_COLOR_BOOL1  __UNIFORM_COLOR_ANY // It is unsupported on all version
#define __UNIFORM_COLOR_BOOL2  __UNIFORM_COLOR_ANY // It is unsupported on all version
#define __UNIFORM_COLOR_BOOL3  __UNIFORM_COLOR_ANY // It is unsupported on all version
#define __UNIFORM_COLOR_BOOL4  __UNIFORM_COLOR_ANY // It is unsupported on all version
#define __UNIFORM_COLOR_INT1   __UNIFORM_COLOR_ANY // It is unsupported on all version
#define __UNIFORM_COLOR_INT2   __UNIFORM_COLOR_ANY // It is unsupported on all version
#define __UNIFORM_COLOR_INT3   __UNIFORM_COLOR_ANY // It is unsupported on all version
#define __UNIFORM_COLOR_INT4   __UNIFORM_COLOR_ANY // It is unsupported on all version
//      __UNIFORM_COLOR_FLOAT1
#define __UNIFORM_COLOR_FLOAT2 __UNIFORM_COLOR_ANY // It is unsupported on all version
#define __UNIFORM_COLOR_FLOAT3 __UNIFORM_COLOR_ANY // If it was not supported in someday or now, please add information
#define __UNIFORM_COLOR_FLOAT4 __UNIFORM_COLOR_ANY // If it was not supported in someday or now, please add information

// Since 4.2.0
// Add alpha slider widget for single component uniform variables (#86)
// https://github.com/crosire/reshade/commit/87a740a8e3c4dcda1dd4eeec8d5cff7fa35fe829#diff-59405a313bd8cbfb0ca6dd633230e504R1820
// Added alpha slider widget for single component uniform variables
// https://reshade.me/forum/releases/5150-4-2
#if SUPPORTED_VERSION(4,2,0)
#define __UNIFORM_COLOR_FLOAT1 __UNIFORM_COLOR_ANY
#else
#define __UNIFORM_COLOR_FLOAT1 __UNIFORM_SLIDER_ANY
#endif

// Since 4.3.0
// Add new "list" GUI widget (#103)
// https://github.com/crosire/reshade/commit/515287d20ce615c19cf3d4c21b49f83896f04ddc#diff-59405a313bd8cbfb0ca6dd633230e504R1894
// Added new "list" GUI widget
// https://reshade.me/forum/releases/5417-4-3
#if SUPPORTED_VERSION(4,3,0)
#define __UNIFORM_LIST_ANY    ui_type = "list";
#else
#define __UNIFORM_LIST_ANY    __UNIFORM_COMBO_ANY
#endif

//      __UNIFORM_LIST_BOOL1
#define __UNIFORM_LIST_BOOL2  __UNIFORM_LIST_ANY // Not supported in all versions
#define __UNIFORM_LIST_BOOL3  __UNIFORM_LIST_ANY // Not supported in all versions
#define __UNIFORM_LIST_BOOL4  __UNIFORM_LIST_ANY // Not supported in all versions
#define __UNIFORM_LIST_INT1   __UNIFORM_LIST_ANY // Supported in 4.3.0
#define __UNIFORM_LIST_INT2   __UNIFORM_LIST_ANY // Not supported in all versions
#define __UNIFORM_LIST_INT3   __UNIFORM_LIST_ANY // Not supported in all versions
#define __UNIFORM_LIST_INT4   __UNIFORM_LIST_ANY // Not supported in all versions
#define __UNIFORM_LIST_FLOAT1 __UNIFORM_LIST_ANY // Not supported in all versions
#define __UNIFORM_LIST_FLOAT2 __UNIFORM_LIST_ANY // Not supported in all versions
#define __UNIFORM_LIST_FLOAT3 __UNIFORM_LIST_ANY // Not supported in all versions
#define __UNIFORM_LIST_FLOAT4 __UNIFORM_LIST_ANY // Not supported in all versions

// For compatible with ComboBox
#define __UNIFORM_LIST_BOOL1  __UNIFORM_COMBO_ANY
//
// End "ReShadeUI.fxh"
//-------------------------------------------------------------

//-------------------------------------------------------------
//
// End "ReShade.fxh"
//
#if !defined(__RESHADE__) || __RESHADE__ < 30000
	#error "ReShade 3.0+ is required to use this header file"
#endif

#ifndef RESHADE_DEPTH_INPUT_IS_UPSIDE_DOWN
	#define RESHADE_DEPTH_INPUT_IS_UPSIDE_DOWN 0
#endif
#ifndef RESHADE_DEPTH_INPUT_IS_REVERSED
	#define RESHADE_DEPTH_INPUT_IS_REVERSED 1
#endif
#ifndef RESHADE_DEPTH_INPUT_IS_LOGARITHMIC
	#define RESHADE_DEPTH_INPUT_IS_LOGARITHMIC 0
#endif

#ifndef RESHADE_DEPTH_MULTIPLIER
	#define RESHADE_DEPTH_MULTIPLIER 1
#endif
#ifndef RESHADE_DEPTH_LINEARIZATION_FAR_PLANE
	#define RESHADE_DEPTH_LINEARIZATION_FAR_PLANE 1000.0
#endif

// Above 1 expands coordinates, below 1 contracts and 1 is equal to no scaling on any axis
#ifndef RESHADE_DEPTH_INPUT_Y_SCALE
	#define RESHADE_DEPTH_INPUT_Y_SCALE 1
#endif
#ifndef RESHADE_DEPTH_INPUT_X_SCALE
	#define RESHADE_DEPTH_INPUT_X_SCALE 1
#endif
// An offset to add to the Y coordinate, (+) = move up, (-) = move down
#ifndef RESHADE_DEPTH_INPUT_Y_OFFSET
	#define RESHADE_DEPTH_INPUT_Y_OFFSET 0
#endif
// An offset to add to the X coordinate, (+) = move right, (-) = move left
#ifndef RESHADE_DEPTH_INPUT_X_OFFSET
	#define RESHADE_DEPTH_INPUT_X_OFFSET 0
#endif

#define BUFFER_PIXEL_SIZE float2(BUFFER_RCP_WIDTH, BUFFER_RCP_HEIGHT)
#define BUFFER_SCREEN_SIZE float2(BUFFER_WIDTH, BUFFER_HEIGHT)
#define BUFFER_ASPECT_RATIO (BUFFER_WIDTH * BUFFER_RCP_HEIGHT)

namespace ReShade
{
#if defined(__RESHADE_FXC__)
	float GetAspectRatio() { return BUFFER_WIDTH * BUFFER_RCP_HEIGHT; }
	float2 GetPixelSize() { return float2(BUFFER_RCP_WIDTH, BUFFER_RCP_HEIGHT); }
	float2 GetScreenSize() { return float2(BUFFER_WIDTH, BUFFER_HEIGHT); }
	#define AspectRatio GetAspectRatio()
	#define PixelSize GetPixelSize()
	#define ScreenSize GetScreenSize()
#else
	// These are deprecated and will be removed eventually.
	static const float AspectRatio = BUFFER_WIDTH * BUFFER_RCP_HEIGHT;
	static const float2 PixelSize = float2(BUFFER_RCP_WIDTH, BUFFER_RCP_HEIGHT);
	static const float2 ScreenSize = float2(BUFFER_WIDTH, BUFFER_HEIGHT);
#endif

	// Global textures and samplers
	texture BackBufferTex : COLOR;
	texture DepthBufferTex : DEPTH;

	sampler BackBuffer { Texture = BackBufferTex; };
	sampler DepthBuffer { Texture = DepthBufferTex; };

	// Helper functions
	float GetLinearizedDepth(float2 texcoord)
	{
#if RESHADE_DEPTH_INPUT_IS_UPSIDE_DOWN
		texcoord.y = 1.0 - texcoord.y;
#endif
		texcoord.x /= RESHADE_DEPTH_INPUT_X_SCALE;
		texcoord.y /= RESHADE_DEPTH_INPUT_Y_SCALE;
		texcoord.x -= RESHADE_DEPTH_INPUT_X_OFFSET / 2.000000001;
		texcoord.y += RESHADE_DEPTH_INPUT_Y_OFFSET / 2.000000001;
		float depth = tex2Dlod(DepthBuffer, float4(texcoord, 0, 0)).x * RESHADE_DEPTH_MULTIPLIER;

#if RESHADE_DEPTH_INPUT_IS_LOGARITHMIC
		const float C = 0.01;
		depth = (exp(depth * log(C + 1.0)) - 1.0) / C;
#endif
#if RESHADE_DEPTH_INPUT_IS_REVERSED
		depth = 1.0 - depth;
#endif
		const float N = 1.0;
		depth /= RESHADE_DEPTH_LINEARIZATION_FAR_PLANE - depth * (RESHADE_DEPTH_LINEARIZATION_FAR_PLANE - N);

		return depth;
	}
}

// Vertex shader generating a triangle covering the entire screen
void PostProcessVS(in uint id : SV_VertexID, out float4 position : SV_Position, out float2 texcoord : TEXCOORD)
{
	texcoord.x = (id == 2) ? 2.0 : 0.0;
	texcoord.y = (id == 1) ? 2.0 : 0.0;
	position = float4(texcoord * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
}
//
// End "ReShade.fxh"
//-------------------------------------------------------------

#ifndef MINIMAL_COLOR_GRADING_USE_HDR_BUFFER
#define MINIMAL_COLOR_GRADING_USE_HDR_BUFFER 0
#endif

// #endregion

// #region Constants

static const float EPSILON = 1e-5;
static const float CONTRAST_LOG_MIDPOINT = 0.18;

// #endregion

// #region Textures

sampler BackBuffer
{
	Texture = ReShade::BackBufferTex;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
	SRGBTexture = true;
};

#if MINIMAL_COLOR_GRADING_USE_HDR_BUFFER

texture MinimalColorGrading_HDRBufferTex <pooled = true;>
{
	Width = BUFFER_WIDTH;
	Height = BUFFER_HEIGHT;
	Format = RGB10A2;
};
sampler HDRBuffer
{
	Texture = MinimalColorGrading_HDRBufferTex;
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = POINT;
};

#endif

// #endregion

// #region Uniforms

uniform float Exposure
<
	__UNIFORM_SLIDER_FLOAT1

	ui_label = "Exposure";
	ui_tooltip = "Default: 0.0";
	ui_category = "Basic Adjustments";
	ui_min = -3.0;
	ui_max = 3.0;
> = 0.0;

uniform float3 ColorFilter
<
	__UNIFORM_COLOR_FLOAT3

	ui_label = "Color Filter";
	ui_tooltip = "Default: 255 255 255";
	ui_category = "Basic Adjustments";
> = float3(1.0, 1.0, 1.0);

uniform float Saturation
<
	__UNIFORM_SLIDER_FLOAT1

	ui_label = "Saturation";
	ui_tooltip = "Default: 1.0";
	ui_category = "Basic Adjustments";
	ui_min = 0.0;
	ui_max = 2.0;
> = 1.0;

uniform float Contrast
<
	__UNIFORM_SLIDER_FLOAT1

	ui_label = "Contrast";
	ui_tooltip = "Default: 1.0";
	ui_category = "Basic Adjustments";
	ui_min = 0.0;
	ui_max = 2.0;
> = 1.0;

uniform int SaturationMode
<
	__UNIFORM_COMBO_INT1

	ui_label = "Saturation Mode";
	ui_tooltip = "Default: Luma";
	ui_items = "Average\0Luminance\0Luma\0Length\0";
> = 2;

uniform float2 MousePoint <source="mousepoint";>;

// #endregion

// #region Functions

float get_grayscale(float3 color)
{
	switch (SaturationMode)
	{
		case 0: // Average
			return dot(color, 0.333);
		case 1: // Luminance
			return max(color.r, max(color.g, color.b));
		case 2: // Luma
			return dot(color, float3(0.299, 0.587, 0.114));
		case 3: // Length
			return exp2(length(log2(color)));
	}

	return 0.0;
}

float3 apply_exposure_color(float3 color)
{
	return color.rgb * exp2(Exposure) * ColorFilter;
}

float3 apply_saturation(float3 color)
{
	float gray = get_grayscale(color.rgb);
	return gray + (color.rgb - gray) * Saturation;
}

float3 apply_contrast(float3 color)
{
	color = log2(color + EPSILON);
	color = CONTRAST_LOG_MIDPOINT + (color - CONTRAST_LOG_MIDPOINT) * Contrast;
	return max(0.0, exp2(color) - EPSILON);
}

// #endregion

// #region Shaders

#if MINIMAL_COLOR_GRADING_USE_HDR_BUFFER

float4 CopyToHDRPS(float4 p : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET
{
	float4 color = tex2D(BackBuffer, uv);
	//color.rgb = inv_reinhard(color.rgb, 1.0 / 10.0);
	return color;
}

#endif

float4 MainPS(float4 p : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET
{
	#if MINIMAL_COLOR_GRADING_USE_HDR_BUFFER

	float4 color = tex2D(HDRBuffer, uv);

	#else

	float4 color = tex2D(BackBuffer, uv);

	#endif

	color.rgb = apply_exposure_color(color.rgb);
	color.rgb = apply_saturation(color.rgb);
	color.rgb = apply_contrast(color.rgb);

	return color;
}

// #endregion

// #region Technique

technique MinimalColorGrading
{
	#if MINIMAL_COLOR_GRADING_USE_HDR_BUFFER

	pass CopyToHDR
	{
		VertexShader = PostProcessVS;
		PixelShader = CopyToHDRPS;
		RenderTarget = MinimalColorGrading_HDRBufferTex;
	}

	#endif

	pass Main
	{
		VertexShader = PostProcessVS;
		PixelShader = MainPS;
		SRGBWriteEnable = true;
	}
}

// #endregion