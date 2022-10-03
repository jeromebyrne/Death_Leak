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
Texture2D txBump;

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
    float2 Tex : TEXCOORD0;
    float3 Norm : NORMAL;
	float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Norm : NORMAL;
	float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
	input.Pos.w = 1.0f;

    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    output.Tex = input.Tex;

	output.Norm = mul( input.Norm, (float3x3)World );
	output.Norm = normalize(output.Norm);

	// Calculate the tangent vector against the world matrix only and then normalize the final value.
    output.tangent = mul(input.tangent, (float3x3)World);
    output.tangent = normalize(output.tangent);

	// Calculate the binormal vector against the world matrix only and then normalize the final value.
    output.binormal = mul(input.binormal, (float3x3)World);
    output.binormal = normalize(output.binormal);
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
    float4 finalColor = 0;
    
    float4 textureSample = txDiffuse.Sample( samLinear, input.Tex );

	float4 bumpMap = txBump.Sample( samLinear, input.Tex);

	bumpMap = (bumpMap * 2.0f) - 1.0f;

	float3 bumpNormal = input.Norm + bumpMap.x * input.tangent + bumpMap.y * input.binormal;

	// Normalize the resulting bump normal.
	bumpNormal = normalize(bumpNormal);

		// Invert the light direction for calculations.
	float3 lightDirection = -lightDir;

	// Calculate the amount of light on this pixel based on the bump map normal value.
	float lightIntensity = saturate(dot(bumpNormal, lightDirection));

	// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
	float4 color = saturate(lightColor * lightIntensity);

	color.a = 1.0;

	finalColor = textureSample * color;

	finalColor.a = finalColor.a * alpha;

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


