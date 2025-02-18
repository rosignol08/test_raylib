#include "sol.h"
Model model_mort;
//Plante plante_morte("Morte", 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0, true, Model{});
// Constructeur
Plante::Plante(string n, int h_min, int h_max, int t_min, int t_max, int inf_h, int inf_t, float t, float ta_max, float pen, int a,bool morte, Model mod)
    : nom(n), humidite_min(h_min), humidite_max(h_max), temperature_min(t_min), temperature_max(t_max), influence_humidite(inf_h), influence_temperature(inf_t),
      taille(t), taille_max(ta_max), pente_max(pen), age(a), morte(morte), model(mod) {
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

void Plante::vieux(){
    if (this->age > 10){
    }
    this->age += 1;
    this->taille *= 1.03;
}
// Méthode pour vérifier si la plante meurt
void Plante::verifierConditionsEtMourir(std::vector<std::vector<GridCell>>& grille, int x, int y) {
    GridCell& cell = grille[x][y];
    // Les conditions d'humidité et de température sont vérifiées directement
    if (not(cell.humidite < this->humidite_min || cell.humidite > this->humidite_max ||
        cell.temperature < this->temperature_min || cell.temperature > this->temperature_max)) {
            // Affichage pour débogage : tu peux vérifier si les conditions de mort sont satisfaites
        //std::cout << "Plante " << this->nom << " meurt (Temp: " << cell.temperature 
        //<< " | Humidite: " << cell.humidite << ")" << std::endl;
        // Si la plante ne survit pas, on la réinitialise
        //this->meurt();
        //on fait vieillir la plante
        this->vieux();
    }
}

// Méthode pour tuer la plante
void Plante::meurt() {
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
}

// Constructeur de GridCell
GridCell::GridCell(Vector3 pos, Model mod, bool act, bool occ, int temp, int hum, float pen)
    : position(pos), model(mod), active(act),occupee(occ), temperature(temp), humidite(hum), pente(pen), plante("", 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0,false, Model{}) {
    // On initialise plante avec le constructeur par défaut
}
// Implémentation de la mise à jour de GridCell
void GridCell::update(std::vector<std::vector<GridCell>>& grille, int x, int y) {
    // Appeler la méthode d'influence sur les voisins pour la plante présente
    plante.influencerVoisins(grille, x, y);

    // Vérifier les conditions pour voir si la plante vieillie ou meurt
    if (temperature >= plante.temperature_min && humidite >= plante.humidite_min) {
        plante.vieux(); // Plante vieillit
    } else {
        // Si les conditions ne sont pas bonnes, la plante meurt
        // Ici, au lieu de pointer à null, on réinitialise simplement l'objet
        plante.meurt();
        occupee = false; // La cellule n'est plus occupée
    }
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