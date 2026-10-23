struct VSInput { float3 position : POSITION; float2 uv : TEXCOORD0; };
struct PSInput { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
Texture2D colorTexture : register(t0); // [한국어 해설] 텍스처 t0와 샘플러 s0는 독립 슬롯이다.
SamplerState colorSampler : register(s0);
PSInput VSMain(VSInput input) { PSInput o; o.position=float4(input.position,1); o.uv=input.uv; return o; } // [한국어 해설] 클립 좌표와 UV를 출력한다. 원근 나눗셈을 VS에서 수동으로 하지 않는다.
float4 PSMain(PSInput input) : SV_Target { return colorTexture.Sample(colorSampler,input.uv); } // [한국어 해설] 전체 참조 HLSL이며 이번 셰이더 컴파일·GPU 실행은 하지 않았다.
