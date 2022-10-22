/*
Adapted for RetroArch from frutbunn's "Another CRT shader" from shadertoy:
https://www.shadertoy.com/view/XdyGzR
*/ 

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

uniform float screenCurveX <
	ui_type = "drag";
	ui_min = 0;
	ui_max = 1;
	ui_label = "Screen Curvature X";
> = .5;

uniform float screenCurveY <
	ui_type = "drag";
	ui_min = 0;
	ui_max = 1;
	ui_label = "Screen Curvature Y";
> = .5;

uniform float screenZoom <
	ui_type = "drag";
	ui_min = .5;
	ui_max = 1.5;
	ui_label = "Screen Zoom";
> = 1;

uniform bool CURVATURE <
	ui_type = "boolean";
	ui_label = "Curvature Toggle [CRT-Frutbunn]";
> = true;

uniform bool SCANLINES <
	ui_type = "boolean";
	ui_label = "Scanlines Toggle [CRT-Frutbunn]";
> = true;

uniform bool CURVED_SCANLINES <
	ui_type = "boolean";
	ui_label = "Scanline Curve Toggle [CRT-Frutbunn]";
> = true;

uniform bool LIGHT <
	ui_type = "boolean";
	ui_label = "Vignetting Toggle [CRT-Frutbunn]";
> = true;

uniform int light <
	ui_type = "drag";
	ui_min = 0;
	ui_max = 20;
	ui_step = 1;
	ui_label = "Vignetting Strength [CRT-Frutbunn]";
> = 9;

uniform float blur <
	ui_type = "drag";
	ui_min = 0.0;
	ui_max = 8.0;
	ui_step = 0.05;
	ui_label = "Blur Strength [CRT-Frutbunn]";
> = 1.0;

uniform float texture_sizeX = BUFFER_WIDTH;
uniform float texture_sizeY = BUFFER_HEIGHT;

static const float gamma = 1.0;
static const float contrast = 1.0;
static const float saturation = 1.0;
static const float brightness = 1.0;

#define texture_size float2(texture_sizeX, texture_sizeY)
#define texture_size_pixel float2(1.0/texture_sizeX, 1.0/texture_sizeY)

// Sigma 1. Size 3
float3 gaussian(in float2 uv, in sampler tex, in float2 iResolution) {
    float b = blur / (iResolution.x / iResolution.y);

    float3 col = tex2D(tex, float2(uv.x - b/iResolution.x, uv.y - b/iResolution.y) ).rgb * 0.077847;
    col += tex2D(tex, float2(uv.x - b/iResolution.x, uv.y) ).rgb * 0.123317;
    col += tex2D(tex, float2(uv.x - b/iResolution.x, uv.y + b/iResolution.y) ).rgb * 0.077847;

    col += tex2D(tex, float2(uv.x, uv.y - b/iResolution.y) ).rgb * 0.123317;
    col += tex2D(tex, float2(uv.x, uv.y) ).rgb * 0.195346;
    col += tex2D(tex, float2(uv.x, uv.y + b/iResolution.y) ).rgb * 0.123317;

    col += tex2D(tex, float2(uv.x + b/iResolution.x, uv.y - b/iResolution.y) ).rgb * 0.077847;
    col += tex2D(tex, float2(uv.x + b/iResolution.x, uv.y) ).rgb * 0.123317;
    col += tex2D(tex, float2(uv.x + b/iResolution.x, uv.y + b/iResolution.y) ).rgb * 0.077847;

    return col;
}

float4 PS_CRTFrutbunn( float4 pos : SV_Position, float2 txcoord : TEXCOORD0) : SV_Target
{
    float2 st = txcoord - float2(0.5,0.5);
    
    // Curvature/light
    float d = length(st*screenCurveX * st*screenCurveY);
	float2 uv;
	
	if (CURVATURE){
		uv = st*d + st*screenZoom;
	}else{
		uv = st;
	}
		
	// CRT color blur
	float3 color = gaussian(uv+.5, ReShade::BackBuffer, texture_size.xy * 2.0);

	// Light
	if (LIGHT > 0.5){
		float l = 1. - min(1., d*light);
		color *= l;
	}

	// Scanlines
	float y;
	if (CURVED_SCANLINES){
		y = uv.y;
	}else{
		y = st.y;
	}

	float showScanlines = 1.;
	//    if (texture_size.y<360.) showScanlines = 0.;
		
	if (SCANLINES)
	{
		float s = 3.189538 + ReShade::ScreenSize.y * texture_size_pixel.y;//1. - smoothstep(texture_size.x, ReShade::ScreenSize.x, texture_size.y) + 4.;//1. - smoothstep(320., 1440., texture_size.y) + 4.;
		float j = cos(y*texture_size.y*s)*.25; // values between .01 to .25 are ok.
		color = abs(showScanlines-1.)*color + showScanlines*(color - color*j);
	//    color *= 1. - ( .01 + ceil(mod( (st.x+.5)*texture_size.x, 3.) ) * (.995-1.01) )*showScanlines;
	}

		// Border mask
	if (CURVATURE)
	{
			float m = max(0.0, 1. - 2.*max(abs(uv.x), abs(uv.y) ) );
			m = min(m*200., 1.);
			color *= m;
	}

	return float4(color, 1.0);//float4(max(float3(0.0,0.0,0.0), min(float3(1.0,1.0,1.0), color)), 1.);
}

technique CRTFrutbunn {
    pass CRTFrutbunn {
		VertexShader = PostProcessVS;
		PixelShader = PS_CRTFrutbunn;
	}
}
