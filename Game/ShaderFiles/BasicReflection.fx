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

TextureCube txEnvMap;
SamplerState samLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
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
    float3 ViewR : TEXCOORD2;
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
    
    // Calculate the reflection vector
    float3 viewNorm = mul( output.Norm, (float3x3)View );
    output.ViewR = reflect( viewNorm, float3(0,0,-1.0) );
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{    
    // Calculate lighting 
    float fLighting = saturate( dot( (float3)lightDir,input.Norm) * lightColor );
   
    // Load the environment map texture
    float4 cReflect = txEnvMap.Sample( samLinearClamp, input.ViewR );
    
    // Load the diffuse texture and multiply by the lighting amount
    
    float4 textureSample = txDiffuse.Sample( samLinear, input.Tex );
    float4 cDiffuse = textureSample * fLighting;
    
    // Add diffuse to reflection and go
    float4 cTotal = cDiffuse + cReflect;
    cTotal.a = textureSample.a * alpha;
    return cTotal;

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


