// HLSL illustrative independent snippets. GPU and FXC/DXC compile NOT RUN.
cbuffer PerFrame : register(b0) { row_major float4x4 worldViewProj; float4 sunDirection; float4 sunColor; };
TextureCube skyTexture : register(t0);
SamplerState skySampler : register(s0);
struct VSIn { float3 pos : POSITION; float3 normal : NORMAL; };
struct VSOut { float4 clip : SV_POSITION; float3 local : TEXCOORD0; float3 n : TEXCOORD1; };
VSOut VSMain(VSIn input) { VSOut o; o.clip=mul(float4(input.pos,1),worldViewProj); o.local=input.pos; o.n=input.normal; return o; }
float4 PSSky(VSOut input) : SV_Target { return skyTexture.Sample(skySampler, normalize(input.local)); }
float4 PSLambert(VSOut input) : SV_Target {
    float3 N=normalize(input.n);
    float3 towardSun=normalize(-sunDirection.xyz); // convention: sunDirection points where sunlight travels
    return float4(saturate(dot(N,towardSun))*sunColor.rgb + 0.12f, 1);
}
