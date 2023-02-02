#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

uniform int u_Int;
uniform float u_Float;
uniform vec3 u_Vec3;
uniform vec4 y_Vec4;
uniform mat4 u_MVP;

out vec3 v_Normal;

void main()
{
	gl_Position = u_MVP * vec4(a_Position, 1.0);
	v_Normal = a_Normal;
}


#type fragment
#version 430

layout(location = 0) out vec4 finalColor;

uniform sampler2D texture;
uniform samplerCube cubeMap;
uniform vec3 u_TestColor;

struct Light
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Light light;

in vec3 v_Normal;

void main()
{
	finalColor = vec4(0.8, 0.0, 0.8, 1.0);


	finalColor = vec4((v_Normal * 0.5 + 0.5), 1.0);// * u_Color.xyz, 1.0);
}