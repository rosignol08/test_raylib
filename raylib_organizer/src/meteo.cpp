#include "meteo.h"
#include <raylib.h>
#include <iostream>
#include <vector>

Meteo ajoute_meteo(vector<Meteo>& liste_meteo, Meteo meteo) {
    liste_meteo.push_back(meteo);
    return meteo;
}

Meteo cherche_la_meteo_actuelle(vector<Meteo>& liste_meteo){
    for (size_t i = 0; i < liste_meteo.size(); i++) {
        if (liste_meteo[i].meteo_actuelle) {
            return liste_meteo[i];
        }
    }
    return Meteo("", 0, 0, WHITE, 0.0f, false); // Retourne une météo vide si aucune n'est trouvée
}

Meteo change_la_meteo_actuelle(vector<Meteo>& liste_meteo, Meteo meteo){
    for (size_t i = 0; i < liste_meteo.size(); i++) {
        if (liste_meteo[i].meteo_actuelle) {
            liste_meteo[i].meteo_actuelle = false; // Désactive la météo actuelle
        }
    }
    for (size_t i = 0; i < liste_meteo.size(); i++) {
        if (liste_meteo[i].nom == meteo.nom) {
            liste_meteo[i].meteo_actuelle = true; // Active la nouvelle météo
            return liste_meteo[i];
        }
    }
    return Meteo("", 0, 0, WHITE, 0.0f, false); // Retourne une météo vide si aucune n'est trouvée
}

int get_meteo_temperature(Meteo meteo){
    return meteo.temperature;
}
int get_meteo_humidite(Meteo meteo){
    return meteo.humidite;
}
Color get_meteo_couleur(Meteo meteo){
    return meteo.couleur;
}
//TODO voir si on affecte cloudThreshold autant que noiseScale
float get_meteo_densite_nuage(Meteo meteo){
    return meteo.densite_nuage;
}