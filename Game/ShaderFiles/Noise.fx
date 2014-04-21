//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
matrix World;
matrix View;
matrix Projection;
float alpha;
float3 lightDir;
float4 lightColor;

// Noise vars
float fTimer;
float fNoiseAmount;
int iSeed;

Texture2D txDiffuse;
SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
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
    // Distortion factor
	float NoiseX = iSeed * fTimer * sin(input.Tex.x * input.Tex.y + fTimer);
	NoiseX = fmod(NoiseX,8) * fmod(NoiseX,4);
	
	// Use our distortion factor to compute how much it will affect each
	// texture coordinate
	float DistortX = fmod(NoiseX,fNoiseAmount);
	float DistortY = fmod(NoiseX,fNoiseAmount+0.002);
	
	// Create our new texture coordinate based on our distortion factor
	float2 DistortTex = float2(DistortX,DistortY);

	// Use our new texture coordinate to look-up a pixel in ColorMapSampler.
	float4 Color = txDiffuse.Sample( samLinear, input.Tex + DistortTex);

	if ((Color.r + Color.b + Color.g) > 3.0) // if the color is completely black then we can assume alpha so clear it out
	{
		Color.a = 0;
	}

	Color.a *= alpha;
   
    return Color;
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


