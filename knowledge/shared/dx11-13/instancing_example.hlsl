cbuffer Frame : register(b0) { row_major float4x4 ViewProj; };
struct VSIn {
 float3 position : POSITION; float2 uv : TEXCOORD0;
 float4 r0 : INSTANCEWORLD0; float4 r1 : INSTANCEWORLD1;
 float4 r2 : INSTANCEWORLD2; float4 r3 : INSTANCEWORLD3;
};
struct VSOut { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
VSOut VSMain(VSIn i) {
 float4x4 world = float4x4(i.r0,i.r1,i.r2,i.r3);
 VSOut o;
 o.position = mul(mul(float4(i.position,1),world),ViewProj);
 o.uv=i.uv; return o;
}
float4 PSMain(VSOut i): SV_Target { return float4(i.uv,0,1); }
