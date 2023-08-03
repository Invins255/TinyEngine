#type compute
#version 450 core

const float PI = 3.14159265359;

layout(binding=0) uniform samplerCube inputTexture;
layout(binding=0, rgba16f) restrict writeonly uniform imageCube outputTexture;

vec3 GetCubeMapTexCoord()
{
    vec2 st = gl_GlobalInvocationID.xy / vec2(imageSize(outputTexture));
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

    vec3 ret;
    if (gl_GlobalInvocationID.z == 0)      ret = vec3(  1.0, uv.y, -uv.x);
    else if (gl_GlobalInvocationID.z == 1) ret = vec3( -1.0, uv.y,  uv.x);
    else if (gl_GlobalInvocationID.z == 2) ret = vec3( uv.x,  1.0, -uv.y);
    else if (gl_GlobalInvocationID.z == 3) ret = vec3( uv.x, -1.0,  uv.y);
    else if (gl_GlobalInvocationID.z == 4) ret = vec3( uv.x, uv.y,   1.0);
    else if (gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y,  -1.0);
    return normalize(ret);
}

layout(local_size_x=32, local_size_y=32, local_size_z=1) in;
void main()
{
    vec3 N = GetCubeMapTexCoord();
    
    vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), N));
    vec3 up    = normalize(cross(N, right));

    vec3 irradiance = vec3(0.0, 0.0, 0.0);
    float sampleDelta = 0.015;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(inputTexture, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    imageStore(outputTexture, ivec3(gl_GlobalInvocationID), vec4(irradiance, 1.0));
}