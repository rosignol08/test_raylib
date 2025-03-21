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

void main() {
    // Copie de la position initiale
    vec3 position = vertexPosition;
    
    // Appliquer l'effet de vent seulement à l'herbe
    if (isGrass == 1) {
        // Calculer l'effet de vent basé sur le temps et la position
        // Plus la position Y (hauteur) est élevée, plus l'effet est fort
        float windEffect = sin(time * windSpeed + position.x * 0.5) * cos(time * windSpeed * 0.7 + position.z * 0.5);
        
        // Appliquer l'effet principalement sur X et un peu sur Z, proportionnel à la hauteur Y
        float heightFactor = position.y / 2.0; // Ajuster selon la hauteur de votre herbe
        position.x += windEffect * windStrength * heightFactor;
        position.z += windEffect * windStrength * 0.3 * heightFactor; // Effet moindre sur Z
        
    }
    
    // Envoyer les attributs de vertex au fragment shader
    fragPosition = vec3(matModel * vec4(position, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 1.0)));
    
    // Calculer la position finale du vertex
    gl_Position = mvp * vec4(position, 1.0);
}