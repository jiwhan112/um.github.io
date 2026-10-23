cbuffer Frame : register(b0) { row_major float4x4 ViewProj; }; // [한국어 해설] b0는 row_major ViewProj다. 인스턴스 행렬은 IA의 네 INSTANCEWORLD 입력에서 받는다.
struct VSIn {
 float3 position : POSITION; float2 uv : TEXCOORD0;
 float4 r0 : INSTANCEWORLD0; float4 r1 : INSTANCEWORLD1;
 float4 r2 : INSTANCEWORLD2; float4 r3 : INSTANCEWORLD3;
};
struct VSOut { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
VSOut VSMain(VSIn i) { // [한국어 해설] 행 네 개로 world를 조립하여 위치*world*ViewProj 순서로 변환한다.
 float4x4 world = float4x4(i.r0,i.r1,i.r2,i.r3);
 VSOut o;
 o.position = mul(mul(float4(i.position,1),world),ViewProj);
 o.uv=i.uv; return o;
}
float4 PSMain(VSOut i): SV_Target { return float4(i.uv,0,1); } // [한국어 해설] UV를 색으로 보여 주는 참조 PS다. 이번 HLSL 컴파일·GPU 실행은 하지 않았다.
