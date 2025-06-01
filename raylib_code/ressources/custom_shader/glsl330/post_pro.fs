/*//post_pro.fs
#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;

// Output fragment color
out vec4 finalColor;

// Uniform variables
uniform sampler2D texture0;  // Texture de l'écran
uniform float time;          // Temps pour l'animation
uniform int rainEffect;      // Activer/désactiver l'effet de pluie
uniform float rainIntensity; // Intensité de la pluie (0.0 - 1.0)
uniform vec2 resolution;      // Résolution de l'écran

// Fonction de génération de nombres pseudo-aléatoires
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Fonction de bruit basée sur la valeur (value noise)
float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Quatre coins du carré
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Interpolation cubique
    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

// Fonction pour créer une goutte de pluie
float raindrop(vec2 uv, vec2 center, float size, float blur) {
    float dist = length((uv - center) / vec2(size, size * 2.0));
    float dropShape = smoothstep(1.0, 1.0 - blur, dist);

    // Créer des cercles concentriques pour l'effet d'ondulation
    float ripple = sin(dist * 50.0 - time * 10.0) * 0.05;
    dropShape += ripple * smoothstep(1.0, 0.0, dist);

    return dropShape;
}

vec2 dropLayer(vec2 uv, float t, float scale) {
    uv *= scale;
    vec2 id = floor(uv);
    uv = fract(uv) - 0.5;

    vec3 n = vec3(
        fract(sin(dot(id, vec2(27.1, 61.7))) * 43758.5453),
        fract(sin(dot(id, vec2(95.7, 12.4))) * 12345.6789),
        fract(sin(dot(id, vec2(13.4, 77.7))) * 34567.891)
    );

    float x = n.x - 0.5;
    float y = t * 0.5 + n.z;

    float wiggle = sin(y * 10.0 + sin(y * 5.0));
    x += wiggle * (0.5 - abs(x)) * (n.y - 0.5);
    x *= 0.7;

    vec2 p = vec2(x, 0.0);
    float d = length(uv - p);
    float mainDrop = smoothstep(0.3, 0.0, d);

    float trail = smoothstep(0.2, 0.0, abs(uv.x - x)) * (1.0 - smoothstep(0.0, 0.1, uv.y));
    float finalDrop = mainDrop + trail;

    return vec2(finalDrop, trail);
}

vec2 drops(vec2 uv, float t) {
    vec2 layer1 = dropLayer(uv, t, 20.0);  // gouttes grosses
    vec2 layer2 = dropLayer(uv * 1.5, t + 10.0, 40.0); // plus fines

    float final = smoothstep(0.3, 1.0, layer1.x + layer2.x);
    float trail = max(layer1.y, layer2.y);

    return vec2(final, trail);
}


void main() {
    // Échantillonner la texture d'écran
    vec2 uv = fragTexCoord;
    vec4 texColor = texture(texture0, uv);

    if (rainEffect == 1) {
        // Paramètres pour les gouttes de pluie
        float blurAmount = 0.2;
        float distortionStrength = 0.03 * rainIntensity;
        float numDrops = 8.0 + rainIntensity * 8.0; // Plus d'intensité = plus de gouttes

        // Appliquer l'effet de traînées de pluie
        vec2 rainUV = uv;
        rainUV.y += time * 0.05;

        float rainStreak = noise(vec2(rainUV.x * 100.0, rainUV.y * 40.0 + time * 2.0));
        rainStreak = pow(rainStreak, 3.0) * rainIntensity * 0.3;

        // Traînées verticales
        vec2 offset = vec2(0.0, rainStreak * 0.05);
        texColor += texture(texture0, uv + offset) * 0.25;

        // Créer plusieurs gouttes de pluie
        float totalDropEffect = 0.0;

        for (float i = 0.0; i < numDrops; i++) {
            // Position aléatoire basée sur le temps et l'indice
            float randX = random(vec2(i * 0.1, time * 0.05));
            float randY = random(vec2(time * 0.1, i * 0.1));

            // Taille aléatoire
            float dropSize = mix(0.1, 0.3, random(vec2(i, time * 0.2))) * rainIntensity;

            // Position qui glisse vers le bas
            vec2 dropPos = vec2(
                randX,
                mod(randY - time * (0.1 + randX * 0.5), 1.0)
            );

            float drop = raindrop(uv, dropPos, dropSize, blurAmount);

            // Effet de distorsion pour la réfraction
            if (drop > 0.0) {
                vec2 distUV = uv;
                distUV += (drop * distortionStrength) * vec2(sin(time + i), cos(time + i));
                vec4 distColor = texture(texture0, distUV);

                // Mélanger avec la couleur originale
                texColor = mix(texColor, distColor, drop * 0.5);

                // Ajouter un léger éclat
                texColor += drop * 0.05;
                totalDropEffect += drop;
            }
        }

        // Ajouter un léger effet de flou/brouillard quand il pleut
        vec4 blurColor = vec4(0.0);
        float blurSize = 0.001 * rainIntensity;

        blurColor += texture(texture0, uv + vec2(blurSize, 0.0));
        blurColor += texture(texture0, uv + vec2(-blurSize, 0.0));
        blurColor += texture(texture0, uv + vec2(0.0, blurSize));
        blurColor += texture(texture0, uv + vec2(0.0, -blurSize));

        blurColor *= 0.25;

        texColor = mix(texColor, blurColor, rainIntensity * 0.2);

        // Assombrir légèrement l'image quand il pleut
        texColor *= 1.0 - rainIntensity * 0.15;

        // Ajouter une teinte bleuâtre
        texColor = mix(texColor, vec4(0.7, 0.8, 1.0, 1.0) * texColor, rainIntensity * 0.2);
    }

    finalColor = texColor;
}
*/

//post_pro.fs
#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;

// Output fragment color
out vec4 fragColor;

// Uniform variables
uniform sampler2D texture0;  // Texture de l'écran
uniform float time;          // Temps pour l'animation
uniform int rainEffect;      // Activer/désactiver l'effet de pluie
uniform float rainIntensity; // Intensité de la pluie (0.0 - 1.0)
uniform vec2 resolution;      // Résolution de l'écran
uniform sampler2D texture1; //texture de bruit

void main() {
    vec2 u = fragTexCoord,
         //n = vec2(random(u * 0.1), random(u * 0.1 + vec2(1.0)));  // Displacement
         n = texture(texture1, u * .1).rg; 
    
    vec4 f = textureLod(texture1, u, 2.5);//textureLod(texture0, u, 2.5);
    int rainEffecte = 1;
    // Check if rain effect is enabled
    if (rainEffecte == 1) {
        // Loop through the different inverse sizes of drops
        for (float r = 4. ; r > 0. ; r--) {
            vec2 x = resolution.xy * r * 0.015 * rainIntensity,  // Number of potential drops (in a grid)
                 p = 6.28 * u * x + (n - 0.5) * 2.0,
                 s = sin(p);
            
            // Current drop properties. Coordinates are rounded to ensure a
            // consistent value among the fragment of a given drop.
            vec4 d = texture(texture1, round(u * x - 0.25) / x);
            
            // Drop shape and fading
            float t = (s.x+s.y) * max(0.0, 1.0 - fract(time * (d.b + 0.1) + d.g) * 2.0);
            
            // d_r -> only x% of drops are kept on, with x depending on the size of drops
            //if (d.r < (5.0-r)*0.08 && t > 0.5) {
            if (d.r < (5.0 - r) * 0.15 && t > 0.2){

                // Drop normal
                vec3 v = normalize(-vec3(cos(p), mix(0.2, 2.0, t-0.5)));
                
                // Poor man's refraction (no visual need to do more)
                //f = texture(texture0, u - v.xy * 0.3 * rainIntensity);
                f = texture(texture0, u - v.xy * 0.6 * rainIntensity);

            }
        }
        
        // Add a slight bluish tint for rain atmosphere
        f = mix(f, vec4(0.7, 0.8, 1.0, 1.0) * f, rainIntensity * 0.2);
        fragColor = f;
    } else {
        // If effect is disabled, simply display the texture
        //f = texture(texture0, fragTexCoord);
                //fragColor = vec4(1.0, 0.0, 0.0, 1.0); // Rouge si rainEffect est activé
                fragColor = f;

    }
    
    //fragColor = f;
}
