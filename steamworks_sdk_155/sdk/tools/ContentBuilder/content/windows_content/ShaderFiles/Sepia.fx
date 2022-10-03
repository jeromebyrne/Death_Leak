//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
matrix World;
matrix View;
matrix Projection;
float alpha;
float3 lightDir;
float4 lightColor;

float r1mult = 0.293 * 0.75;
float r2mult = 0.249 * 0.75;
float r3mult = 0.172 * 0.75;
float g1mult = 0.669 * 0.75;
float g2mult = 0.586 * 0.75;
float g3mult = 0.434 * 0.75;
float b1mult = 0.089 * 0.75;
float b2mult = 0.068 * 0.75;
float b3mult = 0.031 * 0.75;

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
	// Use our new texture coordinate to look-up a pixel in ColorMapSampler.
	float4 color = txDiffuse.Sample( samLinear, input.Tex);
	
	float4 sepiaColor;

    sepiaColor.r = (color.r * r1mult) + (color.g * g1mult) + (color.b * b1mult);
    sepiaColor.g = (color.r * r2mult) + (color.g * g2mult) + (color.b * b2mult);
    sepiaColor.b = (color.r * r3mult) + (color.g * g3mult) + (color.b * b3mult);

	float4 finalColor = { 1.0, 1.0, 1.0, 1.0 };

	float sepiaTrue = step((color.r - (color.g + color.b)), 0.5);
	float redTrue = step(0.5, (color.r - (color.g + color.b)));
	finalColor.r = (sepiaTrue * sepiaColor.r) + (redTrue * color.r) * 0.6;
    finalColor.g = (sepiaTrue* sepiaColor.g) + (redTrue * color.g);
    finalColor.b = (sepiaTrue* sepiaColor.b) + (redTrue * color.b);

	return finalColor;
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


