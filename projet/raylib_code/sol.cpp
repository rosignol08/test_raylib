#include "sol.h"
Model model_mort;
//Plante plante_morte("Morte", 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0, true, Model{});
// Constructeur
Plante::Plante(string n, int sante, int h_min, int h_max, int t_min, int t_max, int inf_h, int inf_temp, float t, float ta_max, float pente_min, float pente_max, int a,bool morte, int age_max, Model mod, Color col)
    : nom(n), sante(sante), humidite_min(h_min), humidite_max(h_max), temperature_min(t_min), temperature_max(t_max), influence_humidite(inf_h), influence_temperature(inf_temp),
      taille(t), taille_max(ta_max),pente_min(pente_min), pente_max(pente_max), age(a), morte(morte), age_max(age_max), model(mod), couleur(col) {
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

// Constructeur de GridCell
GridCell::GridCell(int id, Vector3 pos, Model mod, bool act, bool occ, int temp, int hum, float pen, Plante plante)
    : identifiant(id), position(pos), model(mod), active(act),occupee(occ), temperature(temp), humidite(hum), pente(pen), plante(plante) {
    // On initialise plante avec le constructeur par défaut
}

//constructeur billboard
//billboard(Vector3 pos, Texture2D mod, bool act, bool occupee, int temp, int hum, float pen, Rectangle source, Vector3 billUp, Vector2 size);
billboard::billboard(Vector3 pos, Texture2D tex, Shader shad, bool act, bool occupee, int temp, int hum, float pen, Rectangle source, Vector3 billUp, Vector2 size)
    : position(pos), texture(tex), shader(shad), active(act), occupee(occupee), temperature(temp), humidite(hum), pente(pen), source(source), billUp(billUp), size(size){
    // On initialise plante avec le constructeur par défaut
}

// Constructeur de SolHerbe
SolHerbe::SolHerbe(Vector3 pos, billboard mod, bool act, bool occ, int temp, int hum, float pen)
    : position(pos), model(mod), active(act), occupee(occ), temperature(temp), humidite(hum), pente(pen) {
    // On initialise plante avec le constructeur par défaut
}
