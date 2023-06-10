// Metallicafan212:	MSAA resolving shader, which will disable AA on tiles (z < 0.5f)
#define DO_STANDARD_BUFFER 0
#include "ShaderGlobals.h"


// Metallicafan212:	Constant buffer, but with the added NumAASamples
shared cbuffer CommonBuffer : register (b0)
{
	COMMON_VARS;
	
	// Metallicafan212:	The info we use for this specific shader	
	float	FilterSize				: packoffset(c12.x);
	float	GaussianSigma 			: packoffset(c12.y);
	float	CubicB					: packoffset(c12.z);
	float 	CubicC					: packoffset(c12.w);
	int		FilterType				: packoffset(c13.x);
	float3	Pad3					: packoffset(c13.y);
	float4 	SampleOffsets[8]		: packoffset(c14);
};

// Metallicafan212:	HACK!!!! This includes this twice to define the final color function, as HLSL cannot do out of order compiling
//					The buffer variables have to be defined before they can be used
#define DO_FINAL_COLOR
#include "ShaderGlobals.h"


// Metallicafan212:	Using code from this great writeup here (https://therealmjp.github.io/posts/msaa-resolve-filters/)

static const float Pi = 3.141592654f;
static const float Pi2 = 6.283185307f;
static const float Pi_2 = 1.570796327f;
static const float InvPi = 0.318309886f;
static const float InvPi2 = 0.159154943f;

struct VSInput 
{ 
	float4 pos 		: POSITION0;
	float4 uv		: TEXCOORD0;
	float4 color	: COLOR0;
	float4 fog		: COLOR1;
};

struct PSInput 
{
	float4 pos 		: SV_POSITION0; 
	float2 uv		: TEXCOORD0;
};

// Metallicafan212:	MSAA textures
Texture2DMS<float4> RT		: register(t0);
Texture2DMS<float4>	DST		: register(t1);

PSInput VertShader(VSInput input)
{	
	PSInput output = (PSInput)0;
	
	// Metallicafan212:	Set the W to 1 so matrix math works
	input.pos.w 	= 1.0f;
	
	// Metallicafan212:	Transform it out
	output.pos 		= mul(input.pos, Proj);
	output.uv		= input.uv;
	
	return output;
}

/*
float4 VertShader(in uint VertexID : SV_VertexID) : SV_Position
{
    float4 output = 0.0f;

    if(VertexID == 0)
        output = float4(-1.0f, 1.0f, 1.0f, 1.0f);
    else if(VertexID == 1)
        output = float4(3.0f, 1.0f, 1.0f, 1.0f);
    else if(VertexID == 2)
        output = float4(-1.0f, -3.0f, 1.0f, 1.0f);

    return output;
}
*/

float FilterBox(in float x)
{
    return 1.0f;
}

static float FilterTriangle(in float x)
{
    return saturate(1.0f - x);
}

static float FilterGaussian(in float x)
{
    const float sigma = GaussianSigma;
    const float g = 1.0f / sqrt(2.0f * 3.14159f * sigma * sigma);
    return (g * exp(-(x * x) / (2 * sigma * sigma)));
}

float FilterCubic(in float x, in float B, in float C)
{
    // Rescale from [-2, 2] range to [-FilterWidth, FilterWidth]
    x *= 2.0f;

    float y = 0.0f;
    float x2 = x * x;
    float x3 = x * x * x;
    if(x < 1)
        y = (12 - 9 * B - 6 * C) * x3 + (-18 + 12 * B + 6 * C) * x2 + (6 - 2 * B);
    else if (x <= 2)
        y = (-B - 6 * C) * x3 + (6 * B + 30 * C) * x2 + (-12 * B - 48 * C) * x + (8 * B + 24 * C);

    return y / 6.0f;
}

float FilterSinc(in float x)
{
    float s;

    x *= FilterSize;

    if(x < 0.001f)
        s = 1.0f;
    else
        s = sin(x * Pi) / (x * Pi);

    return s;
}

float FilterBlackmanHarris(in float x)
{
    x = 1.0f - x;

    const float a0 = 0.35875f;
    const float a1 = 0.48829f;
    const float a2 = 0.14128f;
    const float a3 = 0.01168f;
    return saturate(a0 - a1 * cos(Pi * x) + a2 * cos(2 * Pi * x) - a3 * cos(3 * Pi * x));
}

float FilterSmoothstep(in float x)
{
    return 1.0f - smoothstep(0.0f, 1.0f, x);
}

/*
float Filter(in float x)
{
    #if FilterType_ == 0
        return FilterBox(x);
    #elif FilterType_ == 1
        return FilterTriangle(x);
    #elif FilterType_ == 2
        return FilterGaussian(x);
    #elif FilterType_ == 3
        return FilterBlackmanHarris(x);
    #elif FilterType_ == 4
        return FilterSmoothstep(x);
    #elif FilterType_ == 5
        return FilterCubic(x, 1.0, 0.0f);
    #elif FilterType_ == 6
        return FilterCubic(x, 0, 0.5f);
    #elif FilterType_ == 7
        return FilterCubic(x, 1 / 3.0f, 1 / 3.0f);
    #elif FilterType_ == 8
        return FilterCubic(x, CubicB, CubicC);
    #elif FilterType_ == 9
        return FilterSinc(x);
    #endif
}
*/

float Filter(in float x)
{
	switch(FilterType)
	{
		case 0:
		{
			return FilterBox(x);
		}
		
		case 1:
		{
			return FilterTriangle(x);
		}
		
		case 2:
		{
			return FilterGaussian(x);
		}
		
		case 3:
		{
			return FilterBlackmanHarris(x);
		}
		
		case 4:
		{
			return FilterSmoothstep(x);
		}
		
		case 5:
		{
			return FilterCubic(x, 1.0, 0.0f);
		}
		
		case 6:
		{
			return FilterCubic(x, 0, 0.5f);
		}
		
		case 7:
		{
			return FilterCubic(x, 1 / 3.0f, 1 / 3.0f);
		}
		
		case 8:
		{
			return FilterCubic(x, CubicB, CubicC);
		}
		
		case 9:
		{
			return FilterSinc(x);
		}
		
		default:
		{
			return FilterBox(x);
		}
	}
}

float4 PxShader(PSInput input) : SV_TARGET
//float4 PxShader(in float4 Position : SV_Position) : SV_TARGET
{	
	// Metallicafan212:	Sample both the render target and the depth target
	//					TODO! Just return the first sample as a test!!!!
	uint W, H, NumAASamples;
	
	RT.GetDimensions(W, H, NumAASamples);
	
	float2 TextureSize = float2(W, H);
	
	uint SampleRadius 	= (FilterSize / 2.0f) + 0.5f;
	
	/*
	uint2 UVCoords = uint2(W * input.uv.x, H * input.uv.y);
	
	float4 DestColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	
	for(uint i = 0; i < NumAASamples; i++)
	{
		DestColor.xyz += RT.Load(UVCoords, i);
	}
	
	// Metallicafan212:	Now average it
	DestColor.xyz /= NumAASamples;
	
	return DestColor;
	
	//return RT.Load(UVCoords, 7);//float4(RT.Load(UVCoords, 0).xyz, 1.0f);
	*/
	
	float3 sum = 0.0f;
    float totalWeight = 0.0f;
	
	float2 pixelPos = float2(W * input.uv.x, H * input.uv.y);
	
	float2 sampleOffset = float2(0.0f, 0.0f);
	
	/*
	for(uint subSampleIdx = 0; subSampleIdx < NumAASamples; ++subSampleIdx)
    {
		sampleOffset += SampleOffsets[subSampleIdx].xy;
		
		float sampleDist = length(sampleOffset) / (FilterSize / 2.0f);
		if(sampleDist <= 1.0f)
		{
			float3 sample = RT.Load(uint2(pixelPos), subSampleIdx).xyz;
			float weight = Filter(sampleDist);
			sum += sample * weight;
			totalWeight += weight;
		}
	}
	*/

	/*
    for(int y = -SampleRadius; y <= SampleRadius; ++y)
    {
        for(int x = -SampleRadius; x <= SampleRadius; ++x)
        {
            float2 sampleOffset = float2(x, y);
            float2 samplePos = pixelPos + sampleOffset;
            samplePos = clamp(samplePos, 0, TextureSize - 1.0f);

            [unroll]
            for(uint subSampleIdx = 0; subSampleIdx < NumAASamples; ++subSampleIdx)
            {
                sampleOffset += SampleOffsets[subSampleIdx].xy;

                float sampleDist = length(sampleOffset) / (FilterSize / 2.0f);

                [branch]
                if(sampleDist <= 1.0f)
                {
                    float3 sample = RT.Load(uint2(samplePos), subSampleIdx).xyz;

                    float weight = Filter(sampleDist);
                    sum += sample * weight;
                    totalWeight += weight;
                }
            }
        }
    }
	*/

	/*
	//return float4(sum, 1.0f);

    float3 output = sum / max(totalWeight, 0.00001f);
    output = max(output, 0.0f);

    return float4(output, 1.0f);
	*/
	
	for(int i = 0; i < NumAASamples; i++)
	{
		sum += RT.Load(uint2(pixelPos), i).xyz;
	}
	
	return float4(sum / NumAASamples, 1.0f);
}
