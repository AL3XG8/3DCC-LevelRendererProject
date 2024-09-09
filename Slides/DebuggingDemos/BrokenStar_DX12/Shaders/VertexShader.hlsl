struct OUT_VERTEX 
{
	float3 posH : SV_POSITION;
	float4 clr : COLOR;
};

// an ultra simple hlsl vertex shader
OUT_VERTEX main(float4 inputPos : POSITION, float2 inputColor : COLOR)
{
	OUT_VERTEX output = (OUT_VERTEX)0;
	output.posH = float4(inputPos, 0, 0, 1);
	output.clr = inputColor.rgba;
	return output;
}