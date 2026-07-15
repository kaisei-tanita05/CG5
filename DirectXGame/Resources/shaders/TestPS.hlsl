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
    
    //grayScale
    float32_t value = dot(textureColor.rgb, float32_t3(0.2125, 0.7154, 0.0721));
    
    // 位置セット(x y z w) が カラーセット(r g b a)にアクセスできる
    output.color = float32_t4(value, value, value, textureColor.a);
    return output;
}