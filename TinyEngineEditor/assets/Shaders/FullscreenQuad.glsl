#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
    gl_Position = vec4(a_Position.x, a_Position.y, 0.0, 1.0);
    v_TexCoord = a_TexCoord;
}

#type fragment
#version 430

layout(location = 0) out vec4 FragColor;

in vec2 v_TexCoord;

uniform sampler2D u_ScreenTexture;

void main()
{
    FragColor = texture2D(u_ScreenTexture, v_TexCoord);
}
