// an ultra simple hlsl pixel shader
#pragma pack_matrix(row_major)

struct ATTRIBUTES {
    float3 Kd; float d;
    float3 Ks; float Ns;
    float3 Ka; float sharpness;
    float3 Tf; float Ni;
    float3 Ke; uint illum;
};

struct PixelIn
{
    float4 posH : SV_POSITION;
    float3 uvwW : UVW;
    float3 nrmW : NRM;
    float3 posW : WORLD;
};

cbuffer MeshData : register(b0)
{
    float4x4 worldMatrix;
    ATTRIBUTES material;
};

cbuffer SceneData : register(b1)
{
    float4 sunDirection, sunColor, sunAmbient, cameraPos;
    float4x4 viewProjectionMatrix;
};

float4 main(PixelIn pin) : SV_TARGET
{
    float3 normal = normalize(pin.nrmW);
    float3 viewDir = normalize(cameraPos.xyz - pin.posW);
    float3 lightDir = normalize(-sunDirection.xyz);
    float3 reflectDir = reflect(-lightDir, normal);

    float specPower = material.Ns + 0.000001f;
    float specIntensity = pow(max(dot(reflectDir, viewDir), 0.0), specPower);

    float3 diffuse = max(dot(normal, lightDir), 0.0) * material.Kd * sunColor.rgb;
    float3 specular = specIntensity * material.Ks;
    float3 ambient = material.Kd * sunAmbient.rgb;
    float3 color = diffuse + ambient + specular;

    return float4(color, material.d);
}