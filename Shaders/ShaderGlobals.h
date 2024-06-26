// Metallicafan212:	TODO! More shader globals

// Metallicafan212:	Global vertex definition
#ifndef DEFINED_VERTEX
#define DEFINED_VERTEX 1
struct VSInput 
{ 
	float4 pos 			: POSITION0;
	float4 uv			: TEXCOORD0;
	float4 color		: COLOR0;
	float4 fog			: COLOR1;
	
	// Metallicafan212:	This is used for complex surfaces
	#if EXTRA_VERT_INFO
	#if !COMPLEX_SURF_MANUAL_UVs
	float4 XAxis		: TEXCOORD1;
	float4 YAxis		: TEXCOORD2;
	float4 PanScale1	: TEXCOORD3;
	float4 PanScale2	: TEXCOORD4;
	float4 PanScale3	: TEXCOORD5;
	float4 PanScale4	: TEXCOORD6;
	float4 PanScale5	: TEXCOORD7;
	float4 LFScale		: TEXCOORD8;
	#else
	// Metallicafan212:	Per channel UV info
	float4  DM			: TEXCOORD1;
	float4	FX			: TEXCOORD2;
	//float2 D			: TEXCOORD1;
	//float2 M			: TEXCOORD2;
	//float2 F			: TEXCOORD3;
	#endif
	#endif
};
#endif

#ifndef PI
#define PI 					(3.1415926535897932f)
#endif

/*
#ifndef START_CONST_NUM
#define START_CONST_NUM		b3
#endif
*/

#ifndef FIRST_USER_CONSTBUFF
#define FIRST_USER_CONSTBUFF b4
#endif

// Metallicafan212:	OLDVER!
#ifndef START_CONST_NUM
#define START_CONST_NUM FIRST_USER_CONSTBUFF
#endif

// Metallicafan212:	If to define the "final" color function
//					The complex surface shader redefines the input buffer (as I don't want to keep track of multiple independant buffers)
//					So we need a way to JUST define this function (if we include it again

#ifndef DO_STANDARD_BUFFER
#define DO_STANDARD_BUFFER 1
#define DO_FINAL_COLOR 1
#endif

#ifndef MAX_TEX_NUM
#define MAX_TEX_NUM 16
	// Metallicafan212:	Proton doesn't like math in definitions.....
	#if WINE
	#define TEX_ARRAY_SIZE 4
	#else
	#define TEX_ARRAY_SIZE (MAX_TEX_NUM / 4)
	#endif
#endif

// Metallicafan212: Texture vars were moved to another constant buffer (since it could change per draw call or not)
//					Since per-shader information should be limited, it shouldn't be writing it all the time
/*
#ifndef COMMON_VARS	
	#define COMMON_VARS \
		int4	bTexturesBound[TEX_ARRAY_SIZE]			: packoffset(c0);
#endif
*/

#ifndef COMMON_VARS
	#define COMMON_VARS
#endif

#if DO_STANDARD_BUFFER
cbuffer CommonBuffer : register (START_CONST_NUM)
{
    COMMON_VARS
};
#endif


#ifdef DO_FINAL_COLOR

// Metallicafan212:	Define the base constant buffer!!!!
cbuffer FrameVariables : register (b0)
{
	matrix 	Proj				: packoffset(c0);
	float	Gamma				: packoffset(c4.x);
	float2	ViewSize			: packoffset(c4.y);
	int		bDoSelection		: packoffset(c4.w);
	int		bOneXLightmaps		: packoffset(c5.x);
	int		bCorrectFog			: packoffset(c5.y);
	int		bHDR				: packoffset(c5.z);
	int 	GammaMode			: packoffset(c5.w);
	float	HDRExpansion		: packoffset(c6.x);
	float 	ResolutionScale 	: packoffset(c6.y);
	float	WhiteLevel			: packoffset(c6.z);
	//float 	PadHDR2			: packoffset(c6.w);
	int		bDepthDraw			: packoffset(c6.w);
	float	DepthDrawLimit		: packoffset(c7.x);
	
	// Metallicafan212:	DX9 gamma offset values
	float	GammaOffsetRed		: packoffset(c7.y);
	float	GammaOffsetBlue		: packoffset(c7.z);
	float	GammaOffsetGreen	: packoffset(c7.w);
	
	//float3	Paddddddd		: packoffset(c7.y);
};

cbuffer DFogVariables : register (b1)
{
	float4 	DistanceFogColor 	: packoffset(c0);
	float4 	DistanceFogSettings	: packoffset(c1);
	int 	bDoDistanceFog		: packoffset(c2.x);
	float3	Paddy3				: packoffset(c2.y);
};

cbuffer PolyflagVars : register (b2)
{
	// Metallicafan212:	New vars for different effects			
	int		bColorMasked		: packoffset(c0.x);
	float	AlphaReject			: packoffset(c0.y);
	float	BWPercent			: packoffset(c0.z);
	int		bAlphaEnabled		: packoffset(c0.w);
	// Metallicafan212: Temp hack until I recode gamma to be screen-based again, using a different algo
	int		bModulated			: packoffset(c1.x);
	float3	Pad					: packoffset(c1.y);
};

cbuffer TextureVariables : register (b3)
{
	int4	bTexturesBound[TEX_ARRAY_SIZE]			: packoffset(c0);
};

// Metallicafan212:	Moved from the ResScaling.hlsl shader
// 					From https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Shaders/ColorSpaceUtility.hlsli
float3 SRGBToRec2020(float3 In)
{
	static const float3x3 ConvMat =
    {
        0.627402, 0.329292, 0.043306,
        0.069095, 0.919544, 0.011360,
        0.016394, 0.088028, 0.895578
    };
    return mul(ConvMat, In);
}

// Metallicafan212:	Distance fog shit
//					TODO! A better algorithm????
//					I have just straight ported the assembly code I wrote a while ago (since I'm a lazy fucking bastard)
float DoDistanceFog(float InZ)//float4 InPos)
{
	if(!bDoDistanceFog)
		return 0.0f;
	
	// Metallicafan212:	Calcuate the fog value for the pixel shader
	if(InZ > DistanceFogSettings.y)
	{
		float dFog = ((InZ * -DistanceFogSettings.x) + DistanceFogSettings.y) * DistanceFogSettings.z;//mad(-InPos.z, DistanceFogSettings.x, DistanceFogSettings.y) * DistanceFogSettings.z;

		return saturate(DistanceFogSettings.w - dFog);
	}
	
	return 0.0f;
}

// Metallicafan212:	Global selection color
static float4 SelectionColor;

// Metallicafan212:	HACK!!! To reject black and white on UI tiles, I don't want to have to update all the shaders...
static bool		bRejectBW;

// Metallicafan212:	Color masking is currently unimplemented and may not be reimplemented

// Metallicafan212:	Do masked rejection
#define CLIP_PIXEL(ColorIn) if(!bAlphaEnabled) ColorIn.w = 1.0f; SelectionColor = input.color; clip(ColorIn.w - AlphaReject); if(bDepthDraw) ColorIn.xyz = input.origZ / DepthDrawLimit;/*input.pos.z / 1.2;*/

float4 DoPixelFog(float DistFog, float4 Color)
{
	// Metallicafan212:	Early bail
	if(!bDoDistanceFog || DistFog <= 0.0f)
		return Color;
	
	// Metallicafan212:	Now mix the colors
	//					Again this is just a straight port of the asm I wrote a while ago
	float3 Temp = DistanceFogColor.xyz - Color.xyz;
	Temp *= DistanceFogColor.w;
	
	Temp = (Temp * DistFog) + Color.xyz;
	
	return float4(Temp, Color.w);
}

// Metallicafan212:	Not needed now, the per object gamma was removed
/*
float4 DoGammaCorrection(float4 ColorIn)
{
	if(GammaMode != GM_PerObject || Gamma == 1.0f || bModulated)
		return ColorIn;
	
	float OverGamma = 1.0f / Gamma;
	ColorIn.xyz = pow(abs(ColorIn.xyz), float3(OverGamma, OverGamma, OverGamma));
	
	return ColorIn;
}
*/

float4 DoFinalColor(float4 ColorIn)
{
	// Metallicafan212:	If doing selection, move out the selection color
	if(bDoSelection)
	{
		if(ColorIn.a < AlphaReject)
		{
			return float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		else
		{
			return float4(SelectionColor.xyz, 1.0f);
		}
	}
	
	// Metallicafan212:	In the depth mode, just return back the color (it's handled in the macro above)
	if(bDepthDraw)
	{
		return ColorIn;
	}
	
	// Metallicafan212:	Clamp the color
	ColorIn.xyz = clamp(ColorIn.xyz, 0.0, 1.0);
	
	// Metallicafan212:	Early return
	if(BWPercent <= 0.0f || bRejectBW)
	{
		return ColorIn;
	}
	else
	{
		// Metallicafan212:	Sum and divide by 3
		ColorIn.xyz = lerp(ColorIn.xyz, (ColorIn.x + ColorIn.y + ColorIn.z) / 3.0f, BWPercent);
		return ColorIn;
	}
}
#endif