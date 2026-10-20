struct VSOut {
    float4 pos : SV_POSITION;
    float2 halfExtentNdc : TEXCOORD0;
};
struct GSOut {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};
[maxvertexcount(4)]
void GS(point VSOut src[1], inout TriangleStream<GSOut> stream) {
    float2 offsets[4] = {
        float2(-1,-1), float2(-1,1),
        float2( 1,-1), float2( 1,1)
    };
    float2 uv[4] = {
        float2(0,1), float2(0,0),
        float2(1,1), float2(1,0)
    };
    [unroll] for (uint i=0; i<4; ++i) {
        GSOut outV;
        outV.pos = src[0].pos;
        outV.pos.xy += offsets[i] * src[0].halfExtentNdc * outV.pos.w;
        outV.uv = uv[i];
        stream.Append(outV);
    }
    stream.RestartStrip();
}