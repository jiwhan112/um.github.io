// Reference only: NOT compiled with FXC and NOT executed on a GPU in this revision.
// D3D9: compile VSMain as vs_3_0 and PSMain as ps_3_0 in your Windows toolchain.
// Host contract: POSITION0 float3, TEXCOORD0 float2; four float4 ROWS at c0;
// a 2D texture and sampler states at s0; valid target/depth/output state.
// The CPU lab uses double arrays: do NOT upload their bytes as float registers.
row_major float4x4 gWorldViewProjection : register(c0);
sampler2D gAlbedo : register(s0);

struct VSInput {
    float3 position : POSITION0;
    float2 uv : TEXCOORD0;
};
struct VSOutput {
    float4 position : POSITION0;
    float2 uv : TEXCOORD0;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;
    // Keep homogeneous w for hardware clipping and perspective interpolation.
    output.position = mul(float4(input.position, 1.0), gWorldViewProjection);
    output.uv = input.uv;
    return output;
}

float4 PSMain(float2 uv : TEXCOORD0) : COLOR0 {
    return tex2D(gAlbedo, uv);
}

