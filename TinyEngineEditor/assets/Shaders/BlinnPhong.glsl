#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;

out VertexOutput
{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
} vs_Output;

void main()
{
	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
	vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0));
    vs_Output.Normal = mat3(u_Transform) * a_Normal;
    vs_Output.TexCoord = a_TexCoord;
}

#type fragment
#version 430

layout(location = 0) out vec4 color;

uniform vec3 u_CameraPosition;

in VertexOutput
{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
} vs_Input;

void main()
{
	color = vec4(0.8, 0.3, 0.2, 1.0);
}