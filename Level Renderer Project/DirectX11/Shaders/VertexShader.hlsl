// an ultra simple hlsl vertex shader
#pragma pack_matrix(row_major)

struct ATTRIBUTES {
    float3 Kd; float d;
    float3 Ks; float Ns;
    float3 Ka; float sharpness;
    float3 Tf; float Ni;
    float3 Ke; uint illum;
};

struct VertexIn
{
    float4 pos : POS;
    float3 uvw : UVW;
    float3 nrm : NRM;
};

struct VertexOut
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

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    
    float4 worldPos = mul(float4(vin.pos.xyz, 1.0), worldMatrix);
    vout.posH = mul(worldPos, viewProjectionMatrix);
    vout.posW = worldPos.xyz;
    
    float3x3 normalMatrix = (float3x3) worldMatrix;
    vout.nrmW = normalize(mul(vin.nrm, normalMatrix));
    vout.uvwW = vin.uvw;
    
    return vout;
}
