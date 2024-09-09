// an ultra simple hlsl vertex shader
#pragma pack_matrix(row_major)

// TODO: Part 1F
struct VertexIn
{
    float4 pos : POSITION;
    float3 uvw : TEXCOORD;
    float3 nrm : NORMAL;
};

struct VertexOut
{
    float4 posH : SV_POSITION; // Homogeneous Projection Space Position For Rasterization
    float3 uvwW : TEXCOORD; // UVWorld Space UV For Lighting
    float3 nrmW : NORMAL; // World Space Normal Lor Lighting
    float3 posW : WORLD; // World Space Position For Lighting
};

//float4 main(VertexIn input) : SV_POSITION
//{
//    return float4(input.pos, 1.0f);
//}

// TODO: Part 1H 
//float4 main(VertexIn input) : SV_POSITION
//{
//    // Adjust the Y coordinate down by -0.75 and the Z coordinate by +0.75
//    float3 adjustedPosition = float3(input.pos.x, input.pos.y - 0.75f, input.pos.z + 0.75f);

//    return float4(adjustedPosition, 1.0f);
//}

// TODO: Part 2B 
struct OBJ_ATTRIBUTES
{
    float3 Kd; // Diffuse reflectivity
    float d; // Dissolve (transparency)
    float3 Ks; // Specular reflectivity
    float Ns; // Specular exponent
    float3 Ka; // Ambient reflectivity
    float sharpness; // Local reflection map sharpness
    float3 Tf; // Transmission filter
    float Ni; // Optical density (index of refraction)
    float3 Ke; // Emissive reflectivity
    uint illum; // Illumination model
};

cbuffer MeshData : register(b0)
{
    float4x4 worldMatrix;
    OBJ_ATTRIBUTES material;
};

cbuffer SceneData : register(b1)
{
    float4 sunDirection, sunColor, SunAmbient, cameraPos;
    float4x4 viewProjectionMatrix;
};

// TODO: Part 2D 
// TODO: Part 4A 
// TODO: Part 4B 
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

//float4 main(float2 inputVertex : POSITION) : SV_POSITION <-------------- Original
//{
//	return float4(inputVertex, 0, 1);
//}