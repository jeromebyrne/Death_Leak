//#define threshold 0.04
//SamplerState texSampler : register(s0);
//Texture2D<float> depthTex : register(t0);

//float4 EdgeDetectionPS(float4 position : SV_POSITION, float2 texcoord : TEXCOORD0) : SV_TARGET {
        //float fCenter = depthTex.Sample(texSampler, texcoord);
        //float4 fEdges = {
          //      depthTex.Sample(texSampler, texcoord, int2(-1, 0)),
           //     depthTex.Sample(texSampler, texcoord, int2(0, -1)),
            //    depthTex.Sample(texSampler, texcoord, int2(1, 0)),
             //   depthTex.Sample(texSampler, texcoord, int2(0, 1)) // Line 11
        //};

        //float4 delta = abs(fCenter.xxxx - fEdges);
        //float4 edges = step(threshold / 10.0, delta);

        //if (dot(edges, 1.0) == 0.0)
          //      discard;

        //return edges;}

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
matrix World;
matrix View;
matrix Projection;
float alpha;
float3 lightDir;
float4 lightColor;

float Time;
float Waviness;

Texture2D txDiffuse;
SamplerState samLinear
{
	Filter = MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
    float3 Norm : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Norm : NORMAL;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
    
    output.Pos.x += sin( output.Pos.y*0.1f + Time ) * Waviness; // wobble algorithm
    output.Pos.y += sin( output.Pos.x*0.1f + Time ) * Waviness; // wobble algorithm
    
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    output.Norm = mul( input.Norm, World );
    output.Tex = input.Tex;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	// EDGE DETECT CODE
    /*float4 finalColor = 0;

    float4 textureSample = txDiffuse.Sample( samLinear, input.Tex );
       
    float4 s00 = txDiffuse.Sample( samLinear, input.Tex, int2(-1, -1));
    float4 s01 = txDiffuse.Sample( samLinear, input.Tex, int2(0, -1));
    float4 s02 = txDiffuse.Sample( samLinear, input.Tex, int2(1, -1));
    float4 s10 = txDiffuse.Sample( samLinear, input.Tex, int2(-1, 0));
    float4 s12 = txDiffuse.Sample( samLinear, input.Tex, int2(1, 0));
    float4 s20 = txDiffuse.Sample( samLinear, input.Tex, int2(-1, 1));
    float4 s21 = txDiffuse.Sample( samLinear, input.Tex, int2(0, 1));
    float4 s22 = txDiffuse.Sample( samLinear, input.Tex, int2(1, 1)); 
    
    float sobelX = s00 + 1 * s10 + s20 - s02 - 1 * s12 - s22;
	float sobelY = s00 + 1 * s01 + s02 - s20 - 1 * s21 - s22;

	
	float threshold = 0.6f;
	float edgeSqr = (sobelX * sobelX + sobelY * sobelY);
	float result = 1.0 - (edgeSqr > threshold * threshold); //white background, black lines
	
	if (result < 0.2)
	{
		return result;
	}
	else
	{	
		return textureSample * alpha;
	}
	
	return result;*/

	float4 textureSample = saturate(lightColor) * txDiffuse.Sample(samLinear, input.Tex);
	textureSample.a *= alpha;

	return textureSample;
}
//--------------------------------------------------------------------------------------
technique10 Render
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}


