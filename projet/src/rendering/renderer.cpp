// src/rendering/Renderer.cpp
#include "renderer.h"
#include "../ecosystem/EcosystemManager.h"
#include <cmath>
#include <stdio.h>
#include "utils/constants.h"  


#if defined(_WIN32) || defined(_WIN64)
#include <shaders/rlights.h>
#elif defined(__linux__)
#include <shaders/rlights.h>
#endif

Renderer::Renderer() {
    time = 0.0f;
    windTextureTileSize = 2.0f;
    windVerticalStrength = 0.3f;
    windHorizontalDirection = { 1.0f, 0.5f };
    lightDir = Vector3Normalize((Vector3){ 0.35f, -1.0f, -0.35f });
    lightColor = WHITE;
}

Renderer::~Renderer() {
    Cleanup();
}

void Renderer::Initialize() {
    // Charger les shaders
    shadowShader = LoadShader(TextFormat("include/shaders/resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
                             TextFormat("include/shaders/resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    herbe_shader = LoadShader("assets/custom_shader/glsl330/herbe_shader.vs",
                             "assets/custom_shader/glsl330/herbe_shader.fs");
    
    // Configurer les locations des shaders
    shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");
    printf("=== DEBUT RENDERER INITIALIZE ===\n");
    lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
    lightColLoc = GetShaderLocation(shadowShader, "lightColor");
    ambientLoc = GetShaderLocation(shadowShader, "ambient");
    lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
    shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");
    printf("=== DEBUT RENDERER INITIALIZE ===\n");
    // Configuration pour l'herbe
    timeLocation = GetShaderLocation(herbe_shader, "time");
    windStrengthLocation = GetShaderLocation(herbe_shader, "windStrength");
    windSpeedLocation = GetShaderLocation(herbe_shader, "windSpeed");
    isGrassLocation = GetShaderLocation(herbe_shader, "isGrass");
    windTextureTileSizeLocation = GetShaderLocation(herbe_shader, "windTextureTileSize");
    windVerticalStrengthLocation = GetShaderLocation(herbe_shader, "windVerticalStrength");
    windHorizontalDirectionLocation = GetShaderLocation(herbe_shader, "windHorizontalDirection");
    noiseTextureLoc = GetShaderLocation(herbe_shader, "noiseTexture");
    noiseScaleLoc = GetShaderLocation(herbe_shader, "noiseScale");
    
    // Créer la shadowmap
    shadowMap = LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
    
    // Configuration de la caméra de lumière
    lightCam.position = Vector3Scale(lightDir, -15.0f);
    lightCam.target = Vector3Zero();
    lightCam.projection = CAMERA_ORTHOGRAPHIC;
    lightCam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    lightCam.fovy = 20.0f;
    
    // Générer la texture de bruit
    Image noiseImage = GenImagePerlinNoise(1024, 1024, 0, 0, 100.0f);
    noiseTexture = LoadTextureFromImage(noiseImage);
    UnloadImage(noiseImage);
    
    // Charger le modèle d'herbe
    model_herbe_instance = LoadModel("models/herbe/untitled2.glb");
    model_herbe_instance.materials[0].shader = herbe_shader;
    
    // Initialiser les valeurs par défaut
    lightColorNormalized = ColorNormalize(lightColor);
    SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
    
    int shadowMapResolution = SHADOWMAP_RESOLUTION;
    SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);
    SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);
}

void Renderer::Cleanup() {
    UnloadShader(shadowShader);
    UnloadShader(herbe_shader);
    UnloadShadowmapRenderTexture(shadowMap);
    UnloadTexture(noiseTexture);
    UnloadTexture(temperatureTexture);
    UnloadModel(model_herbe_instance);
    if (terrainColorImage.data) {
        UnloadImage(terrainColorImage);
    }
}

void Renderer::InitGrassParticles(Vector3 taille_terrain, Image image_sol) {
    for (int i = 0; i < MAX_GRASS; i++) {
        float posX = ((float)GetRandomValue(0, 10000) / 10000.0f) * taille_terrain.x - taille_terrain.x / 2;
        float posZ = ((float)GetRandomValue(0, 10000) / 10000.0f) * taille_terrain.z - taille_terrain.z / 2;
        
        float offsetX = ((float)GetRandomValue(-1000, 1000) / 10000.0f);
        float offsetZ = ((float)GetRandomValue(-1000, 1000) / 10000.0f);
        posX += offsetX;
        posZ += offsetZ;
        
        float height = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ }, image_sol, taille_terrain);
        
        // Calcul de la pente
        float seuil_verification_pente = 0.2f;
        float heightLeft = GetHeightFromTerrain((Vector3){ posX - seuil_verification_pente, 0.0f, posZ }, image_sol, taille_terrain);
        float heightRight = GetHeightFromTerrain((Vector3){ posX + seuil_verification_pente, 0.0f, posZ }, image_sol, taille_terrain);
        float heightUp = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ - seuil_verification_pente }, image_sol, taille_terrain);
        float heightDown = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ + seuil_verification_pente }, image_sol, taille_terrain);

        float deltaX = fabsf(heightLeft - heightRight);
        float deltaZ = fabsf(heightUp - heightDown);
        float slope = sqrtf((deltaX * deltaX + deltaZ * deltaZ) / 2.0f);

        const float SLOPE_THRESHOLD = 0.20f;
        if (slope > SLOPE_THRESHOLD) {
            i--;
            continue;
        }

        grass[i].size = (Vector2){0.01f + ((float)GetRandomValue(0, 10000) / 10000.0f) * 0.02f, 
                                 0.04f + ((float)GetRandomValue(0, 10000) / 10000.0f) * 0.03f};
        grass[i].position = (Vector3){ posX, height + grass[i].size.y, posZ };
        grass[i].rotationY = ((float)GetRandomValue(0, 10000) / 10000.0f) * PI * 2.0f;
        grass[i].rotationX = ((float)GetRandomValue(-2000, 2000) / 10000.0f);
        
        float teinte = 0.8f + ((float)GetRandomValue(0, 2000) / 10000.0f);
        grass[i].color = (Color){
            (unsigned char)(34 * teinte),
            (unsigned char)(139 * teinte),
            (unsigned char)(34 * teinte),
            255
        };
    }
}

void Renderer::RenderGrass(float timeOfDay, int viewMode) {
    Color grassColor = GetGrassColorFromTime(timeOfDay);
    bool useTerrainColorForGrass = (viewMode != MODE_NORMAL);
    
    for (int i = 0; i < MAX_GRASS; i++) {
        DrawGrassQuad(grass[i], grassColor, useTerrainColorForGrass, terrainColorImage, {4, 2, 4});
    }
}

void Renderer::RenderShadows(Camera camera, Model& model_sol, std::vector<std::vector<GridCell>>& grille, 
                           std::vector<Nuage>& grandsNuages, Vector3 mapPosition) {
    Matrix lightView, lightProj;
    BeginTextureMode(shadowMap);
    ClearBackground(SKYBLUE);

    BeginMode3D(lightCam);
    lightView = rlGetMatrixModelview();
    lightProj = rlGetMatrixProjection();
    
    // Rendu des objets pour les ombres
    DrawModel(model_sol, mapPosition, 0.1f, WHITE);
    
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            if (grille[x][z].active) {
                float scale = grille[x][z].plante.taille;
                DrawModel(grille[x][z].plante.model, grille[x][z].position, scale, WHITE);
            }
        }
    }
    
    // Rendu des nuages pour les ombres
    rlEnableColorBlend();
    for (auto& nuage : grandsNuages) {
        for (size_t i = 0; i < nuage.plans.size(); i++) {
            DrawModelEx(nuage.plans[i], nuage.positions[i], (Vector3){0, 1, 0}, 
                       nuage.rotations[i], (Vector3){nuage.scales[i], nuage.scales[i], nuage.scales[i]}, WHITE);
        }
    }
    
    EndMode3D();
    EndTextureMode();
    
    Matrix lightViewProj = MatrixMultiply(lightView, lightProj);
    SetShaderValueMatrix(shadowShader, lightVPLoc, lightViewProj);
    
    rlEnableShader(shadowShader.id);
    int slot = 10;
    rlActiveTextureSlot(10);
    rlEnableTexture(shadowMap.depth.id);
    rlSetUniform(shadowMapLoc, &slot, SHADER_UNIFORM_INT, 1);
}

void Renderer::RenderScene(Camera camera, Image image_sol, Vector3 taille_terrain, Model& model_sol, 
                         std::vector<std::vector<GridCell>>& grille, int viewMode, 
                         int minTemp, int maxTemp, int minHum, int maxHum, Vector3 mapPosition) {
    
    // Rendu du terrain
    DrawModel(model_sol, mapPosition, 0.1f, WHITE);
    
    // Rendu des plantes
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            if (grille[x][z].active) {
                Color plantColor = WHITE;
                
                if (viewMode == MODE_TEMPERATURE) {
                    plantColor = GetTemperatureColor(grille[x][z].temperature, minTemp, maxTemp);
                } else if (viewMode == MODE_HUMIDITE) {
                    plantColor = GetHumidityColor(grille[x][z].humidite, minHum, maxHum);
                } else if (viewMode == MODE_PLUVIOMETRIE) {
                    plantColor = GetPluviometrieColor(grille[x][z].pluviometrie, minTemp, maxTemp);
                } else {
                    plantColor = grille[x][z].plante.couleur;
                }
                
                float scale = grille[x][z].plante.taille;
                DrawModel(grille[x][z].plante.model, grille[x][z].position, scale, plantColor);
            }
        }
    }
}

void Renderer::RenderClouds(std::vector<Nuage>& grandsNuages) {
    rlDisableBackfaceCulling();
    rlEnableColorBlend();
    
    for (auto& nuage : grandsNuages) {
        for (size_t i = 0; i < nuage.plans.size(); i++) {
            DrawModelEx(nuage.plans[i], nuage.positions[i], (Vector3){0, 1, 0}, 
                       nuage.rotations[i], (Vector3){nuage.scales[i], nuage.scales[i], nuage.scales[i]}, WHITE);
        }
    }
}

void Renderer::UpdateLighting(float timeOfDay, std::vector<Biome>& les_biomes) {
    // Calcul de la direction du soleil
    float sunAngle = ((timeOfDay - 6.0f) / 12.0f) * PI;
    lightDir = Vector3Normalize((Vector3){
        cosf(sunAngle),
        -sinf(sunAngle),
        0.0f
    });
    
    // Mise à jour de la position de la caméra de lumière
    lightCam.position = Vector3Scale(lightDir, -15.0f);
    lightCam.target = Vector3Zero();
    
    // Couleur de la lumière en fonction de l'heure et du biome
    Color sunColor = GetSunColor(timeOfDay);
    Biome biome_actuel = cherche_le_biome_actuelle(les_biomes);
    Color couleur_biome = biome_actuel.couleur;
    
    lightColor = (Color){
        sunColor.r / 2 + couleur_biome.r / 2,
        sunColor.g / 2 + couleur_biome.g / 2,
        sunColor.b / 2 + couleur_biome.b / 2,
        sunColor.a / 2 + couleur_biome.a / 2
    };
    
    lightColorNormalized = ColorNormalize(lightColor);
    
    // Mise à jour des shaders
    SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
    SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightPos"), &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightColor"), &lightColorNormalized, SHADER_UNIFORM_VEC4);
    SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightDir"), &lightDir, SHADER_UNIFORM_VEC3);
}

void Renderer::UpdateWind(float windStrength, float windSpeed, float deltaTime) {
    time += deltaTime;
    
    SetShaderValue(herbe_shader, timeLocation, &time, SHADER_UNIFORM_FLOAT);
    SetShaderValue(herbe_shader, windStrengthLocation, &windStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(herbe_shader, windSpeedLocation, &windSpeed, SHADER_UNIFORM_FLOAT);
    SetShaderValue(herbe_shader, windTextureTileSizeLocation, &windTextureTileSize, SHADER_UNIFORM_FLOAT);
    SetShaderValue(herbe_shader, windVerticalStrengthLocation, &windVerticalStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(herbe_shader, windHorizontalDirectionLocation, &windHorizontalDirection, SHADER_UNIFORM_VEC2);
    SetShaderValueTexture(herbe_shader, noiseTextureLoc, noiseTexture);
    
    float noiseScale = 10.0f;
    SetShaderValue(herbe_shader, noiseScaleLoc, &noiseScale, SHADER_UNIFORM_FLOAT);
}

void Renderer::DrawGrassQuad(GrassQuad g, Color baseColor, bool useTerrainColor, Image terrainImage, Vector3 taille_terrain) {
    float w = g.size.x * 0.5f;
    float h = g.size.y;

    Vector3 topLeft = (Vector3){ -w, 0.0f, 0.0f };
    Vector3 topRight = (Vector3){ w, 0.0f, 0.0f };
    Vector3 bottomRight = (Vector3){ w, -h, 0.0f };
    Vector3 bottomLeft = (Vector3){ -w, -h, 0.0f };

    Matrix rot = MatrixRotateY(g.rotationY);
    topLeft = Vector3Transform(topLeft, rot);
    topRight = Vector3Transform(topRight, rot);
    bottomRight = Vector3Transform(bottomRight, rot);
    bottomLeft = Vector3Transform(bottomLeft, rot);

    rot = MatrixRotateX(g.rotationX);
    topLeft = Vector3Transform(topLeft, rot);
    topRight = Vector3Transform(topRight, rot);
    bottomRight = Vector3Transform(bottomRight, rot);
    bottomLeft = Vector3Transform(bottomLeft, rot);

    topLeft = Vector3Add(topLeft, g.position);
    topRight = Vector3Add(topRight, g.position);
    bottomRight = Vector3Add(bottomRight, g.position);
    bottomLeft = Vector3Add(bottomLeft, g.position);
    
    Color finalColor = g.color;

    if (useTerrainColor && terrainImage.data) {
        int ix = (int)((g.position.x + taille_terrain.x/2) * terrainImage.width / taille_terrain.x);
        int iz = (int)((g.position.z + taille_terrain.z/2) * terrainImage.height / taille_terrain.z);
        ix = Clamp(ix, 0, terrainImage.width - 1);
        iz = Clamp(iz, 0, terrainImage.height - 1);
        finalColor = GetImageColor(terrainImage, ix, iz);
    }

    rlBegin(RL_QUADS);
    Color blendColor = useTerrainColor ? finalColor : baseColor;
    unsigned char r = (unsigned char)Clamp((g.color.r * 0.5f + blendColor.r * 0.5f), 0, 255);
    unsigned char v = (unsigned char)Clamp((g.color.g * 0.5f + blendColor.g * 0.5f), 0, 255);
    unsigned char b = (unsigned char)Clamp((g.color.b * 0.5f + blendColor.b * 0.5f), 0, 255);
    
    rlColor4ub(r, v, b, 255);
    rlVertex3f(topLeft.x, topLeft.y, topLeft.z);
    rlVertex3f(topRight.x, topRight.y, topRight.z);
    rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
    rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
    rlEnd();
}

Color Renderer::GetGrassColorFromTime(float timeOfDay) {
    if (timeOfDay < 6.0f || timeOfDay >= 20.0f) {
        return (Color){ 10, 10, 30, 255 };
    } else if (timeOfDay < 7.0f) {
        float t = timeOfDay - 6.0f;
        return (Color){
            (unsigned char)(255 * t),
            (unsigned char)(165 * t),
            (unsigned char)(10 * t),
            255
        };
    } else if (timeOfDay < 8.0f) {
        float t = timeOfDay - 7.0f;
        return (Color){
            255,
            (unsigned char)(200 + t * 55),
            (unsigned char)(100 - t * 50),
            255
        };
    } else if (timeOfDay < 17.0f) {
        return (Color){ 255, 255, 255, 255 };
    } else if (timeOfDay < 18.0f) {
        float t = timeOfDay - 17.0f;
        return (Color){
            (unsigned char)(255 - t * 5),
            (unsigned char)(255 - t * 55),
            (unsigned char)(100 + t * 50),
            255
        };
    } else if (timeOfDay < 19.0f) {
        float t = timeOfDay - 18.0f;
        return (Color){
            (unsigned char)(255 - t * 155),
            (unsigned char)(150 - t * 100),
            (unsigned char)(100 + t * 50),
            255
        };
    } else {
        float t = timeOfDay - 19.0f;
        return (Color){
            (unsigned char)(100 - t * 90),
            (unsigned char)(50 - t * 40),
            (unsigned char)(150 + t * 10),
            255
        };
    }
}

Color Renderer::GetSunColor(float timeOfDay) {
    if (timeOfDay < 6.0f || timeOfDay >= 20.0f) {
        return (Color){ 10, 10, 30, 255 };
    } else if (timeOfDay < 7.0f) {
        float t = (timeOfDay - 6.0f);
        return (Color){
            (unsigned char)(255 * t),
            (unsigned char)(165 * t),
            (unsigned char)(10 * t),
            255
        };
    } else if (timeOfDay < 8.0f) {
        float t = (timeOfDay - 7.0f);
        return (Color){
            255,
            (unsigned char)(200 + t * 55),
            (unsigned char)(100 - t * 50),
            255
        };
    } else if (timeOfDay < 17.0f) {
        return (Color){ 255, 255, 255, 255 };
    } else if (timeOfDay < 18.0f) {
        float t = (timeOfDay - 17.0f);
        return (Color){
            (unsigned char)(255 - t * 5),
            (unsigned char)(255 - t * 55),
            (unsigned char)(100 + t * 50),
            255
        };
    } else if (timeOfDay < 19.0f) {
        float t = (timeOfDay - 18.0f);
        return (Color){
            (unsigned char)(255 - t * 155),
            (unsigned char)(150 - t * 100),
            (unsigned char)(100 + t * 50),
            255
        };
    } else {
        float t = (timeOfDay - 19.0f);
        return (Color){
            (unsigned char)(100 - t * 90),
            (unsigned char)(50 - t * 40),
            (unsigned char)(150 + t * 10),
            255
        };
    }
}

// Fonctions utilitaires de rendu
void InitializeTerrain(Image& image_sol, Model& model_sol, Vector3& taille_terrain) {
    Mesh mesh_sol = GenMeshHeightmap(image_sol, (Vector3){ 40, 20, 40 });
    model_sol = LoadModelFromMesh(mesh_sol);
    
    Image image_texture_sol = LoadImage("assets/compress_terrain_texture_tiede.jpg");
    Texture2D texture_sol = LoadTextureFromImage(image_texture_sol);
    UnloadImage(image_texture_sol);
    
    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol;
}

void UnloadTerrainResources(Image& image_sol) {
    UnloadImage(image_sol);
}

void HandleViewModeInput(int& viewMode, std::vector<std::vector<GridCell>>& grille, 
                        Model& model_sol, int& minTemp, int& maxTemp, int& minHum, int& maxHum, 
                        int& minPluv, int& maxPluv) {
    static Texture2D temperatureTexture = { 0 };
    static Image terrainColorImage = { 0 };
    
    if (IsKeyPressed(KEY_T)) {
        viewMode = (viewMode == MODE_NORMAL) ? MODE_TEMPERATURE : MODE_NORMAL;
        
        if (viewMode == MODE_TEMPERATURE) {
            Image tempImage = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int z = 0; z < GRID_SIZE; z++) {
                    Color tempColor = GetTemperatureColor(grille[x][z].temperature, minTemp, maxTemp);
                    ImageDrawPixel(&tempImage, x, z, tempColor);
                }
            }
            if (temperatureTexture.id != 0) UnloadTexture(temperatureTexture);
            temperatureTexture = LoadTextureFromImage(tempImage);
            if (terrainColorImage.data) UnloadImage(terrainColorImage);
            terrainColorImage = ImageCopy(tempImage);
            UnloadImage(tempImage);
            model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = temperatureTexture;
        }
    }
    
    if (IsKeyPressed(KEY_Y)) {
        viewMode = (viewMode == MODE_NORMAL) ? MODE_HUMIDITE : MODE_NORMAL;
        
        if (viewMode == MODE_HUMIDITE) {
            Image humImage = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int z = 0; z < GRID_SIZE; z++) {
                    Color humColor = GetHumidityColor(grille[x][z].humidite, minHum, maxHum);
                    ImageDrawPixel(&humImage, x, z, humColor);
                }
            }
            if (temperatureTexture.id != 0) UnloadTexture(temperatureTexture);
            temperatureTexture = LoadTextureFromImage(humImage);
            if (terrainColorImage.data) UnloadImage(terrainColorImage);
            terrainColorImage = ImageCopy(humImage);
            UnloadImage(humImage);
            model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = temperatureTexture;
        }
    }
    
    if (IsKeyPressed(KEY_P)) {
        viewMode = (viewMode == MODE_NORMAL) ? MODE_PLUVIOMETRIE : MODE_NORMAL;
        
        if (viewMode == MODE_PLUVIOMETRIE) {
            Image pluvImage = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int z = 0; z < GRID_SIZE; z++) {
                    Color pluvColor = GetPluviometrieColor(grille[x][z].pluviometrie, minPluv, maxPluv);
                    ImageDrawPixel(&pluvImage, x, z, pluvColor);
                }
            }
            if (temperatureTexture.id != 0) UnloadTexture(temperatureTexture);
            temperatureTexture = LoadTextureFromImage(pluvImage);
            if (terrainColorImage.data) UnloadImage(terrainColorImage);
            terrainColorImage = ImageCopy(pluvImage);
            UnloadImage(pluvImage);
            model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = temperatureTexture;
        }
    }
}

Color GetTemperatureColor(int temperature, int minTemp, int maxTemp) {
    float normalizedTemp = (float)(temperature - minTemp) / (maxTemp - minTemp);
    
    Color coldColor = BLUE;
    Color hotColor = RED;
    
    unsigned char r = (unsigned char)(coldColor.r + (hotColor.r - coldColor.r) * normalizedTemp);
    unsigned char g = (unsigned char)(coldColor.g + (hotColor.g - coldColor.g) * normalizedTemp);
    unsigned char b = (unsigned char)(coldColor.b + (hotColor.b - coldColor.b) * normalizedTemp);
    
    return (Color){r, g, b, 255};
}

Color GetHumidityColor(int humidity, int minHum, int maxHum) {
    float normalizedHum = (float)(humidity - minHum) / (maxHum - minHum);
    
    Color dryColor = BLUE;
    Color wetColor = GREEN;
    
    unsigned char r = (unsigned char)(dryColor.r + (wetColor.r - dryColor.r) * normalizedHum);
    unsigned char g = (unsigned char)(dryColor.g + (wetColor.g - dryColor.g) * normalizedHum);
    unsigned char b = (unsigned char)(dryColor.b + (wetColor.b - dryColor.b) * normalizedHum);
    
    return (Color){r, g, b, 255};
}

Color GetPluviometrieColor(int rainfall, int minPluv, int maxPluv) {
    float normalizedPluv = (float)(rainfall - minPluv) / (maxPluv - minPluv);
    normalizedPluv = Clamp(normalizedPluv, 0.0f, 1.0f);
    
    Color dryColor = WHITE;
    Color wetColor = BLUE;
    
    unsigned char r = (unsigned char)(dryColor.r + (wetColor.r - dryColor.r) * normalizedPluv);
    unsigned char g = (unsigned char)(dryColor.g + (wetColor.g - dryColor.g) * normalizedPluv);
    unsigned char b = (unsigned char)(dryColor.b + (wetColor.b - dryColor.b) * normalizedPluv);
    
    return (Color){r, g, b, 255};
}