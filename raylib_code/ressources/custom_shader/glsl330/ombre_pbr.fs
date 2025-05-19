#version 330

#define MAX_LIGHTS              4
#define LIGHT_DIRECTIONAL       0
#define LIGHT_POINT             1
#define PI 3.14159265358979323846

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
    float intensity;
};

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec4 shadowPos;
in mat3 TBN;

// Output fragment color
out vec4 finalColor;

// Input uniform values
uniform int numOfLights;
uniform sampler2D albedoMap;
uniform sampler2D mraMap;
uniform sampler2D normalMap;
uniform sampler2D emissiveMap; // r: Hight g:emissive
uniform sampler2D shadowMap;   // Ajout de la texture shadowMap

uniform vec2 tiling;
uniform vec2 offset;

uniform int useTexAlbedo;
uniform int useTexNormal;
uniform int useTexMRA;
uniform int useTexEmissive;
uniform int useShadowMap;      // Ajout d'un flag pour activer/désactiver les ombres

uniform vec4  albedoColor;
uniform vec4  emissiveColor;
uniform float normalValue;
uniform float metallicValue;
uniform float roughnessValue;
uniform float aoValue;
uniform float emissivePower;

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec3 viewPos;

uniform vec3 ambientColor;
uniform float ambient;

// Shadow parameters
uniform float shadowBias;      // Bias pour corriger les artefacts d'ombres
uniform int shadowSamples;     // Nombre d'échantillons pour PCF

// Reflectivity in range 0.0 to 1.0
// NOTE: Reflectivity is increased when surface view at larger angle
vec3 SchlickFresnel(float hDotV, vec3 refl)
{
    return refl + (1.0 - refl)*pow(1.0 - hDotV, 5.0);
}

float GgxDistribution(float nDotH, float roughness)
{
    float a = roughness * roughness * roughness * roughness;
    float d = nDotH * nDotH * (a - 1.0) + 1.0;
    d = PI * d * d;
    return a / max(d, 0.0000001);
}

float GeomSmith(float nDotV, float nDotL, float roughness)
{
    float r = roughness + 1.0;
    float k = r*r / 8.0;
    float ik = 1.0 - k;
    float ggx1 = nDotV/(nDotV*ik + k);
    float ggx2 = nDotL/(nDotL*ik + k);
    return ggx1*ggx2;
}

// Fonction pour calculer l'ombre
float ShadowCalculation()
{
    if (useShadowMap == 0) return 1.0;

    // Perspective divide (obtenir les coordonnées normalisées)
    vec3 projCoords = shadowPos.xyz / shadowPos.w;
    
    // Transformer à l'intervalle [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    
    // Vérifier si le fragment est dans les limites de la shadowMap
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z > 1.0) 
        return 1.0;
    
    // Obtenir la profondeur la plus proche du point de vue de la lumière
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Profondeur actuelle du fragment
    float currentDepth = projCoords.z;
    
    // Appliquer un biais pour éviter le shadow acne
    currentDepth -= shadowBias;
    
    // PCF (Percentage Closer Filtering) pour adoucir les ombres
    float shadow = 0.0;
    float texelSize = 1.0 / textureSize(shadowMap, 0).x;
    int sampleRadius = max(1, shadowSamples / 2);
    
    for (int x = -sampleRadius; x <= sampleRadius; ++x) {
        for (int y = -sampleRadius; y <= sampleRadius; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > pcfDepth ? 0.0 : 1.0;
        }
    }
    
    int totalSamples = (2 * sampleRadius + 1) * (2 * sampleRadius + 1);
    shadow /= float(totalSamples);
    
    return shadow;
}

vec3 ComputePBR()
{
    vec3 albedo = texture(albedoMap, vec2(fragTexCoord.x*tiling.x + offset.x, fragTexCoord.y*tiling.y + offset.y)).rgb;
    albedo = vec3(albedoColor.x*albedo.x, albedoColor.y*albedo.y, albedoColor.z*albedo.z);
    
    float metallic = clamp(metallicValue, 0.0, 1.0);
    float roughness = clamp(roughnessValue, 0.0, 1.0);
    float ao = clamp(aoValue, 0.0, 1.0);
    
    if (useTexMRA == 1)
    {
        vec4 mra = texture(mraMap, vec2(fragTexCoord.x*tiling.x + offset.x, fragTexCoord.y*tiling.y + offset.y))*useTexMRA;
        metallic = clamp(mra.r + metallicValue, 0.04, 1.0);
        roughness = clamp(mra.g + roughnessValue, 0.04, 1.0);
        ao = (mra.b + aoValue)*0.5;
    }

    vec3 N = normalize(fragNormal);
    if (useTexNormal == 1)
    {
        N = texture(normalMap, vec2(fragTexCoord.x*tiling.x + offset.x, fragTexCoord.y*tiling.y + offset.y)).rgb;
        N = normalize(N*2.0 - 1.0);
        N = normalize(N*TBN);
    }

    vec3 V = normalize(viewPos - fragPosition);

    vec3 emissive = vec3(0);
    if (useTexEmissive == 1) {
        emissive = texture(emissiveMap, vec2(fragTexCoord.x*tiling.x+offset.x, fragTexCoord.y*tiling.y+offset.y)).g * emissiveColor.rgb*emissivePower;
    }

    // Calcul du facteur d'ombre
    float shadow = ShadowCalculation();

    // if dia-electric use base reflectivity of 0.04 otherwise ut is a metal use albedo as base reflectivity
    vec3 baseRefl = mix(vec3(0.04), albedo.rgb, metallic);
    vec3 lightAccum = vec3(0.0);  // Acumulate lighting lum

    for (int i = 0; i < numOfLights; i++)
    {
        if (lights[i].enabled == 0) continue;
        
        vec3 L = normalize(lights[i].position - fragPosition);      // Compute light vector
        vec3 H = normalize(V + L);                                  // Compute halfway bisecting vector
        float dist = length(lights[i].position - fragPosition);     // Compute distance to light
        float attenuation = 1.0/(dist*dist*0.23);                   // Compute attenuation
        vec3 radiance = lights[i].color.rgb*lights[i].intensity*attenuation; // Compute input radiance, light energy comming in

        // Cook-Torrance BRDF distribution function
        float nDotV = max(dot(N,V), 0.0000001);
        float nDotL = max(dot(N,L), 0.0000001);
        float hDotV = max(dot(H,V), 0.0);
        float nDotH = max(dot(N,H), 0.0);
        float D = GgxDistribution(nDotH, roughness);    // Larger the more micro-facets aligned to H
        float G = GeomSmith(nDotV, nDotL, roughness);   // Smaller the more micro-facets shadow
        vec3 F = SchlickFresnel(hDotV, baseRefl);       // Fresnel proportion of specular reflectance

        vec3 spec = (D*G*F)/(4.0*nDotV*nDotL);
        
        // Difuse and spec light can't be above 1.0
        // kD = 1.0 - kS  diffuse component is equal 1.0 - spec comonent
        vec3 kD = vec3(1.0) - F;
        
        // Mult kD by the inverse of metallnes, only non-metals should have diffuse light
        kD *= 1.0 - metallic;
        
        // Appliquer le facteur d'ombre à la lumière (pas à l'ambiance)
        lightAccum += ((kD*albedo.rgb/PI + spec)*radiance*nDotL) * shadow; 
    }
    
    vec3 ambientFinal = (ambientColor + albedo)*ambient*0.5;
    
    return ambientFinal + lightAccum*ao + emissive;
}

void main()
{
    vec3 color = ComputePBR();

    // HDR tonemapping
    color = pow(color, color + vec3(1.0));
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));

    finalColor = vec4(color, 1.0);
}