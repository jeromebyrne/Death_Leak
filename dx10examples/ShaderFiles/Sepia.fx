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
	// Use our new texture coordinate to look-up a pixel in ColorMapSampler.
	float4 Color = txDiffuse.Sample( samLinear, input.Tex);
	
	float4 finalColor;
	// Apply Sepia after distortion
	finalColor.r = (Color.r * 0.393) + (Color.g * 0.769) + (Color.b * 0.189);
    finalColor.g = (Color.r * 0.349) + (Color.g * 0.686) + (Color.b * 0.168);    
    finalColor.b = (Color.r * 0.272) + (Color.g * 0.534) + (Color.b * 0.131);
	
	// Keep our alphachannel at 1.
	finalColor.a = alpha;
   
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


