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
	vec3 Binormal;
	vec2 TexCoord;
	vec4 LightSpacePosition;
} vs_Output;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;
uniform mat4 u_LightSpaceMatrix;

void main()
{
	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
	vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0));
    vs_Output.Normal = mat3(u_Transform) * a_Normal;
    vs_Output.Binormal = a_Bitangent;
	vs_Output.TexCoord = a_TexCoord;	
	vs_Output.LightSpacePosition = u_LightSpaceMatrix * vec4(vs_Output.WorldPosition, 1.0);
}

#type fragment
#version 430

layout(location = 0) out vec4 fragColor;

in VertexOutput
{
	vec3 WorldPosition;
    vec3 Normal;
	vec3 Binormal;
	vec2 TexCoord;
	vec4 LightSpacePosition;
} fs_Input;

struct DirectionalLight
{
	vec3 Direction;
	vec3 Radiance;
	float Intensity;
};

uniform DirectionalLight u_DirectionalLight;
uniform vec3 u_CameraPosition;

uniform vec3 u_AlbedoColor;
uniform float u_AlbedoTexToggle;
uniform sampler2D u_AlbedoTexture;

uniform sampler2D u_ShadowMapTexture;

float CalculateShadow(vec4 lightSpacePosition)
{
	vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
	projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture2D(u_ShadowMapTexture, projCoords.xy).r;
	float currentDepth = projCoords.z;
	
	vec3 normal = normalize(fs_Input.Normal);
	vec3 lightDir = normalize(u_DirectionalLight.Direction); 
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

	float shadow = 0.0; 
	vec2 texelSize = 1.0 / textureSize(u_ShadowMapTexture, 0); 
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture2D(u_ShadowMapTexture, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
	}	

	if(projCoords.z > 1.0)
        shadow = 0.0;

	return shadow;
}

void main()
{
	float ambientStrength = 0.3;
	vec3 ambient = ambientStrength * u_DirectionalLight.Radiance;

	vec3 normal = normalize(fs_Input.Normal);
	vec3 lightDir = normalize(u_DirectionalLight.Direction); 
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * u_DirectionalLight.Radiance;

	float specularStrength = 0.5;
	vec3 viewDir = normalize(u_CameraPosition - fs_Input.WorldPosition);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
	vec3 specular = specularStrength * spec * u_DirectionalLight.Radiance;

	vec4 albedo = u_AlbedoTexToggle > 0.5 ? texture2D(u_AlbedoTexture, fs_Input.TexCoord) : vec4(u_AlbedoColor, 1.0);
	float shadow = CalculateShadow(fs_Input.LightSpacePosition);

	vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * albedo.rgb * u_DirectionalLight.Intensity;
	fragColor = vec4(result, 1.0);
}