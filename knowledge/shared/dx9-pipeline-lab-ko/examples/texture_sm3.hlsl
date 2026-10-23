// Reference only: NOT compiled with FXC and NOT executed on a GPU in this revision.
// D3D9: compile VSMain as vs_3_0 and PSMain as ps_3_0 in your Windows toolchain.
// Host contract: POSITION0 float3, TEXCOORD0 float2; four float4 ROWS at c0;
// a 2D texture and sampler states at s0; valid target/depth/output state.
// The CPU lab uses double arrays: do NOT upload their bytes as float registers.
// [한국어 해설] 호스트는 float4 행 네 개를 c0부터 전달해야 한다. CPU double 배열을 그대로 업로드하면 안 된다.
row_major float4x4 gWorldViewProjection : register(c0);
sampler2D gAlbedo : register(s0);

// [한국어 해설] 정점 선언과 POSITION0 float3/TEXCOORD0 float2 의미·형식을 맞춰야 한다.
struct VSInput {
    float3 position : POSITION0;
    float2 uv : TEXCOORD0;
};
struct VSOutput {
    float4 position : POSITION0;
    float2 uv : TEXCOORD0;
};

// [한국어 해설] 동차 w를 그대로 출력하여 하드웨어가 클리핑과 원근 보간을 수행하게 한다.
VSOutput VSMain(VSInput input) {
    VSOutput output;
    // Keep homogeneous w for hardware clipping and perspective interpolation.
    output.position = mul(float4(input.position, 1.0), gWorldViewProjection);
    output.uv = input.uv;
    return output;
}

// [한국어 해설] s0의 텍스처와 샘플러 상태를 호스트가 준비해야 한다. FXC 컴파일·GPU 실행은 이번에 하지 않았다.
float4 PSMain(float2 uv : TEXCOORD0) : COLOR0 {
    return tex2D(gAlbedo, uv);
}

