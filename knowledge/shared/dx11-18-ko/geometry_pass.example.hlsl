struct VSOut { float4 pos : SV_POSITION; float3 normal : NORMAL; float2 uv : TEXCOORD0; };
struct GBOut { float4 albedo : SV_Target0; float4 normal : SV_Target1; }; // [한국어 해설] SV_Target0/1을 서로 다른 렌더 타깃에 기록한다. 호스트의 MRT 바인딩·포맷이 맞아야 한다.
Texture2D DiffuseMap : register(t0);
SamplerState LinearSampler : register(s0);
GBOut PSGeometry(VSOut i) { // [한국어 해설] 법선 공간과 인코딩 규약을 조명 패스와 합의해야 한다. 여기에는 조명 패스 전체가 포함되지 않는다.
    GBOut o;
    o.albedo = DiffuseMap.Sample(LinearSampler, i.uv);
    o.normal = float4(normalize(i.normal)*0.5f+0.5f, 1.f); // [한국어 해설] 정규화 법선을 [-1,1]에서 [0,1]로 인코딩한다. 0법선·공간 변환은 호출 경계에서 다룬다.
    return o;
}
// Lighting pass의 Pixel Shader는 GBuffer를 t0/t1 등 SRV로 읽어 조명 처리.
