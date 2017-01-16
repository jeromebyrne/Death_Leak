//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
matrix World;
matrix View;
matrix Projection;
float alpha;
float3 lightDir;
float4 lightColor;

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
	
	//float4 sepiaColor;

	// sepiaColor.r = (color.r * 0.393) + (color.g * 0.769) + (color.b * 0.189);
	// sepiaColor.g = (color.r * 0.349) + (color.g * 0.686) + (color.b * 0.168);
	// sepiaColor.b = (color.r * 0.272) + (color.g * 0.534) + (color.b * 0.131);

	// sepiaColor.r = (color.r * 0.293) + (color.g * 0.669) + (color.b * 0.089);
	// sepiaColor.g = (color.r * 0.249) + (color.g * 0.586) + (color.b * 0.068);
	// sepiaColor.b = (color.r * 0.172) + (color.g * 0.434) + (color.b * 0.031);

	float4 finalColor = { 1.0, 1.0, 1.0, 1.0 };

	float sepiaTrue = step((color.r - (color.g + color.b)), 0.5);
	float redTrue = step(0.5, (color.r - (color.g + color.b)));
	finalColor.r = (color.r) + (redTrue * 5.0);
	finalColor.g = (color.g) + (redTrue * 5.0);
	finalColor.b = (color.b) + (redTrue * 5.0);

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


