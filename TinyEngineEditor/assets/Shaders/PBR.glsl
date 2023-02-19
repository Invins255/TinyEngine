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
	mat3 WorldTransform;
	mat3 WorldNormals;
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
	vs_Output.WorldTransform = mat3(u_Transform);
	vs_Output.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Bitangent, a_Normal);
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
	mat3 WorldTransform;
	mat3 WorldNormals;
} fs_Input;

struct DirectionalLight
{
	vec3 Direction;
	vec3 Radiance;
	float Intensity;
};

uniform DirectionalLight u_DirectionalLight;
uniform vec3 u_CameraPosition;

//-------------------------------------------------------------
//Material textures
//-------------------------------------------------------------
uniform vec3 u_AlbedoColor;
uniform float u_AlbedoTexToggle;
uniform sampler2D u_AlbedoTexture;

uniform float u_NormalTexToggle;
uniform sampler2D u_NormalTexture;

uniform float u_Roughness;
uniform float u_RoughnessTexToggle;
uniform sampler2D u_RoughnessTexture;

uniform float u_Metalness;
uniform float u_MetalnessTexToggle;
uniform sampler2D u_MetalnessTexture;
//-------------------------------------------------------------

uniform sampler2D u_ShadowMapTexture;

const float PI = 3.14159265359;
const vec3 Fdielectric = vec3(0.4);

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = NdotH2 * (a2 - 1.0) + 1.0;
	denom = PI * denom * denom;
	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggxV = GeometrySchlickGGX(NdotV, roughness);
    float ggxL = GeometrySchlickGGX(NdotL, roughness);

    return ggxV * ggxL;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

struct PBRParameters
{
	vec3 Albedo;
	float Roughness;
	float Metalness;
	vec3 F0;			//Fresnel reflectance

	vec3 Normal;
	vec3 View;
};
PBRParameters params;

vec3 CalculateLight()
{
	vec3 result = vec3(0.0);

	//For each light
	vec3 L = normalize(u_DirectionalLight.Direction);
	vec3 H = normalize(L + params.View);
	vec3 radiance = u_DirectionalLight.Radiance * u_DirectionalLight.Intensity;

	float D = DistributionGGX(params.Normal, H, params.Roughness);
	float G = GeometrySmith(params.Normal, params.View, L, params.Roughness);
	vec3 F = FresnelSchlick(max(dot(H, params.View), 0.0), params.F0);

	float cosL = max(0.0, dot(params.Normal, L));
	float cosV = max(0.0, dot(params.Normal, params.View));

	//Specular (Cook-Torrance)
	vec3 nom = D * G * F;
	float denom = 4.0 * cosL * cosV + 0.0001; //+0.0001 to prevent divide by 0
	vec3 specularBRDF = nom / denom; 

	//Diffuse
	vec3 kd = (1.0 - F) * (1.0 - params.Metalness);
	vec3 diffuseBRDF = kd * params.Albedo / PI;

	result += (diffuseBRDF + specularBRDF) * radiance * cosL;
	return result;
}

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
	params.Albedo = u_AlbedoTexToggle > 0.5 ? texture2D(u_AlbedoTexture, fs_Input.TexCoord).rgb : u_AlbedoColor;
	params.Roughness = u_RoughnessTexToggle > 0.5 ? texture2D(u_RoughnessTexture, fs_Input.TexCoord).r : u_Roughness;
	params.Metalness = u_MetalnessTexToggle > 0.5 ? texture2D(u_MetalnessTexture, fs_Input.TexCoord).r : u_Metalness; 
	params.F0 = mix(Fdielectric, params.Albedo, vec3(params.Metalness));

	params.Normal = normalize(fs_Input.Normal);
	if(u_NormalTexToggle > 0.5)
	{
		params.Normal = normalize(2.0 * texture2D(u_NormalTexture, fs_Input.TexCoord).rgb - 1.0);
		params.Normal = normalize(fs_Input.WorldNormals * params.Normal);
	}
	params.View = normalize(u_CameraPosition - fs_Input.WorldPosition);

	vec3 ambient = vec3(0.03) * params.Albedo;	
	vec3 Lo = CalculateLight();

	vec3 result = ambient + Lo;
	//Gamma correct
	result = pow(result, vec3(1.0 / 2.2));
	fragColor = vec4(result, 1.0);
}