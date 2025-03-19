#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightDir;
uniform vec4 lightColor;
uniform vec4 ambient;
uniform vec3 viewPos;
uniform mat4 lightVP;
uniform sampler2D shadowMap;
uniform int shadowMapResolution;
uniform bool toon;
uniform int color_steps;
uniform float edge_thickness;

// Output fragment color
out vec4 finalColor;

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 normal = normalize(fragNormal);
    vec3 lightDirNorm = normalize(lightDir);
    float light_intensity = max(dot(normal, lightDirNorm), 0.0);
    vec3 specular = vec3(0.0);
    vec3 viewD = normalize(viewPos - fragPosition);
    float specCo = pow(max(0.0, dot(viewD, reflect(-lightDirNorm, normal))), 16.0);
    specular += specCo;
    vec4 fragPosLightSpace = lightVP * vec4(fragPosition, 1.0);
    fragPosLightSpace.xyz /= fragPosLightSpace.w;
    fragPosLightSpace.xyz = (fragPosLightSpace.xyz + 1.0) / 2.0;
    vec2 sampleCoords = fragPosLightSpace.xy;
    float curDepth = fragPosLightSpace.z;
    float bias = max(0.0002 * (1.0 - dot(normal, lightDirNorm)), 0.00002) + 0.00001;
    int shadowCounter = 0;
    const int numSamples = 9;
    vec2 texelSize = vec2(1.0 / float(shadowMapResolution));
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float sampleDepth = texture(shadowMap, sampleCoords + texelSize * vec2(x, y)).r;
            if (curDepth - bias > sampleDepth) shadowCounter++;
        }
    }
    float shadowFactor = float(shadowCounter) / float(numSamples);
    float shadowIntensity = 0.8;
    vec3 outline_color = vec3(0.0);
    float outline = smoothstep(0.5 - edge_thickness, 0.5, length(normal));
    if (toon) {
        light_intensity = floor(light_intensity * float(color_steps)) / float(color_steps);
        finalColor = mix(vec4(outline_color, 1.0), texelColor * (colDiffuse + vec4(specular, 1.0)) * vec4(light_intensity, light_intensity, light_intensity, 1.0), outline);
    } else {
        finalColor = texelColor * (colDiffuse + vec4(specular, 1.0)) * vec4(light_intensity, light_intensity, light_intensity, 1.0);
    }
    finalColor = mix(finalColor, vec4(0, 0, 0, 1), shadowFactor * shadowIntensity);
    finalColor += texelColor * (ambient / 10.0) * colDiffuse;
    finalColor = pow(finalColor, vec4(1.0 / 2.2));
}
