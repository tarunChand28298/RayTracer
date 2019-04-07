RWTexture2D<float4> pixels : register(u0);
struct Sphere
{
    float3 position;
    float radius;
    float3 albedo;
    float3 specular;
};
StructuredBuffer<Sphere> spheres : register(t0);

struct Ray
{
    float3 origin;
    float3 direction;
    float3 energy;
};
Ray CreateRay(float3 origin, float3 direction)
{
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    ray.energy = float3(1.0f, 1.0f, 1.0f);
    return ray;
}

struct RayHit
{
    float3 position;
    float distance;
    float3 normal;
    float3 albedo;
    float3 specular;
};
RayHit CreateRayHit()
{
    RayHit hit;
    hit.position = float3(0.0f, 0.0f, 0.0f);
    hit.distance = 1.#INF;
    hit.normal = float3(0.0f, 0.0f, 0.0f);
    hit.albedo = float3(0.0f, 0.0f, 0.0f);
    hit.specular = float3(0.0f, 0.0f, 0.0f);
    return hit;
}

Ray CreateCameraRay(float2 uv)
{
    
    float4x4 cameraToWorld = float4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.97f, 0.21f, 8.8f, 0.0f, 0.21f, -0.97f, -17.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    float4x4 cameraInverseProjection = float4x4(0.76f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.6f, 1.6f);

    float3 origin = mul(cameraToWorld, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    float3 direction = mul(cameraInverseProjection, float4(uv, 0.0f, 1.0f)).xyz;

    direction = mul(cameraToWorld, float4(direction, 0.0f)).xyz;
    direction = normalize(direction);

    return CreateRay(origin, direction);
}

void IntersectGroundPlane(Ray ray, inout RayHit bestHit)
{
    float3 groundAlbedo = float3(0.1f, 0.1f, 0.1f);
    float3 groundSpecular = float3(0.4f, 0.4f, 0.4f);

    float t = -ray.origin.y / ray.direction.y;
    if (t > 0 && t < bestHit.distance)
    {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = float3(0.0f, 1.0f, 0.0f);
        bestHit.albedo = groundAlbedo;
        bestHit.specular = groundSpecular;
    }
}
void IntersectSphere(Ray ray, inout RayHit bestHit, Sphere sphere)
{
    float3 d = ray.origin - sphere.position;
    float p1 = -dot(ray.direction, d);
    float p2sqr = p1 * p1 - dot(d, d) + sphere.radius * sphere.radius;
    if (p2sqr < 0)
        return;
    float p2 = sqrt(p2sqr);
    float t = p1 - p2 > 0 ? p1 - p2 : p1 + p2;
    if (t > 0 && t < bestHit.distance)
    {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = normalize(bestHit.position - sphere.position);
        bestHit.albedo = sphere.albedo;
        bestHit.specular = sphere.specular;
    }
}

RayHit Trace(Ray ray)
{
    RayHit bestHit = CreateRayHit();
    IntersectGroundPlane(ray, bestHit);

    uint numberOfSpheres;
    uint stride;
    spheres.GetDimensions(numberOfSpheres, stride);
    
    for (int i = 0; i < numberOfSpheres; i++) 
    {
        IntersectSphere(ray, bestHit, spheres[i]);
    }

    return bestHit;
}
float3 Shade(inout Ray ray, RayHit hit)
{
    float4 directionalLight = float4(-0.3, -0.8, 0.6, 1);

    if (hit.distance < 1.#INF)
    {

        ray.origin = hit.position + hit.normal * 0.001f;
        ray.direction = reflect(ray.direction, hit.normal);
        ray.energy *= hit.specular;

        bool shadow = false;
        Ray shadowRay = CreateRay(hit.position + hit.normal * 0.001f, -1 * directionalLight.xyz);
        RayHit shadowHit = Trace(shadowRay);
        if (shadowHit.distance != 1.#INF)
        {
            return float3(0.0f, 0.0f, 0.0f);
        }

        return saturate(dot(hit.normal, directionalLight.xyz) * -1) * directionalLight.w * hit.albedo;
    }
    else
    {
        ray.energy = 0.0f;
        return float3(0.5f, 0.5f, 0.5f);
    }
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width, height;
    pixels.GetDimensions(width, height);
    float2 uv = float2((DTid.xy + float2(0.5f, 0.5f)) / float2(width, height) * 2.0f - 1.0f);
    uv.y = -uv.y;

    float3 result = float3(0, 0, 0);

    //===================================================================================
    
    Ray ray = CreateCameraRay(uv);
    for (int i = 0; i < 5; i++)
    {
        RayHit hit = Trace(ray);
        result += ray.energy * Shade(ray, hit);
        if (!any(ray.energy))
            break;
    }

    //===================================================================================
    pixels[DTid.xy] = float4(result, 1);
}
