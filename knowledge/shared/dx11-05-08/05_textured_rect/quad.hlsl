struct VSInput { float3 position : POSITION; float2 uv : TEXCOORD0; };
struct PSInput { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
Texture2D colorTexture : register(t0);
SamplerState colorSampler : register(s0);
PSInput VSMain(VSInput input) { PSInput o; o.position=float4(input.position,1); o.uv=input.uv; return o; }
float4 PSMain(PSInput input) : SV_Target { return colorTexture.Sample(colorSampler,input.uv); }
