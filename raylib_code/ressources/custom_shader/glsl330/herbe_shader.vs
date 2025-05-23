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
uniform int isGrass = 0;               // Indicateur si c'est de l'herbe (1) ou non (0)

// Paramètres de bruit adaptés du shader Godot
uniform sampler2D noiseTexture;        // Texture de bruit
uniform float windTextureTileSize = 20.0;
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
    float height = vertexTexCoord.y;
    
    // Paramètres pour les rafales
    float gustStrength = 0.8;     // Force des rafales (0.0 - 1.0+)
    float gustFrequency = 0.2;    // Fréquence des rafales (plus bas = moins fréquent)
    float gustSpeed = 0.5;        // Vitesse des rafales
    
    // Coordonnées pour le bruit primaire (mouvement de base)
    vec2 noiseCoord = vec2(
        (position.x + position.z) * 0.05 + time * windSpeed, 
        (position.z - position.x) * 0.05 + time * windSpeed * 0.7
    );
    
    // Coordonnées pour le bruit des rafales (échelle plus grande, mouvement plus lent)
    vec2 gustCoord = vec2(
        (position.x + position.z) * 0.02 + time * gustSpeed, 
        (position.z - position.x) * 0.02 + time * gustSpeed * 0.6
    );
    
    // Échantillonnage des bruits
    float noise = texture(noiseTexture, noiseCoord).r - 0.5;
    float gust = texture(noiseTexture, gustCoord).r;
    
    // Traitement du bruit de rafale pour créer des pics occasionnels
    // Mapping non-linéaire pour créer des pics plus prononcés
    gust = pow(gust, 3.0) * gustStrength;
    
    // Variation unique pour chaque brin d'herbe
    float variation = sin(position.x * 12.3 + position.z * 15.7) * 0.5 + 0.5;
    
    // Direction du vent
    vec2 windDir = normalize(windHorizontalDirection);
    
    // Force de vent modulée par les rafales
    float effectiveWindStrength = windStrength * (1.0 + gust);
    
    // Mouvement combinant bruit principal et rafales
    float xMovement = noise * windDir.x + sin(time * 2.0 + position.z * 0.1 + variation) * 0.3;
    
    float zMovement = noise * windDir.y + cos(time * 1.5 + position.x * 0.1 + variation) * 0.3;

    // Application du déplacement avec effet de la rafale
    float baseMovement = 0.15; // Mouvement minimal à la base
    float effectiveHeight =position.y / 2.0;// baseMovement + height * (1.0 - baseMovement);
    //effectiveHeight = pow(effectiveHeight, 1.5); 
    position.x += xMovement * effectiveHeight * effectiveWindStrength;
    position.z += zMovement * effectiveHeight * 0.3 * effectiveWindStrength;
    
    // Affaissement vertical accentué pendant les rafales
    float verticalDroop = 0.4 * (1.0 + gust * 0.5); // Plus d'affaissement pendant les rafales
    position.y -= (abs(xMovement) + abs(zMovement)) * height * verticalDroop * effectiveWindStrength;
    
    // Ajoutez un léger tremblement pendant les rafales fortes
    if (gust > 0.5) {
        position.x += sin(time * 15.0 + position.y * 10.0) * gust * 0.02 * height;
        position.z += cos(time * 17.0 + position.y * 10.0) * gust * 0.02 * height;
    }
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