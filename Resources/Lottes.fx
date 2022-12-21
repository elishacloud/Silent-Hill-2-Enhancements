//
// PUBLIC DOMAIN CRT STYLED SCAN-LINE SHADER
//
//   by Timothy Lottes
//
// This is more along the style of a really good CGA arcade monitor.
// With RGB inputs instead of NTSC.
// The shadow mask example has the mask rotated 90 degrees for less chromatic aberration.
//
// Left it unoptimized to show the theory behind the algorithm.
//
// It is an example what I personally would want as a display option for pixel art games.
// Please take and use, change, or whatever.
//
// Ported to ReShade by Zackin5
//

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

// -- config  -- //
uniform float hardScan <
	ui_label = "hardScan";
	ui_min = -20.0; ui_max = 0.0; ui_step = 1.0;
	ui_type = "slider";
> = -8.0;

uniform float hardPix <
	ui_label = "hardPix";
	ui_min = -20.0; ui_max = 0.0; ui_step = 1.0;
	ui_type = "slider";
> = -3.0;

uniform float2 warp <
	ui_label = "warp";
	ui_min = 0.0; ui_max = 0.125; ui_step = 0.01;
	ui_type = "slider";
> = float2(0.031, 0.041);

uniform float maskDark <
	ui_label = "maskDark";
	ui_min = 0.0; ui_max = 2.0; ui_step = 0.1;
	ui_type = "slider";
> = 0.5;

uniform float maskLight <
	ui_label = "maskLight";
	ui_min = 0.0; ui_max = 2.0; ui_step = 0.1;
	ui_type = "slider";
> = 1.5;

uniform bool scaleInLinearGamma <
	ui_label = "scaleInLinearGamma";
> = true;

uniform bool simpleLinearGamma <
	ui_label = "simpleLinearGamma";
> = false;

uniform float shadowMask <
	ui_label = "shadowMask";
	ui_min = 0.0; ui_max = 4.0; ui_step = 1.0;
	ui_type = "slider";
> = 3.0;

uniform float brightboost <
	ui_label = "brightness";
	ui_min = 0.0; ui_max = 2.0; ui_step = 0.05;
	ui_type = "slider";
> = 1.0;

uniform float hardBloomPix <
	ui_label = "bloom-x soft";
	ui_min = -2.0; ui_max = -0.5; ui_step = 0.1;
	ui_type = "slider";
> = -1.5;

uniform float hardBloomScan <
	ui_label = "bloom-y soft";
	ui_min = -4.0; ui_max = -1.0; ui_step = 0.1;
	ui_type = "slider";
> = -2.0;

uniform float bloomAmount <
	ui_label = "bloom amt";
	ui_min = 0.0; ui_max = 1.0; ui_step = 0.05;
	ui_type = "slider";
> = 0.15;

uniform float shape <
	ui_label = "filter kernel shape";
	ui_min = 0.0; ui_max = 10.0; ui_step = 0.05;
	ui_type = "slider";
> = 2.0;

uniform bool DO_BLOOM <
	ui_label = "use bloom";
> = true;

/* COMPATIBILITY
   - HLSL compilers
   - Cg   compilers
   - FX11 compilers
*/

//------------------------------------------------------------------------

// sRGB to Linear.
// Assuing using sRGB typed textures this should not be needed.
float ToLinear1(float c)
{
   return(c<=0.04045)?c/12.92:pow((c+0.055)/1.055,2.4);
}
float3 ToLinear(float3 c)
{
   if (scaleInLinearGamma || simpleLinearGamma) return c;
   return float3(ToLinear1(c.r),ToLinear1(c.g),ToLinear1(c.b));
}

// Linear to sRGB.
// Assuming using sRGB typed textures this should not be needed.
float ToSrgb1(float c)
{
   return(c<0.0031308?c*12.92:1.055*pow(c,0.41666)-0.055);
}

float3 ToSrgb(float3 c)
{
    if (simpleLinearGamma) return pow(c, 1.0 / 2.2);
    if (scaleInLinearGamma) return c;
    return float3(ToSrgb1(c.r),ToSrgb1(c.g),ToSrgb1(c.b));
}

// Nearest emulated sample given floating point position and texel offset.
// Also zero's off screen.
float3 Fetch(float2 pos, float2 off, float2 texture_size){
    pos=(floor(pos*texture_size.xy+off)+float2(0.5,0.5))/texture_size.xy;

    if (simpleLinearGamma)
        return ToLinear(brightboost * pow(tex2D(ReShade::BackBuffer,pos.xy).rgb, 2.2));
    else
        return ToLinear(brightboost * tex2D(ReShade::BackBuffer,pos.xy).rgb);
}

// Distance in emulated pixels to nearest texel.
float2 Dist(float2 pos, float2 texture_size){pos=pos*texture_size.xy;return -((pos-floor(pos))-float2(0.5, 0.5));}
    
// 1D Gaussian.
float Gaus(float pos,float scale){return exp2(scale*pow(abs(pos),shape));}

// 3-tap Gaussian filter along horz line.
float3 Horz3(float2 pos, float off, float2 texture_size){
  float3 b=Fetch(pos,float2(-1.0,off),texture_size);
  float3 c=Fetch(pos,float2( 0.0,off),texture_size);
  float3 d=Fetch(pos,float2( 1.0,off),texture_size);
  float dst=Dist(pos, texture_size).x;
  // Convert distance to weight.
  float scale=hardPix;
  float wb=Gaus(dst-1.0,scale);
  float wc=Gaus(dst+0.0,scale);
  float wd=Gaus(dst+1.0,scale);
  // Return filtered sample.
  return (b*wb+c*wc+d*wd)/(wb+wc+wd);}
  
// 5-tap Gaussian filter along horz line.
float3 Horz5(float2 pos, float off, float2 texture_size){
  float3 a=Fetch(pos,float2(-2.0,off),texture_size);
  float3 b=Fetch(pos,float2(-1.0,off),texture_size);
  float3 c=Fetch(pos,float2( 0.0,off),texture_size);
  float3 d=Fetch(pos,float2( 1.0,off),texture_size);
  float3 e=Fetch(pos,float2( 2.0,off),texture_size);
  float dst=Dist(pos, texture_size).x;
  // Convert distance to weight.
  float scale=hardPix;
  float wa=Gaus(dst-2.0,scale);
  float wb=Gaus(dst-1.0,scale);
  float wc=Gaus(dst+0.0,scale);
  float wd=Gaus(dst+1.0,scale);
  float we=Gaus(dst+2.0,scale);
  // Return filtered sample.
  return (a*wa+b*wb+c*wc+d*wd+e*we)/(wa+wb+wc+wd+we);}

// 7-tap Gaussian filter along horz line.
float3 Horz7(float2 pos, float off, float2 texture_size){
  float3 a=Fetch(pos,float2(-3.0,off),texture_size);
  float3 b=Fetch(pos,float2(-2.0,off),texture_size);
  float3 c=Fetch(pos,float2(-1.0,off),texture_size);
  float3 d=Fetch(pos,float2( 0.0,off),texture_size);
  float3 e=Fetch(pos,float2( 1.0,off),texture_size);
  float3 f=Fetch(pos,float2( 2.0,off),texture_size);
  float3 g=Fetch(pos,float2( 3.0,off),texture_size);
  float dst=Dist(pos, texture_size).x;
  // Convert distance to weight.
  float scale=hardBloomPix;
  float wa=Gaus(dst-3.0,scale);
  float wb=Gaus(dst-2.0,scale);
  float wc=Gaus(dst-1.0,scale);
  float wd=Gaus(dst+0.0,scale);
  float we=Gaus(dst+1.0,scale);
  float wf=Gaus(dst+2.0,scale);
  float wg=Gaus(dst+3.0,scale);
  // Return filtered sample.
  return (a*wa+b*wb+c*wc+d*wd+e*we+f*wf+g*wg)/(wa+wb+wc+wd+we+wf+wg);}

// Return scanline weight.
float Scan(float2 pos,float off, float2 texture_size){
  float dst=Dist(pos, texture_size).y;
  return Gaus(dst+off,hardScan);}
  
  // Return scanline weight for bloom.
float BloomScan(float2 pos,float off, float2 texture_size){
  float dst=Dist(pos, texture_size).y;
  return Gaus(dst+off,hardBloomScan);}

// Allow nearest three lines to effect pixel.
float3 Tri(float2 pos, float2 texture_size){
  float3 a=Horz3(pos,-1.0, texture_size);
  float3 b=Horz5(pos, 0.0, texture_size);
  float3 c=Horz3(pos, 1.0, texture_size);
  float wa=Scan(pos,-1.0, texture_size);
  float wb=Scan(pos, 0.0, texture_size);
  float wc=Scan(pos, 1.0, texture_size);
  return a*wa+b*wb+c*wc;}
  
// Small bloom.
float3 Bloom(float2 pos, float2 texture_size){
  float3 a=Horz5(pos,-2.0, texture_size);
  float3 b=Horz7(pos,-1.0, texture_size);
  float3 c=Horz7(pos, 0.0, texture_size);
  float3 d=Horz7(pos, 1.0, texture_size);
  float3 e=Horz5(pos, 2.0, texture_size);
  float wa=BloomScan(pos,-2.0, texture_size);
  float wb=BloomScan(pos,-1.0, texture_size);
  float wc=BloomScan(pos, 0.0, texture_size);
  float wd=BloomScan(pos, 1.0, texture_size);
  float we=BloomScan(pos, 2.0, texture_size);
  return a*wa+b*wb+c*wc+d*wd+e*we;}

// Distortion of scanlines, and end of screen alpha.
float2 Warp(float2 pos){
  pos=pos*2.0-1.0;    
  pos*=float2(1.0+(pos.y*pos.y)*warp.x,1.0+(pos.x*pos.x)*warp.y);
  return pos*0.5+0.5;}

// Shadow mask 
float3 Mask(float2 pos){
  float3 mask=float3(maskDark,maskDark,maskDark);

  // Very compressed TV style shadow mask.
  if (shadowMask == 1) {
    float mask_line = maskLight;
    float odd=0.0;
    if(frac(pos.x/6.0)<0.5) odd = 1.0;
    if(frac((pos.y+odd)/2.0)<0.5) mask_line = maskDark;  
    pos.x=frac(pos.x/3.0);
   
    if(pos.x<0.333)mask.r=maskLight;
    else if(pos.x<0.666)mask.g=maskLight;
    else mask.b=maskLight;
    mask *= mask_line;  
  } 

  // Aperture-grille.
  else if (shadowMask == 2) {
    pos.x=frac(pos.x/3.0);

    if(pos.x<0.333)mask.r=maskLight;
    else if(pos.x<0.666)mask.g=maskLight;
    else mask.b=maskLight;
  } 

  // Stretched VGA style shadow mask (same as prior shaders).
  else if (shadowMask == 3) {
    pos.x+=pos.y*3.0;
    pos.x=frac(pos.x/6.0);

    if(pos.x<0.333)mask.r=maskLight;
    else if(pos.x<0.666)mask.g=maskLight;
    else mask.b=maskLight;
  }

  // VGA style shadow mask.
  else if (shadowMask == 4) {
    pos.xy=floor(pos.xy*float2(1.0,0.5));
    pos.x+=pos.y*3.0;
    pos.x=frac(pos.x/6.0);

    if(pos.x<0.333)mask.r=maskLight;
    else if(pos.x<0.666)mask.g=maskLight;
    else mask.b=maskLight;
  }

  return mask;
}    

float4 crt_lottes(float4 vpos : SV_Position, float2 texcoord : TexCoord) : SV_Target
{    
    float2 pos = Warp(texcoord);
    float3 outColor = Tri(pos, ReShade::ScreenSize);

    if(DO_BLOOM)
        //Add Bloom
        outColor.rgb+=Bloom(pos, ReShade::ScreenSize)*bloomAmount;

    if(shadowMask)
        outColor.rgb*=Mask(floor(texcoord*ReShade::ScreenSize)+float2(0.5,0.5));

    return float4(ToSrgb(outColor.rgb),1.0);
}

technique CRT_Lottes
{
	pass CRT_Lottes
	{
		VertexShader=PostProcessVS;
		PixelShader=crt_lottes;
	}
}
