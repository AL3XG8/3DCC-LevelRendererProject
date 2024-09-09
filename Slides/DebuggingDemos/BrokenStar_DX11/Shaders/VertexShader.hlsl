struct OUT_VERTEX 
{
	float2 posH : SV_POSITION;
	float3 clr : COLOR;
};

// an ultra simple hlsl vertex shader
OUT_VERTEX main(float3 inputPos : POSITION, float2 inputColor : COLOR)
{
	OUT_VERTEX output = (OUT_VERTEX) 0;
	output.posH = float4(inputPos, 1);
	output.clr = inputColor;
	return output;
}