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
	vec4 LightCascadePosition[4];
	vec3 ViewPosition;
} vs_Output;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_Transform;
uniform mat4 u_LightSpaceMatrix;
uniform mat4 u_LightCascadeMatrix0;
uniform mat4 u_LightCascadeMatrix1;
uniform mat4 u_LightCascadeMatrix2;
uniform mat4 u_LightCascadeMatrix3;

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

	vs_Output.LightCascadePosition[0] = u_LightCascadeMatrix0 * vec4(vs_Output.WorldPosition, 1.0);
	vs_Output.LightCascadePosition[1] = u_LightCascadeMatrix1 * vec4(vs_Output.WorldPosition, 1.0);
	vs_Output.LightCascadePosition[2] = u_LightCascadeMatrix2 * vec4(vs_Output.WorldPosition, 1.0);
	vs_Output.LightCascadePosition[3] = u_LightCascadeMatrix3 * vec4(vs_Output.WorldPosition, 1.0);	 
	vs_Output.ViewPosition = vec3(u_ViewMatrix * vec4(vs_Output.WorldPosition, 1.0));
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
	vec4 LightCascadePosition[4];
	vec3 ViewPosition;
} fs_Input;

struct DirectionalLight
{
	vec3 Direction;
	vec3 Radiance;
	float Intensity;
	int ShadowsType;
	int SamplingRadius;
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

uniform sampler2D u_ShadowMapTextures[4];
uniform vec4 u_CascadeSplits;

//-------------------------------------------------------------
//Environment
//-------------------------------------------------------------
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_EnvPrefliteredMap;
uniform sampler2D u_BRDFLUTMap;
//-------------------------------------------------------------

const float PI = 3.14159265359;
const vec3 Fdielectric = vec3(0.4);

//-------------------------------------------------------------
//PBR
//-------------------------------------------------------------
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

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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

vec3 IBL()
{
	float NdotV = max(dot(params.Normal, params.View), 0.0);

	vec3 ks = FresnelSchlickRoughness(NdotV, params.F0, params.Roughness);
	vec3 kd = (1.0 - ks) * (1.0 - params.Metalness);
	vec3 diffuseIrradiance = texture(u_IrradianceMap, params.Normal).rgb;	
	vec3 diffuse = kd * diffuseIrradiance * params.Albedo;	

	vec3 R = reflect(-params.View, params.Normal);
	int texLevels = textureQueryLevels(u_EnvPrefliteredMap);
	vec3 specularIrradiance = textureLod(u_EnvPrefliteredMap, R, params.Roughness * texLevels).rgb;

	vec2 specularBRDF = texture2D(u_BRDFLUTMap, vec2(NdotV, params.Roughness)).rg;
	vec3 specular = specularIrradiance * (ks * specularBRDF.x + specularBRDF.y);

	return diffuse + specular;
}

//-------------------------------------------------------------
//Shadow
//-------------------------------------------------------------

const int MaxCascadeCount = 4;

float GetBias(vec3 normal, vec3 lightDir)
{
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	return bias;
}


//Hard Shadows
float HardShadows(sampler2D shadowMapTexture, vec3 projCoords)
{
	float closestDepth = texture2D(shadowMapTexture, projCoords.xy).r;
	float currentDepth = projCoords.z;

	vec3 normal = normalize(fs_Input.Normal);
	vec3 lightDir = normalize(u_DirectionalLight.Direction); 
	float bias = GetBias(normal, lightDir);

	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	if(projCoords.z > 1.0)
        shadow = 0.0;

	return shadow;
}

const vec2 PoissonDistribution[64] = vec2[](
	vec2(-0.884081, 0.124488),
	vec2(-0.714377, 0.027940),
	vec2(-0.747945, 0.227922),
	vec2(-0.939609, 0.243634),
	vec2(-0.985465, 0.045534),
	vec2(-0.861367, -0.136222),
	vec2(-0.881934, 0.396908),
	vec2(-0.466938, 0.014526),
	vec2(-0.558207, 0.212662),
	vec2(-0.578447, -0.095822),
	vec2(-0.740266, -0.095631),
	vec2(-0.751681, 0.472604),
	vec2(-0.553147, -0.243177),
	vec2(-0.674762, -0.330730),
	vec2(-0.402765, -0.122087),
	vec2(-0.319776, -0.312166),
	vec2(-0.413923, -0.439757),
	vec2(-0.979153, -0.201245),
	vec2(-0.865579, -0.288695),
	vec2(-0.243704, -0.186378),
	vec2(-0.294920, -0.055748),
	vec2(-0.604452, -0.544251),
	vec2(-0.418056, -0.587679),
	vec2(-0.549156, -0.415877),
	vec2(-0.238080, -0.611761),
	vec2(-0.267004, -0.459702),
	vec2(-0.100006, -0.229116),
	vec2(-0.101928, -0.380382),
	vec2(-0.681467, -0.700773),
	vec2(-0.763488, -0.543386),
	vec2(-0.549030, -0.750749),
	vec2(-0.809045, -0.408738),
	vec2(-0.388134, -0.773448),
	vec2(-0.429392, -0.894892),
	vec2(-0.131597, 0.065058),
	vec2(-0.275002, 0.102922),
	vec2(-0.106117, -0.068327),
	vec2(-0.294586, -0.891515),
	vec2(-0.629418, 0.379387),
	vec2(-0.407257, 0.339748),
	vec2(0.071650, -0.384284),
	vec2(0.022018, -0.263793),
	vec2(0.003879, -0.136073),
	vec2(-0.137533, -0.767844),
	vec2(-0.050874, -0.906068),
	vec2(0.114133, -0.070053),
	vec2(0.163314, -0.217231),
	vec2(-0.100262, -0.587992),
	vec2(-0.004942, 0.125368),
	vec2(0.035302, -0.619310),
	vec2(0.195646, -0.459022),
	vec2(0.303969, -0.346362),
	vec2(-0.678118, 0.685099),
	vec2(-0.628418, 0.507978),
	vec2(-0.508473, 0.458753),
	vec2(0.032134, -0.782030),
	vec2(0.122595, 0.280353),
	vec2(-0.043643, 0.312119),
	vec2(0.132993, 0.085170),
	vec2(-0.192106, 0.285848),
	vec2(0.183621, -0.713242),
	vec2(0.265220, -0.596716),
	vec2(-0.009628, -0.483058),
	vec2(-0.018516, 0.435703)
);

vec2 SamplePoisson(int index)
{
   return PoissonDistribution[index % 64];
}

//PCF
float PCF(sampler2D shadowMapTexture, vec3 projCoords, int radius)
{
	float closestDepth = texture2D(shadowMapTexture, projCoords.xy).r;
	float currentDepth = projCoords.z;

	vec3 normal = normalize(fs_Input.Normal);
	vec3 lightDir = normalize(u_DirectionalLight.Direction); 
	float bias = GetBias(normal, lightDir);

	float shadow = 0.0; 
	vec2 texelSize = 1.0 / textureSize(shadowMapTexture, 0); 

	int NUM_SAMPLES = 64;
	for(int i = 0; i < NUM_SAMPLES; i++)
	{
		float pcfDepth = texture2D(shadowMapTexture, projCoords.xy + radius * SamplePoisson(i) * texelSize).r; 
		shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
	}
	shadow /= NUM_SAMPLES;

	if(projCoords.z > 1.0)
        shadow = 0.0;

	return shadow;
}

float averageBlockerDistance(vec3 projCoords, int radius)
{
	int blockers = 0;
	float blockerDistance = 0;

	vec3 normal = normalize(fs_Input.Normal);
	vec3 lightDir = normalize(u_DirectionalLight.Direction); 
	float bias = GetBias(normal, lightDir);

	vec2 texelSize = 1.0 / textureSize(u_ShadowMapTexture, 0);

	int NUM_SAMPLES = 64;
	for(int i = 0; i < NUM_SAMPLES; i++)
	{
		float dist = texture2D(u_ShadowMapTexture, projCoords.xy + radius * SamplePoisson(i) * texelSize).r;
		if(dist < projCoords.z - bias)
		{
			blockers++;
			blockerDistance += dist;
		}
	}

	if(blockers == 0)
		return 1.0;

	return blockerDistance / blockers;
}

//PCSS
float PCSS(sampler2D shadowMapTexture, vec3 projCoords, int radius)
{
	float wLight = 10.0;
	float zReceiver = projCoords.z;

	//step1: Calculate average blocker distance
	float zBlocker = averageBlockerDistance(projCoords, radius);	
	//step2: Calculate pernumbra size
	float wPenumbra = (zReceiver - zBlocker) * wLight / zBlocker;
	//step3: PCF filtering 
	float shadow = PCF(shadowMapTexture, projCoords, int(wPenumbra));

	return shadow;
}

int cascadeIndex = 0;

float CalculateShadow(sampler2D shadowMapTexture, vec4 lightSpacePosition)
{
	vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;	
	projCoords = projCoords * 0.5 + 0.5;

	float shadow;
	switch(u_DirectionalLight.ShadowsType)
	{
		case 0:
			shadow = HardShadows(shadowMapTexture, projCoords);
			break;
		case 1:
			shadow = PCF(shadowMapTexture, projCoords, u_DirectionalLight.SamplingRadius);
			break;
		case 2:
			shadow = PCSS(shadowMapTexture, projCoords, u_DirectionalLight.SamplingRadius);
			break;
	}		
	return shadow;
}

float CalculateShadow_CSM()
{
	if(abs(fs_Input.ViewPosition.z) < abs(u_CascadeSplits[3])) 
		cascadeIndex = 3;		
	if(abs(fs_Input.ViewPosition.z) < abs(u_CascadeSplits[2])) 
		cascadeIndex = 2;		
	if(abs(fs_Input.ViewPosition.z) < abs(u_CascadeSplits[1])) 
		cascadeIndex = 1;		
	if(abs(fs_Input.ViewPosition.z) < abs(u_CascadeSplits[0])) 
		cascadeIndex = 0;

	//TODO: 边界阴影处理不够好
	const float cascadeTransitionFade = 10.0;
	float c0 = smoothstep(u_CascadeSplits[0] + cascadeTransitionFade, u_CascadeSplits[0] - cascadeTransitionFade, fs_Input.ViewPosition.z);
	float c1 = smoothstep(u_CascadeSplits[1] + cascadeTransitionFade, u_CascadeSplits[1] - cascadeTransitionFade, fs_Input.ViewPosition.z);
	float c2 = smoothstep(u_CascadeSplits[2] + cascadeTransitionFade, u_CascadeSplits[2] - cascadeTransitionFade, fs_Input.ViewPosition.z);
	
	float shadow;
	if (c0 > 0.0 && c0 < 1.0)
	{	
		//Sample 0 and 1
		float shadow0 = CalculateShadow(u_ShadowMapTextures[0], fs_Input.LightCascadePosition[0]);
		float shadow1 = CalculateShadow(u_ShadowMapTextures[1], fs_Input.LightCascadePosition[1]);		
		shadow = mix(shadow0, shadow1, 1.0);

	}
	else if (c1 > 0.0 && c1 < 1.0)
	{
		//Sample 1 and 2
		float shadow1 = CalculateShadow(u_ShadowMapTextures[1], fs_Input.LightCascadePosition[1]);
		float shadow2 = CalculateShadow(u_ShadowMapTextures[2], fs_Input.LightCascadePosition[2]);		
		shadow = mix(shadow1, shadow2, 1.0);
	}
	else if (c2 > 0.0 && c2 < 1.0)
	{
		//Sample 2 and 3
		float shadow2 = CalculateShadow(u_ShadowMapTextures[2], fs_Input.LightCascadePosition[2]);
		float shadow3 = CalculateShadow(u_ShadowMapTextures[3], fs_Input.LightCascadePosition[3]);		
		shadow = mix(shadow2, shadow3, 1.0);
	}
	else
	{
		shadow = CalculateShadow(u_ShadowMapTextures[cascadeIndex], fs_Input.LightCascadePosition[cascadeIndex]);
	}	
	return shadow;
}

//-------------------------------------------------------------

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

	//Ambient
	vec3 ambient = IBL();	
	//Lights
	vec3 Lo = CalculateLight();
	//Shadows
	//float shadow = CalculateShadow(u_ShadowMapTexture, fs_Input.LightSpacePosition);
	float shadow = CalculateShadow_CSM();

	vec3 color = ambient + Lo * (1 - shadow);
	
	const float gamma = 2.2;
	const float pureWhite = 1.0;
	const float exposure = 1.0;

	//HDR mapped
	// Reinhard tonemapping operator.
	// see: "Photographic Tone Reproduction for Digital Images", eq. 4
	color *= exposure;
	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);
	vec3 mappedColor = (mappedLuminance / luminance) * color;
	//vec3 mappedColor = vec3(1.0) - exp(color * exposure);

	//Gamma correct
	vec3 result = pow(mappedColor, vec3(1.0 / gamma));
	fragColor = vec4(result, 1.0);		


		
	if(cascadeIndex == 3)
		fragColor *= vec4(1.0, 0.0, 0.0, 1.0);
	else if(cascadeIndex == 2)
		fragColor *= vec4(0.0, 1.0, 0.0, 1.0);
	else if(cascadeIndex == 1)
		fragColor *= vec4(0.0, 0.0, 1.0, 1.0);
	else if(cascadeIndex == 0)
		fragColor *= vec4(1.0, 1.0, 0.0, 1.0);
	
	
}