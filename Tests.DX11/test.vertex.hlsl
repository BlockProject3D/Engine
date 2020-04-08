struct Vertex2D
{
    float2 Position : POSITION;
    float2 TexCoords : TEXCOORDS;
};

struct VSOutput
{
    float2 TexCoord : TEXCOORD;
    float4 Position : SV_POSITION;
};

VSOutput main(Vertex2D input)
{
    VSOutput o;
    o.Position = float4(input.Position, 0, 1);
    o.TexCoord = input.TexCoords;
    return (o);
}
