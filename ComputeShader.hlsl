
RWTexture2D<float4> pixels;
StructuredBuffer<float4> spheres;

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    pixels[DTid.xy] = float4(float(DTid.x) / 800.0f, 0.0f, float(DTid.y) / 600.0f, 1.0f);
}
