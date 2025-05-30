#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
in int nuage;  // Attribut booléen ajouté du second shader

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

// Paramètres de bruit adaptés du shader Godot
uniform sampler2D noiseTexture;        // Texture de bruit
uniform float windTextureTileSize = 20.0;
uniform float windVerticalStrength = 0.3;
uniform vec2 windHorizontalDirection = vec2(1.0, 0.5);

// Paramètres ajustables
float noiseInfluence = 0.7;     // Influence du bruit (0.0 - 1.0)
float sineInfluence = 0.3;      // Influence du mouvement sinusoïdal (0.0 - 1.0)
float verticalDroop = 0.4;      // Affaissement vertical (0.0 - 1.0)
float treeTopFactor = 0.6;      // Facteur d'influence de la hauteur pour l'effet d'arbre

void main() {
    
    //Copie de la position initiale
    vec3 position = vertexPosition;
    
    
        // Calculer un facteur de hauteur normalisé
        float normalizedHeight = (position.y - min(position.y, 0.0)) / max(position.y, 1.0);
        normalizedHeight = pow(normalizedHeight, 2.0); // Accentuer l'effet vers le haut
        
        // Paramètres pour le mouvement de l'arbre
        float treeGustStrength = 0.5;
        float treeWindSpeed = 0.7;
        
        // Coordonnées pour le bruit
        vec2 treeNoiseCoord = vec2(
            time * treeWindSpeed * 0.2,
            time * treeWindSpeed * 0.15
        );
        
        // Mouvement de base de l'arbre
        float treeSway = sin(time * 1.5) * 0.02;
        float treeNoise = texture(noiseTexture, treeNoiseCoord).r - 0.5;
        
        // Calcul des mouvements
        float treeXMovement = (treeSway + treeNoise * 0.2) * normalizedHeight * treeTopFactor;
        float treeZMovement = (cos(time * 1.2) * 0.015 + treeNoise * 0.1) * normalizedHeight * treeTopFactor;
        
        // Appliquer le mouvement
        position.x += treeXMovement * windStrength * 2.0;
        position.z += treeZMovement * windStrength * 2.0;
        
        // Légère réduction de hauteur pour simuler la flexion
        position.y -= (abs(treeXMovement) + abs(treeZMovement)) * 0.5 * normalizedHeight;
    
    
    // Envoyer les attributs de vertex au fragment shader
    fragPosition = vec3(matModel * vec4(position, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 1.0)));
    
    // Calculer la position finale du vertex
    gl_Position = mvp * vec4(position, 1.0);
}