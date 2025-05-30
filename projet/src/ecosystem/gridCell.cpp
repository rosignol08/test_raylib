// src/ecosystem/GridCell.cpp
#include "gridCell.h"

// Constructeur de GridCell
GridCell::GridCell(int id, Vector3 pos, Model mod, bool act, bool occ, 
                   int temp, int hum, int plu, float pen, Plante plante)
    : identifiant(id), position(pos), model(mod), active(act), occupee(occ), 
      temperature(temp), humidite(hum), pluviometrie(plu), pente(pen), plante(plante) {
}