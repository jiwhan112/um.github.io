// 셰이더 통합용 보조 함수. 이 실행에서는 HLSL 컴파일과 GPU 실행을 하지 않았다.
// 입력 RGB는 선형 SDR [0,1], alpha는 선형 합성 가중치라는 계약이다.
float3 EncodeSrgb(float3 linearRgb)
{
    float3 low = 12.92 * linearRgb;
    float3 high = 1.055 * pow(linearRgb, 1.0 / 2.4) - 0.055;
    return float3(linearRgb.r <= 0.0031308 ? low.r : high.r,
                  linearRgb.g <= 0.0031308 ? low.g : high.g,
                  linearRgb.b <= 0.0031308 ? low.b : high.b);
}
float4 EncodeOutput(float4 linearRgba)
{
    // sRGB RTV가 인코딩한다면 이 함수를 다시 적용하지 않는다.
    return float4(EncodeSrgb(linearRgba.rgb), linearRgba.a);
}
