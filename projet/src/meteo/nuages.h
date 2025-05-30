#ifndef NUAGES_H
#define NUAGES_H

#include <raylib.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

struct Nuage {
    std::vector<Model> plans;
    std::vector<Vector3> positions;
    std::vector<float> scales;
    std::vector<Texture2D> textures;
    std::vector<float> rotations;
    float vitesseDefile;
    float largeurTotale;
};

Texture2D GenererTextureNuage(int largeur, int hauteur, int seed, float seuil = 0.2f, float echelle = 10.0f);
Nuage GenererGrandNuage(Vector3 position, float longueur, float hauteur, int nombrePlans, float seuil = 0.2f, float echelle = 10.0f);

#endif // NUAGES_H