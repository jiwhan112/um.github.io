// Shader Model 4.0: row-vector multiplication and explicit row_major payload.
cbuffer Frame : register(b0) {
    row_major float4x4 WorldViewProjection;
    float4 Tint;
};
Texture2D Albedo : register(t0);
SamplerState LinearSampler : register(s0);
struct VSIn {float3 position:POSITION;float2 uv:TEXCOORD0;};
struct VSOut {float4 position:SV_POSITION;float2 uv:TEXCOORD0;};
VSOut VSMain(VSIn input){
    VSOut result;
    result.position=mul(float4(input.position,1.0f),WorldViewProjection);
    result.uv=input.uv;
    return result;
}
float4 PSMain(VSOut input):SV_Target {
    return Albedo.Sample(LinearSampler,input.uv)*Tint;
}
