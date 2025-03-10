#include "sol.h"
Model model_mort;
//Plante plante_morte("Morte", 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0, true, Model{});
// Constructeur
Plante::Plante(string n, int sante, int h_min, int h_max, int t_min, int t_max, int inf_h, int inf_temp, float t, float ta_max, float pente_min, float pente_max, int a,bool morte, int age_max, Model mod)
    : nom(n), sante(sante), humidite_min(h_min), humidite_max(h_max), temperature_min(t_min), temperature_max(t_max), influence_humidite(inf_h), influence_temperature(inf_temp),
      taille(t), taille_max(ta_max),pente_min(pente_min), pente_max(pente_max), age(a), morte(morte), age_max(age_max), model(mod) {
    // Initialisation des attributs via la liste d'initialisation
}


// Implémentation de la méthode qui influence les voisins
void Plante::influencerVoisins(std::vector<std::vector<GridCell>>& grille, int x, int y) {
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < static_cast<int>(grille.size()) && ny >= 0 && ny < static_cast<int>(grille[0].size())) {
                grille[nx][ny].humidite += influence_humidite;
                grille[nx][ny].temperature += influence_temperature;
            }
        }
    }
}


// Méthode pour tuer la plante
//void Plante::meurt() {
    // Copier les attributs de la plante morte
    //this->nom = plante_morte.nom;
    //this->humidite_min = plante_morte.humidite_min;
    //this->humidite_max = plante_morte.humidite_max;
    //this->temperature_min = plante_morte.temperature_min;
    //this->temperature_max = plante_morte.temperature_max;
    //this->influence_humidite = plante_morte.influence_humidite;
    //this->influence_temperature = plante_morte.influence_temperature;
    //this->taille = plante_morte.taille;
    //this->taille_max = plante_morte.taille_max;
    //this->pente_max = plante_morte.pente_max;
    //this->age = plante_morte.age;
    //this->morte = true;
    //this->model = model_mort;
//}

// Constructeur de GridCell
GridCell::GridCell(Vector3 pos, Model mod, bool act, bool occ, int temp, int hum, float pen, Plante plante)
    : position(pos), model(mod), active(act),occupee(occ), temperature(temp), humidite(hum), pente(pen), plante(plante) {
    // On initialise plante avec le constructeur par défaut
}

//constructeur billboard
//billboard(Vector3 pos, Texture2D mod, bool act, bool occupee, int temp, int hum, float pen, Rectangle source, Vector3 billUp, Vector2 size);
billboard::billboard(Vector3 pos, Texture2D tex, bool act, bool occupee, int temp, int hum, float pen, Rectangle source, Vector3 billUp, Vector2 size)
    : position(pos), texture(tex), active(act), occupee(occupee), temperature(temp), humidite(hum), pente(pen), source(source), billUp(billUp), size(size){
    // On initialise plante avec le constructeur par défaut
}

// Constructeur de SolHerbe
SolHerbe::SolHerbe(Vector3 pos, billboard mod, bool act, bool occ, int temp, int hum, float pen)
    : position(pos), model(mod), active(act), occupee(occ), temperature(temp), humidite(hum), pente(pen) {
    // On initialise plante avec le constructeur par défaut
}
/*
// monobjet.cpp
#include <iostream>
#include "raylib.h"
#include <vector>


class MonObjet {
    public:
        int valeur;
    MonObjet(int x) : valeur(x) {}
    void afficher() const {
        std::cout << "Valeur: " << valeur << std::endl;
    }
};


// Déclarations des fonctions C pour accéder à l'objet C++
extern "C" {
    MonObjet* MonObjet_nouveau(int x) {
        return new MonObjet(x);
    }

    void MonObjet_afficher(MonObjet* obj) {
        obj->afficher();
    }

    void MonObjet_supprimer(MonObjet* obj) {
        delete obj;
    }

    Plante* Plante_nouvelle(const char* nom, int h_min, int t_min, int inf_h, int inf_t) {
        return new Plante(nom, h_min, t_min, inf_h, inf_t);
    }

    void Plante_influencerVoisins(Plante* plante, std::vector<std::vector<GridCell>>& grille, int x, int y) {
        plante->influencerVoisins(grille, x, y);
    }

    void Plante_supprimer(Plante* plante) {
        delete plante;
    }

    GridCell* GridCell_nouvelle(Vector3 pos, Model mod, bool act, int temp, int hum) {
        return new GridCell(pos, mod, act, temp, hum);
    }

    void GridCell_update(GridCell* cellule, std::vector<std::vector<GridCell>>& grille, int x, int y) {
        cellule->update(grille, x, y);
    }

    void GridCell_assignerPlante(GridCell* cellule, Plante* plante) {
        cellule->plante = plante;
    }

    void GridCell_supprimer(GridCell* cellule) {
        delete cellule;
    }
}
*/