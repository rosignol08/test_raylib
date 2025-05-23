#include <raylib.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

struct Nuage {
    std::vector<Model> plans; // Ensemble de plans formant un grand nuage
    std::vector<Vector3> positions; // Positions des différentes parties du nuage
    std::vector<float> scales; // Tailles des différentes parties
    std::vector<Texture2D> textures;  //pour stocker les textures
    std::vector<float> rotations; // Nouvelle propriété pour stocker les rotations
    float vitesseDefile; // Vitesse de défilement du nuage
    float largeurTotale; // Largeur totale du nuage (pour savoir quand le faire revenir)
};

Texture2D GenererTextureNuage(int largeur, int hauteur, int seed, float seuil = 0.2f, float echelle = 10.0f);
Nuage GenererGrandNuage(Vector3 position, float longueur, float hauteur, int nombrePlans, float seuil = 0.2f, float echelle = 10.0f);