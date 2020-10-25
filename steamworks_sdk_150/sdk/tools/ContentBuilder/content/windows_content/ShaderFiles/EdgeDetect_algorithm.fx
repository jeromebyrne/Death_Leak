// I had this as part of another shader before but didn't like the end result. 
// Putting the shader code into a temp doc in case I want to use it again.

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