#include "Test.hlsli"

Texture2D<float32_t4> gtexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    
    float32_t2 uv = input.texcoord;
    float32_t4 textureColor = gtexture.Sample(gSampler, uv);

    // https://learn.microsoft.com/ja-jp/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math
    // 位置セット(x y z w) が カラーセット(r g b a)にアクセスできる
    output.color = textureColor;
    return output;
}