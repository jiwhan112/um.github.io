struct VSOut { float4 pos : SV_POSITION; float3 normal : NORMAL; float2 uv : TEXCOORD0; };
struct GBOut { float4 albedo : SV_Target0; float4 normal : SV_Target1; };
Texture2D DiffuseMap : register(t0);
SamplerState LinearSampler : register(s0);
GBOut PSGeometry(VSOut i) {
    GBOut o;
    o.albedo = DiffuseMap.Sample(LinearSampler, i.uv);
    o.normal = float4(normalize(i.normal)*0.5f+0.5f, 1.f);
    return o;
}
// Lighting pass의 Pixel Shader는 GBuffer를 t0/t1 등 SRV로 읽어 조명 처리.
