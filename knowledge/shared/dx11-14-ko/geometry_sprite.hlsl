struct VSOut {
    float4 pos : SV_POSITION;
    float2 halfExtentNdc : TEXCOORD0;
};
struct GSOut {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};
[maxvertexcount(4)]
void GS(point VSOut src[1], inout TriangleStream<GSOut> stream) { // [한국어 해설] point 입력을 triangle strip으로 확장하는 GS 참조다. CPU 출력 cap 모델과 실제 maxvertexcount는 구분한다.
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
        outV.pos.xy += offsets[i] * src[0].halfExtentNdc * outV.pos.w; // [한국어 해설] NDC 반폭에 clip w를 곱해 원근 나눗셈 뒤 일정한 화면 크기를 유지한다.
        outV.uv = uv[i];
        stream.Append(outV);
    }
    stream.RestartStrip(); // [한국어 해설] sprite마다 strip을 끊어 다음 점에서 생성한 정점과 삼각형이 이어지지 않도록 한다.
}
