#type vertex
#version 460

layout(location = 0) in vec3 a_Position;

uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

out vec3 TexCoord;

void main()
{
    TexCoord = a_Position;
    vec4 pos =  u_ProjectionMatrix * u_ViewMatrix * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
}

#type fragment
#version 460

layout(location = 0) out vec4 fragColor;

in vec3 TexCoord;

uniform samplerCube u_Skybox;

void main()
{
    fragColor = vec4(texture(u_Skybox, TexCoord).rgb, 1.0);
}