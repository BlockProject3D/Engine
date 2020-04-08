Texture2D<float4> BaseTexture : register(t0);
sampler BaseTextureSampler : register(s0);

struct PSInput
{
    float2 TexCoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET0
{
    float4 color = BaseTexture.Sample(BaseTextureSampler, input.TexCoord);
    return (/*color **/ float4(input.TexCoord, 1, 1));
}
