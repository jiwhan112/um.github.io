// shader_contract.fx — legacy Direct3D9 Effect sample
float4x4 gWorldViewProj;
float4x4 gWorldView;
float4 gFogColor = float4(0.2,0.3,0.4,1);
float gFogStart = 2.0f, gFogEnd = 8.0f;
texture gDiffuseTexture;
sampler2D DiffuseSampler = sampler_state {
    Texture = <gDiffuseTexture>;
    MinFilter=LINEAR; MagFilter=LINEAR; MipFilter=LINEAR;
    AddressU=WRAP; AddressV=WRAP;
};
struct VS_IN { float4 pos:POSITION0; float2 uv:TEXCOORD0; };
struct VS_OUT { float4 clip:POSITION0; float2 uv:TEXCOORD0; float viewZ:TEXCOORD1; };
struct PS_IN { float2 uv:TEXCOORD0; float viewZ:TEXCOORD1; };
VS_OUT VSMain(VS_IN a) {
    VS_OUT b;
    b.clip=mul(a.pos,gWorldViewProj);
    b.uv=a.uv;
    b.viewZ=mul(a.pos,gWorldView).z;
    return b;
}
float4 PSMain(PS_IN a):COLOR0 {
    float4 tex=tex2D(DiffuseSampler,a.uv);
    float keep=saturate((gFogEnd-a.viewZ)/max(gFogEnd-gFogStart,0.0001f));
    return float4(lerp(gFogColor.rgb,tex.rgb,keep),tex.a);
}
technique FoggedTextured {
 pass P0 {
  VertexShader=compile vs_2_0 VSMain();
  PixelShader=compile ps_2_0 PSMain();
  ZEnable=TRUE; ZWriteEnable=TRUE; AlphaBlendEnable=FALSE;
 }
}