// an ultra simple hlsl vertex shader
#pragma pack_matrix(row_major)

// TODO: Part 1C 
struct VertexIn
{
    float4 Pos : POSITION; // Use float4 for the position
};

struct VertexOut
{
    float4 Pos : SV_POSITION; // Use float4 for the output position
};

//VertexOut main(VertexIn vin) //<------------------------------------- Original
//{
//    VertexOut vout;
//    vout.Pos = vin.Pos;
//    return vout;
//}

// TODO: Part 2B 
//cbuffer SHADER_VARS : register(b0)
//{
//    float4x4 world;
//}

// TODO: Part 2F 
//VertexOut main(VertexIn vin)
//{
//    VertexOut vout;
//    vout.Pos = mul(vin.Pos, world);
//    return vout;
//}

// TODO: Part 2G
//cbuffer SHADER_VARS : register(b0)
//{
//    float4x4 world;
//    float4x4 view;
//    float4x4 projection; // Added for Part 3A
//}

//VertexOut main(VertexIn vin)
//{
//    VertexOut vout;
//    vout.Pos = mul(vin.Pos, world);
//    vout.Pos = mul(vout.Pos, view);
//    vout.Pos = mul(vout.Pos, projection);
//    return vout;
//}

// TODO: Part 3B 
cbuffer SHADER_VARS : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection; // Added for Part 3A
    float4x4 viewProjection;
}

//VertexOut main(VertexIn vin)
//{
//    VertexOut vout;
//    float4 worldPos = mul(vin.Pos, world);
//    float4 viewPos = mul(worldPos, view);
//    float4 projPos = mul(viewPos, projection);
//    vout.Pos = projPos;
//    //vout.Pos = mul(worldPos, view);   
//    return vout;
//}

VertexOut main(VertexIn vin)
{
    VertexOut vout;
    float4 worldPos = mul(vin.Pos, world);
    vout.Pos = mul(worldPos, viewProjection);
    return vout;
}

//float4 main(float4 inputVertex : POSITION) : SV_POSITION <----------- Original
//{
//    return inputVertex;
//}