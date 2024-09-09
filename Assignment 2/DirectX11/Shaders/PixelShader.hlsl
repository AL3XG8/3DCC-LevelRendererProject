// an ultra simple hlsl pixel shader
#pragma pack_matrix(row_major)

// TODO: Part 3A 
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

//float4 main() : SV_TARGET
//{
//    return float4(material.Kd, material.d);
//}

// TODO: Part 4B
struct PixelIn
{
    float4 posH : SV_POSITION;
    float3 uvwW : TEXCOORD;
    float3 nrmW : NORMAL;
    float3 posW : WORLD;
};

// TODO: Part 4F 
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
    float3 ambient = material.Kd * SunAmbient.rgb;
    float3 color = diffuse + ambient + specular;

    return float4(color, material.d);
}
 
//float4 main() : SV_TARGET <----------------------------------------------------- Original
//{
//	//return float4(0.25f,0.0f,1.0f,0);
    
//    // TODO: Part 1A (optional)
//    return float4(0.5f, 0.0f, 1.0f, 0);
//}