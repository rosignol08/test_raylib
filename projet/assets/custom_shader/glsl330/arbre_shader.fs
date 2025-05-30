#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float windStrength;  // Add wind strength parameter
uniform float time;          // Add time for wind animation

// Output fragment color
out vec4 finalColor;

// Input lighting values
uniform vec3 lightDir;
uniform vec4 lightColor;
uniform vec4 ambient;
uniform vec3 viewPos;

// Input shadowmapping values
uniform mat4 lightVP;
uniform sampler2D shadowMap;
uniform int shadowMapResolution;

void main()
{
    // Tree-specific parameters
    float leafTransparencyThreshold = 0.5;  // Adjust based on your tree texture
    float leafShadowIntensity = 0.3;        // Lighter shadows for leaves
    float trunkShadowIntensity = 0.7;       // Stronger shadows for trunk
    
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    
    // Lighting calculations
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);
    vec3 l = -lightDir;

    float NdotL = max(dot(normal, l), 0.0);
    lightDot += lightColor.rgb * NdotL;

    // Reduce specular for leaves to give a more matte appearance
    float specCo = 0.0;
    if (NdotL > 0.0 && texelColor.a > 0.9) {  // More specular on trunk than leaves
        specCo = pow(max(0.0, dot(viewD, reflect(-(l), normal))), 16.0);
    }
    specular += specCo;

    finalColor = (texelColor * ((colDiffuse + vec4(specular, 1.0)) * vec4(lightDot, 1.0)));

    // Shadow calculations
    vec4 fragPosLightSpace = lightVP * vec4(fragPosition, 1);
    fragPosLightSpace.xyz /= fragPosLightSpace.w;
    fragPosLightSpace.xyz = (fragPosLightSpace.xyz + 1.0f) / 2.0f;
    vec2 sampleCoords = fragPosLightSpace.xy;
    float curDepth = fragPosLightSpace.z;
    
    float bias = max(0.0002 * (1.0 - dot(normal, l)), 0.00002) + 0.00001;
    int shadowCounter = 0;
    const int numSamples = 9;
    
    vec2 texelSize = vec2(1.0f / float(shadowMapResolution));
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float sampleDepth = texture(shadowMap, sampleCoords + texelSize * vec2(x, y)).r;
            if (curDepth - bias > sampleDepth) {
                shadowCounter++;
            }
        }
    }
    
    // Transparency and shadow handling specific for trees
    float shadowIntensity;
    if (texelColor.a < 0.1) {
        discard;  // Completely transparent parts
    }
    else if (texelColor.a < leafTransparencyThreshold) {
        // Leaf parts (semi-transparent)
        shadowIntensity = leafShadowIntensity;
        gl_FragDepth = gl_FragCoord.z;
    }
    else {
        // Trunk parts (more opaque)
        shadowIntensity = trunkShadowIntensity;
        gl_FragDepth = gl_FragCoord.z;
    }
    
    // Apply shadows
    finalColor = mix(finalColor, vec4(0, 0, 0, 1), (shadowIntensity) * (float(shadowCounter) / float(numSamples)));
    
    // Add a slightly greenish ambient for trees
    vec4 treeAmbient = ambient + vec4(0.02, 0.03, 0.0, 0.0);
    finalColor += texelColor * (treeAmbient/10.0) * colDiffuse;

    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0/2.2));
}
