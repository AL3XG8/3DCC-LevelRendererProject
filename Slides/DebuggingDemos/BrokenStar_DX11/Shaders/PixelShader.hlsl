// an ultra simple hlsl pixel shader
float4 main(float4 posH : SV_POSITION, float3 inputColor : COLOR) : SV_TARGET
{
	return float4(inputColor, 0, 1);
}