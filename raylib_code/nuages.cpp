#include "nuages.h"
#include <raylib.h>
#include <iostream>
#include <vector>

// Function pour clamp une valeur entre un minimum et un maximum
float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

using namespace std;
// Fonction pour générer une texture de nuage personnalisée pour chaque sphère
Texture2D GenererTextureNuage(int largeur, int hauteur, int seed, float seuil, float echelle) {
    // Générer une image de bruit de Perlin avec un seed unique et une échelle ajustable
    // Une valeur d'échelle plus élevée donne des variations plus petites
    Image nuageImage = GenImagePerlinNoise(largeur, hauteur, seed, seed + 20, echelle);
    
    // Modifier l'image pour créer des nuages toujours un peu translucides
    for(int y = 0; y < nuageImage.height; y++) {
        for(int x = 0; x < nuageImage.width; x++) {
            Color pixel = GetImageColor(nuageImage, x, y);
            
            // Normaliser la valeur du pixel
            float normalizedValue = (float)pixel.r / 255.0f;
            
            // Application du seuil avec un maximum d'opacité limité
            float alpha;
            if (normalizedValue < seuil) {
                alpha = 0.0f; // Complètement transparent sous le seuil
            } else {
                // Calculer l'opacité mais avec un maximum de ~70% pour garder une translucidité permanente
                // Plage de 0 à 180 (au lieu de 0 à 255)
                alpha = 180.0f * ((normalizedValue - seuil) / (1.0f - seuil));
                
                // Ajouter une légère variation aléatoire pour un aspect plus naturel
                alpha *= (0.8f + ((float)GetRandomValue(0, 40) / 100.0f));
            }
            
            // Limiter l'alpha à une valeur maximum (180 est environ 70% opaque)
            alpha = clamp(alpha, 0.0f, 180.0f);
            
            // Définir la couleur du nuage avec la transparence
            Color nuageColor = {255, 255, 255, (unsigned char)alpha};
            ImageDrawPixel(&nuageImage, x, y, nuageColor);
        }
    }
    
    // Appliquer un filtre flou pour adoucir les bords
    ImageBlurGaussian(&nuageImage, 2);
    
    // Convertir en texture
    Texture2D nuageTexture = LoadTextureFromImage(nuageImage);
    
    // Libérer la mémoire de l'image
    UnloadImage(nuageImage);
    
    return nuageTexture;
}
// Fonction pour générer un grand nuage composé d'un seul plan sans rotation
Nuage GenererGrandNuage(Vector3 position, float longueur, float hauteur, int nombrePlans, float seuil, float echelle) {
    Nuage nuage;
    nuage.vitesseDefile = 0.2f;
    nuage.largeurTotale = longueur * 2.0f;
    
    // Dimensions du plan
    float largeurPlan = longueur;
    float hauteurPlan = hauteur;
    
    // Générer un plan (quad)
    Mesh planeMesh = GenMeshPlane(largeurPlan, hauteurPlan, 1, 1);
    Model model = LoadModelFromMesh(planeMesh);
    
    // Créer une texture de nuage unique pour ce plan avec les paramètres spécifiés
    int seed = GetRandomValue(0, 1000);
    Texture2D nuageTexture = GenererTextureNuage( 512, 512, seed, seuil, echelle);
    
    // Appliquer la texture au plan
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = nuageTexture;
    
    // Position du plan
    Vector3 pos = {
        position.x,
        position.y,
        position.z
    };
    
    nuage.plans.push_back(model);
    nuage.positions.push_back(pos);
    nuage.scales.push_back(1.0f);
    nuage.rotations.push_back(0.0f);
    nuage.textures.push_back(nuageTexture);
    
    return nuage;
}