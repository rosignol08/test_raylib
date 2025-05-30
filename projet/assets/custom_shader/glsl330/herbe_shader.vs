#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// Paramètres pour l'effet de vent
uniform float windStrength = 0.1;      // Force du vent
uniform float windSpeed = 1.0;         // Vitesse du vent
uniform float time;                    // Temps (fourni par l'application)
uniform int isGrass = 0;               // Indicateur si c'est de l'herbe (1) ou non (0) si c'est 2 c'est un arbre


// Paramètres de bruit adaptés du shader Godot
uniform sampler2D noiseTexture;        // Texture de bruit
uniform float windTextureTileSize = 2.0;
uniform float windVerticalStrength = 0.3;
uniform vec2 windHorizontalDirection = vec2(1.0, 0.5);

// Nouveaux paramètres pour le bruit de Perlin
//uniform sampler2D noiseTexture;        // Texture de bruit
//uniform float noiseScale = 10.0;       // Échelle du bruit
// Paramètres ajustables
float noiseInfluence = 0.7;     // Influence du bruit (0.0 - 1.0)
float sineInfluence = 0.3;      // Influence du mouvement sinusoïdal (0.0 - 1.0)
float verticalDroop = 0.4;      // Affaissement vertical (0.0 - 1.0)


void main() {
    
    //Copie de la position initiale
    vec3 position = vertexPosition;
    
    if (isGrass == 1) {
        vec3 modifiedNormal = normalize(vec3(matNormal * vec4(vertexNormal, 1.0)));

    float height = vertexTexCoord.y;
    
    // Obtenir les coordonnées en espace monde pour ce vertex
    vec3 worldPos = vec3(matModel * vec4(position, 1.0));
    
    // Paramètres pour les rafales
    float gustStrength = 0.8;
    float gustFrequency = 0.2;
    float gustSpeed = 0.5;
    
    // Direction du vent
    vec2 windDir = normalize(windHorizontalDirection);
    
    // Coordonnées pour le bruit (windSpeed affecte la vitesse d'échantillonnage)
    vec2 noiseCoord = vec2(
        worldPos.x / windTextureTileSize + time * windSpeed * windDir.x,
        worldPos.z / windTextureTileSize + time * windSpeed * windDir.y
    );
    
    vec2 noiseCoord2 = vec2(
        worldPos.x / (windTextureTileSize * 0.5) + time * windSpeed * 0.7 * windDir.x,
        worldPos.z / (windTextureTileSize * 0.5) + time * windSpeed * 0.7 * windDir.y
    );
    
    vec2 gustCoord = vec2(
        worldPos.x / (windTextureTileSize * 2.0) + time * windSpeed * gustSpeed * windDir.x,
        worldPos.z / (windTextureTileSize * 2.0) + time * windSpeed * gustSpeed * windDir.y
    );
    
    // Échantillonnage des bruits
    float noise = texture(noiseTexture, noiseCoord).r - 0.5;
    float noise2 = texture(noiseTexture, noiseCoord2).r - 0.5;
    float gust = texture(noiseTexture, gustCoord).r;
    
    // Mélanger les bruits
    noise = noise * 0.7 + noise2 * 0.3;
    gust = smoothstep(0.6, 0.9, gust) * gustStrength;
    
    // Variation spatiale
    float spatialVariation = fract(sin(worldPos.x * 12.3 + worldPos.z * 15.7) * 43758.5453);
    float phaseOffset = spatialVariation * 6.28;
    
    // *** CORRECTION: windSpeed affecte maintenant directement le mouvement ***
    float timeScale = time * windSpeed; // Facteur de temps basé sur windSpeed
    
    // Force de vent modulée
    float effectiveWindStrength = windStrength * (1.0 + gust);
    
    // Mouvement avec windSpeed intégré dans les oscillations temporelles
    float xMovement = noise * windDir.x + sin(timeScale * 2.0 + phaseOffset) * 0.3 * spatialVariation;
    float zMovement = noise * windDir.y + cos(timeScale * 1.5 + phaseOffset) * 0.3 * spatialVariation;
    
    // Application du déplacement
    float effectiveHeight = smoothstep(0.0, 1.0, height);
    position.x += xMovement * effectiveHeight * effectiveWindStrength;
    position.z += zMovement * effectiveHeight * 0.3 * effectiveWindStrength;
    
    // Affaissement vertical
    float verticalDroop = 0.4 * (1.0 + gust * 0.5);
    position.y -= (abs(xMovement) + abs(zMovement)) * height * verticalDroop * effectiveWindStrength;
    
    // Tremblement pendant les rafales avec windSpeed
    if (gust > 0.7) {
        float tremblePhase = timeScale * 15.0 + spatialVariation * 30.0;
        position.x += sin(tremblePhase) * gust * 0.02 * height;
        position.z += cos(tremblePhase + 1.3) * gust * 0.02 * height;
    }
    // Envoyer les attributs de vertex au fragment shader
    fragPosition = vec3(matModel * vec4(position, 1.0));
    //fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = modifiedNormal;
    
    
    // Calculer la position finale du vertex
    gl_Position = mvp * vec4(position, 1.0);
}else if (isGrass == 2) {
    // Pour les arbres, on applique un mouvement sinusoïdal plus subtil et sur les extrémitées
    // Hauteur relative du vertex (pour les arbres, les branches supérieures bougeront plus)
    float height = position.y / 10.0;  // Ajustez la division selon la taille de vos arbres
    float gustSpeed = 400.3; // Vitesse des rafales pour les arbres
    // Réduire l'effet du mouvement pour les arbres
    float treeWindStrength = windStrength * 0.5;
    float treeGustStrength = 0.4;     // Rafales plus douces pour les arbres
    
    // Coordonnées pour le bruit avec une échelle plus grande (mouvement plus large)
    vec2 treeNoiseCoord = vec2(
        position.x * 0.01 + time * windSpeed * 0.5, 
        position.z * 0.01 + time * windSpeed * 0.3
    );
    
    // Coordonnées pour les rafales
    vec2 treeGustCoord = vec2(
        position.x * 0.05 + time * gustSpeed * 0.3, 
        position.z * 0.05 + time * gustSpeed * 0.2
    );
    
    // Échantillonnage des bruits avec une influence plus faible
    float treeNoise = (texture(noiseTexture, treeNoiseCoord).r - 0.5) * 0.7;
    float treeGust = texture(noiseTexture, treeGustCoord).r * treeGustStrength;
    
    // Mappage non-linéaire pour des rafales plus naturelles
    treeGust = pow(treeGust, 2.0);
    
    // Atténuation pour le tronc (les parties basses bougent moins)
    float trunkFactor = smoothstep(0.0, 0.7, height);
    
    // Direction du vent
    vec2 windDir = normalize(windHorizontalDirection);
    
    // Force effective du vent
    float treeEffectiveWindStrength = treeWindStrength * (1.0 + treeGust);
    
    // Mouvement principal
    float treeXMovement = treeNoise * windDir.x + sin(time * 0.8 + position.z * 0.05) * 2.15;
    float treeZMovement = treeNoise * windDir.y + cos(time * 0.7 + position.x * 0.05) * 2.15;
    
    // Appliquer le mouvement avec l'atténuation pour le tronc
    position.x += treeXMovement * trunkFactor * treeEffectiveWindStrength;
    position.z += treeZMovement * trunkFactor * treeEffectiveWindStrength * 0.5;
    
    // Léger affaissement pendant les rafales fortes (surtout pour les branches/feuilles)
    position.y -= (abs(treeXMovement) + abs(treeZMovement)) * height * 0.2 * treeEffectiveWindStrength;
    
    // Mouvement supplémentaire pour les feuillages (parties les plus hautes)
    if (height > 0.6) {
        float leafFactor = (height - 0.6) / 0.4;  // 0 au début du feuillage, 1 au sommet
        float leafNoise = sin(time * 3.0 + position.x * 15.0 + position.z * 15.0) * 0.01;
        
        position.x += leafNoise * leafFactor * treeGust;
        position.z += leafNoise * leafFactor * treeGust;
        position.y += cos(time * 4.0 + position.x * 10.0) * 0.01 * leafFactor * treeGust;
    }



    // Envoyer les attributs de vertex au fragment shader
    fragPosition = vec3(matModel * vec4(position, 1.0));
    //fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 1.0)));
    
    // Calculer la position finale du vertex
    gl_Position = mvp * vec4(position, 1.0);
    //gl_Position = mvp * vec4(vertexPosition, 1.0);
}
}