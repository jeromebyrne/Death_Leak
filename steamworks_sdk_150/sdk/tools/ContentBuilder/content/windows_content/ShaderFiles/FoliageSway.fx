//---------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
matrix World;
matrix View;
matrix Projection;
float alpha;
float4 lightColor;

float Time;
float TimeMultiplier;
float GlobalTimeMultiplier;

Texture2D txDiffuse;
SamplerState samLinear
{
	Filter = MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
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
    
	output.Pos.x += sin(output.Pos.y*0.1f + ((Time * TimeMultiplier) * GlobalTimeMultiplier)) * input.Norm.x;
	output.Pos.y += sin(output.Pos.x*0.1f + ((Time * TimeMultiplier) * GlobalTimeMultiplier)) * input.Norm.y;
    
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
	float4 textureSample = txDiffuse.Sample( samLinear, input.Tex );

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

