#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

out VertexOutput
{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
} vs_Output;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;

void main()
{
	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
	vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0));
    vs_Output.Normal = mat3(u_Transform) * a_Normal;
    vs_Output.TexCoord = a_TexCoord;
}

#type fragment
#version 430

layout(location = 0) out vec4 fragColor;

in VertexOutput
{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
} vs_Input;

struct DirectionalLight
{
	vec3 Direction;
	vec3 Radiance;
	float Multiplier;
};

uniform DirectionalLight u_DirectionalLights;
uniform vec3 u_CameraPosition;

uniform sampler2D u_AlbedoTexture;
uniform float u_AlbedoTexToggle;
uniform vec3 u_AlbedoColor;

void main()
{
	float ambientStrength = 0.3;
	vec3 ambient = ambientStrength * u_DirectionalLights.Radiance;

	vec3 normal = normalize(vs_Input.Normal);
	vec3 lightDir = normalize(u_DirectionalLights.Direction); 
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * u_DirectionalLights.Radiance;

	vec4 albedo = u_AlbedoTexToggle > 0.5 ? texture2D(u_AlbedoTexture, vs_Input.TexCoord) : vec4(u_AlbedoColor, 1.0);

	vec3 result = (ambient + diffuse) * albedo.rgb;
	fragColor = vec4(result, 1.0);
}