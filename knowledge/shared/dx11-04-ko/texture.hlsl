// Shader Model 4.0: row-vector multiplication and explicit row_major payload.
cbuffer Frame : register(b0) { // [한국어 해설] b0의 row_major 행렬과 Tint를 CPU 구조체 배치에 맞춰 전달해야 한다.
    row_major float4x4 WorldViewProjection;
    float4 Tint;
};
Texture2D Albedo : register(t0);
SamplerState LinearSampler : register(s0);
struct VSIn {float3 position:POSITION;float2 uv:TEXCOORD0;};
struct VSOut {float4 position:SV_POSITION;float2 uv:TEXCOORD0;};
VSOut VSMain(VSIn input){ // [한국어 해설] 동차 위치를 반환하고 UV는 래스터 보간으로 넘긴다.
    VSOut result;
    result.position=mul(float4(input.position,1.0f),WorldViewProjection);
    result.uv=input.uv;
    return result;
}
float4 PSMain(VSOut input):SV_Target { // [한국어 해설] t0 텍스처와 s0 샘플러를 별도 바인딩한다. 이번 FXC/GPU 실행은 하지 않았다.
    return Albedo.Sample(LinearSampler,input.uv)*Tint;
}
