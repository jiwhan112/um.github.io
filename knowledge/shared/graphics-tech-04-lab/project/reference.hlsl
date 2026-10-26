// 셰이더 통합용 보조 함수. 독립 실행 셰이더나 GPU 검증 결과가 아니다.
Texture2D<float> ShadowDepth : register(t0);
SamplerComparisonState ShadowComparison : register(s0);
// 호스트 계약: LESS_EQUAL, COMPARISON_MIN_MAG_MIP_POINT, 유효한 0..1 depth.
float Pcf3x3(float2 uv, float receiverDepth, float bias)
{
    if (any(uv < 0.0) || any(uv >= 1.0)) return 1.0;
    uint width, height;
    ShadowDepth.GetDimensions(width, height);
    float2 texel = 1.0 / float2(width, height);
    float visibility = 0.0;
    [unroll] for (int y = -1; y <= 1; ++y)
    {
        [unroll] for (int x = -1; x <= 1; ++x)
        {
            float2 tapUv = uv + float2(x, y) * texel;
            // CPU 실습과 같은 밝은 border 정책을 명시한다.
            visibility += any(tapUv < 0.0) || any(tapUv >= 1.0)
                ? 1.0 : ShadowDepth.SampleCmpLevelZero(
                    ShadowComparison, tapUv, receiverDepth - bias);
        }
    }
    return visibility / 9.0;
}
