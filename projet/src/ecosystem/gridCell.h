// src/ecosystem/GridCell.h
#ifndef GRIDCELL_H
#define GRIDCELL_H

#include <raylib.h>
#include "plante.h"

class GridCell {
public:
    int identifiant;
    Vector3 position;
    Model model;
    bool active;
    bool occupee;
    int temperature;
    int humidite;
    int pluviometrie;
    float pente;
    Plante plante;

    // Constructeur
    GridCell(int id, Vector3 pos, Model mod, bool act, bool occupee, 
             int temp, int hum, int pluviometrie, float pen, Plante plante);
};

#endif