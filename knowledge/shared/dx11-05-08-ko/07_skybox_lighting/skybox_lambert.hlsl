// HLSL illustrative independent snippets. GPU and FXC/DXC compile NOT RUN.
cbuffer PerFrame : register(b0) { row_major float4x4 worldViewProj; float4 sunDirection; float4 sunColor; }; // [한국어 해설] 참고용 독립 HLSL 조각이다. CPU/호스트 상수 배치와 입력 공간을 맞춰야 한다.
TextureCube skyTexture : register(t0);
SamplerState skySampler : register(s0);
struct VSIn { float3 pos : POSITION; float3 normal : NORMAL; };
struct VSOut { float4 clip : SV_POSITION; float3 local : TEXCOORD0; float3 n : TEXCOORD1; };
VSOut VSMain(VSIn input) { VSOut o; o.clip=mul(float4(input.pos,1),worldViewProj); o.local=input.pos; o.n=input.normal; return o; } // [한국어 해설] 원문은 normal을 그대로 전달한다. 비균일 월드 변환의 법선 역전치는 별도로 준비해야 한다.
float4 PSSky(VSOut input) : SV_Target { return skyTexture.Sample(skySampler, normalize(input.local)); } // [한국어 해설] cube 방향으로 샘플링한다. 스카이 far-depth 강제는 이 조각에 포함되어 있지 않다.
float4 PSLambert(VSOut input) : SV_Target { // [한국어 해설] sunDirection은 빛이 진행하는 방향이어서 표면에서 광원 쪽은 부호를 반대로 쓴다.
    float3 N=normalize(input.n);
    float3 towardSun=normalize(-sunDirection.xyz); // convention: sunDirection points where sunlight travels
    return float4(saturate(dot(N,towardSun))*sunColor.rgb + 0.12f, 1);
}
