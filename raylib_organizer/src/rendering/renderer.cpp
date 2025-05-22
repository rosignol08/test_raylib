#include "renderer.h"
#include "../utils/math_utils.h"
#include <cstdio>

Renderer::Renderer(int width, int height, const char* title)
    : screenWidth(width), screenHeight(height),
      angleX(0.0f), angleY(0.0f), distance_cam(5.0f),
      isRotating(false), timeOfDay(12.0f), time(0.0f),
      windStrength(0.7f), windSpeed(1.0f), herbeCount(0),
      cloudThreshold(0.6f), noiseScale(10.0f),
      windTextureTileSize(2.0f), windVerticalStrength(0.3f) {
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(width, height, title);
    rlDisableBackfaceCulling();
    rlEnableColorBlend();
    rlSetBlendMode(RL_BLEND_ALPHA);
    SetTargetFPS(60);
    
    // Initialiser caméra (depuis main.cpp)
    camera = { 
        .position = (Vector3){ -5.0f, 0.0f, -5.0f },
        .target = (Vector3){ 0.0f, 0.0f, 0.0f },
        .up = (Vector3){ 0.0f, 1.0f, 0.0f },
        .fovy = 85.0f,
        .projection = CAMERA_PERSPECTIVE
    };
    
    windHorizontalDirection = { 1.0f, 0.5f };
    
    initializeShaders();
    initializeModels();
    initializeClouds();
}

Renderer::~Renderer() {
    // Nettoyage toutes les ressources
    UnloadShader(shadowShader);
    UnloadShader(herbe_shader);
    UnloadShader(pbr_ombre_shader);
    UnloadShader(shader_taille);
    
    UnloadModel(model_sol);
    UnloadModel(model_sapin);
    UnloadModel(model_buisson_europe);
    UnloadModel(model_acacia);
    UnloadModel(model_mort);
    UnloadModel(emptyModel);
    UnloadModel(model_herbe_instance);
    
    UnloadTexture(texture_sol);
    UnloadTexture(temperatureTexture);
    UnloadTexture(noiseTexture);
    UnloadTexture(texture_buisson_europe);
    UnloadTexture(texture_acacia);
    
    UnloadImage(image_texture_sol);
    
    UnloadRenderTexture(shadowMap);
    
    models_herbe_vecteur.clear();
    position_herbe.clear();
    
    CloseWindow();
}

void Renderer::initializeShaders() {
    // Charger shaders (depuis main.cpp)
    shadowShader = LoadShader(TextFormat("include/shaders/resources/shaders/glsl%i/lighting.vs", 330),
                             TextFormat("include/shaders/resources/shaders/glsl%i/lighting.fs", 330));
    
    herbe_shader = LoadShader("ressources/custom_shader/glsl330/herbe_shader.vs",
                             "ressources/custom_shader/glsl330/herbe_shader.fs");
    
    pbr_ombre_shader = LoadShader("ressources/custom_shader/glsl330/ombre_pbr.vs",
                                 "ressources/custom_shader/glsl330/ombre_pbr.fs");
    
    shader_taille = LoadShader("include/shaders/resources/shaders/glsl100/base.vs", 
                              "include/shaders/resources/shaders/glsl100/base.fs");
    
    // Configuration shaders
    shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");
    
    lightDir = Vector3Normalize((Vector3){ 0.35f, -1.0f, -0.35f });
    lightColor = WHITE;
    lightColorNormalized = ColorNormalize(lightColor);
    lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
    lightColLoc = GetShaderLocation(shadowShader, "lightColor");
    SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
    
    // Configuration herbe shader
    SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightPos"), &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightColor"), &lightColorNormalized, SHADER_UNIFORM_VEC4);
    SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightDir"), &lightDir, SHADER_UNIFORM_VEC3);
    
    // Locations shader herbe
    timeLocation = GetShaderLocation(herbe_shader, "time");
    windStrengthLocation = GetShaderLocation(herbe_shader, "windStrength");
    windSpeedLocation = GetShaderLocation(herbe_shader, "windSpeed");
    isGrassLocation = GetShaderLocation(herbe_shader, "isGrass");
    noiseTextureLoc = GetShaderLocation(herbe_shader, "noiseTexture");
    noiseScaleLoc = GetShaderLocation(herbe_shader, "noiseScale");
    windTextureTileSizeLocation = GetShaderLocation(herbe_shader, "windTextureTileSize");
    windVerticalStrengthLocation = GetShaderLocation(herbe_shader, "windVerticalStrength");
    windHorizontalDirectionLocation = GetShaderLocation(herbe_shader, "windHorizontalDirection");
    
    // Shadow mapping
    ambientLoc = GetShaderLocation(shadowShader, "ambient");
    lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
    shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");
    int shadowMapResolution = SHADOWMAP_RESOLUTION;
    SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);
    SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);
    
    // Noise texture
    Image noiseImage = GenImagePerlinNoise(1024, 1024, 0, 0, 100.0f);
    noiseTexture = LoadTextureFromImage(noiseImage);
    UnloadImage(noiseImage);
    
    // Shadow map
    shadowMap = LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
    
    // Light camera
    lightCam = { 0 };
    lightCam.position = Vector3Scale(lightDir, -15.0f);
    lightCam.target = Vector3Zero();
    lightCam.projection = CAMERA_ORTHOGRAPHIC;
    lightCam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    lightCam.fovy = 20.0f;
}

void Renderer::initializeModels() {
    // Charger modèles (depuis main.cpp)
    model_mort = LoadModel("models/arb_mort/scene.gltf");
    model_sapin = LoadModel("models/pine_tree/scene.glb");
    model_buisson_europe = LoadModel("models/buisson/foret_classique/scene.gltf");
    texture_buisson_europe = LoadTexture("models/buisson/foret_classique/textures/gbushy_baseColor.png");
    model_buisson_europe.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_buisson_europe;
    
    model_herbe_instance = LoadModel("models/herbe/lpherbe.glb");
    
    model_acacia = LoadModel("models/acacia/scene.gltf");
    texture_acacia = LoadTexture("models/acacia/Acacia_Dry_Green__Mature__Acacia_Leaves_1_baked_Color-Acacia_Dry_Green__Mature__Acacia_Leaves_1_baked_Opacity.png");
    model_acacia.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_acacia;
    
    // Appliquer shaders aux modèles
    model_sapin.materials[0].shader = shadowShader;
    for (int i = 0; i < model_sapin.materialCount; i++) {
        model_sapin.materials[i].shader = shadowShader;
    }
    
    model_buisson_europe.materials[0].shader = shadowShader;
    for (int i = 0; i < model_buisson_europe.materialCount; i++) {
        model_buisson_europe.materials[i].shader = shadowShader;
    }
    
    model_acacia.materials[0].shader = shadowShader;
    for (int i = 0; i < model_acacia.materialCount; i++) {
        model_acacia.materials[i].shader = shadowShader;
    }
    
    model_mort.materials[0].shader = shadowShader;
    for (int i = 0; i < model_mort.materialCount; i++) {
        model_mort.materials[i].shader = shadowShader;
    }
    
    model_herbe_instance.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_herbe_instance.materialCount; i++) {
        model_herbe_instance.materials[i].shader = herbe_shader;
    }
    
    // Couleurs
    model_sapin.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_buisson_europe.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_acacia.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_mort.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_herbe_instance.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    
    // Modèle vide
    Mesh emptyMesh = GenMeshCube(0.0f, 0.0f, 0.0f);
    emptyModel = LoadModelFromMesh(emptyMesh);
    emptyModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BLANK;
}

void Renderer::initializeGrass(const Simulation& simulation) {
    // Initialiser herbe (depuis main.cpp)
    Vector3 taille_terrain = simulation.getTerrainSize();
    Image image_sol = simulation.getTerrainImage();
    
    models_herbe_vecteur.resize(NBHERBE * NBHERBE, model_herbe_instance);
    position_herbe.resize(NBHERBE * NBHERBE);
    
    herbeCount = 0;
    
    for (int x = 0; x < NBHERBE; x++) {
        for (int z = 0; z < NBHERBE; z++) {
            float posX = (float) x - taille_terrain.x / 2; 
            float posZ = (float) z - taille_terrain.z / 2;
            
            float espacementX = 4.0f / NBHERBE;
            float espacementZ = 4.0f / NBHERBE;
            
            posX = x * espacementX - taille_terrain.x / 2;
            posZ = z * espacementZ - taille_terrain.z / 2;
            
            float offsetX = MathUtils::randomFloat(-0.1f , 0.1f);
            float offsetZ = MathUtils::randomFloat(-0.1f, 0.1f);
            posX += offsetX;
            posZ += offsetZ;
            
            float height = MathUtils::getHeightFromTerrain((Vector3){ posX, 0.0f, posZ }, image_sol, taille_terrain);
            float tauxPente = MathUtils::calculateSlope((Vector3){ posX, 0.0f, posZ }, image_sol, taille_terrain);
            
            if (tauxPente <= 0.2f) {
                float randomRotationY = MathUtils::randomFloat(0.0f, 2.0f * PI);
                
                Model modelHerbe = model_herbe_instance;
                
                Matrix transform = MatrixIdentity();
                transform = MatrixMultiply(transform, MatrixRotateY(randomRotationY));
                
                modelHerbe.transform = transform;
                
                models_herbe_vecteur[herbeCount] = modelHerbe;
                position_herbe[herbeCount] = (Vector3){ posX, height-0.0f, posZ };
                
                herbeCount++;
            }
        }
    }
}

void Renderer::initializeClouds() {
    Vector3 taille_terrain = { 4, 2, 4 }; // Valeur par défaut
    grandsNuages.push_back(GenererGrandNuage({-taille_terrain.x, 4.0f, 0.0f}, taille_terrain.x * 3.0f, taille_terrain.x * 3.0f, 1, cloudThreshold, noiseScale));
    grandsNuages.push_back(GenererGrandNuage({-taille_terrain.x, 3.0f, 0.0f}, taille_terrain.x * 3.0f, taille_terrain.x * 3.0f, 1, cloudThreshold, noiseScale));
    grandsNuages.push_back(GenererGrandNuage({-taille_terrain.x, 2.0f, 0.0f}, taille_terrain.x * 3.0f, taille_terrain.x * 3.0f, 1, cloudThreshold, noiseScale));
}

void Renderer::beginFrame() {
    BeginDrawing();
}

void Renderer::endFrame() {
    EndDrawing();
}

void Renderer::renderSimulation(const Simulation& simulation) {
   handleCameraInput();
   updateLighting();
   updateClouds(GetFrameTime());
   
   // Gestion changements mode de vue
   if (IsKeyPressed(KEY_T)) {
       int newMode = (simulation.getViewMode() == MODE_NORMAL) ? MODE_TEMPERATURE : MODE_NORMAL;
       const_cast<Simulation&>(simulation).setViewMode(newMode);
       if (newMode == MODE_TEMPERATURE) {
           updateTemperatureTexture(simulation);
       } else {
           model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol;
       }
   }
   
   if (IsKeyPressed(KEY_Y)) {
       int newMode = (simulation.getViewMode() == MODE_NORMAL) ? MODE_HUMIDITE : MODE_NORMAL;
       const_cast<Simulation&>(simulation).setViewMode(newMode);
       if (newMode == MODE_HUMIDITE) {
           updateHumidityTexture(simulation);
       } else {
           model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol;
       }
   }
   
   // Mettre à jour textures si en mode spécial
   if (simulation.getViewMode() == MODE_TEMPERATURE) {
       updateTemperatureTexture(simulation);
   } else if (simulation.getViewMode() == MODE_HUMIDITE) {
       updateHumidityTexture(simulation);
   }
   
   // Rendu shadow map d'abord
   renderShadowMap(simulation);
   
   // Rendu scène principale
   renderMainScene(simulation);
   
   renderUI(const_cast<Simulation&>(simulation));
}

void Renderer::handleCameraInput() {
   // Contrôles caméra (depuis main.cpp)
   if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) isRotating = true;
   if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) isRotating = false;

   if (isRotating) {
       Vector2 mouseDelta = GetMouseDelta();
       angleX -= mouseDelta.y * 0.2f;
       angleY -= mouseDelta.x * 0.2f;
   }
   
   distance_cam -= GetMouseWheelMove() * 0.5f;
   if (distance_cam < 2.0f) distance_cam = 2.0f;
   if (distance_cam > 200.0f) distance_cam = 200.0f;

   if (angleX > 89.0f) angleX = 89.0f;
   if (angleX < -89.0f) angleX = -89.0f;

   float radAngleX = DEG2RAD * angleX;
   float radAngleY = DEG2RAD * angleY;

   camera.position.x = distance_cam * cos(radAngleX) * sin(radAngleY);
   camera.position.y = distance_cam * sin(radAngleX);
   camera.position.z = distance_cam * cos(radAngleX) * cos(radAngleY);

   ShowCursor();
}

void Renderer::updateLighting() {
   // Mise à jour éclairage (depuis main.cpp)
   float sunAngle = ((timeOfDay - 6.0f) / 12.0f) * PI;

   lightDir = {
       cosf(sunAngle),
       -sinf(sunAngle),
       0.0f
   };
   lightDir = Vector3Normalize(lightDir);

   lightCam.position = Vector3Scale(lightDir, -15.0f);
   lightCam.target = Vector3Zero();

   float lightIntensity = 1.0f;
   if (timeOfDay < 6.0f || timeOfDay > 18.0f) {
       lightIntensity = 0.0f;
   } else if (timeOfDay < 8.0f || timeOfDay > 16.0f) {
       lightIntensity = 0.6f;
   }

   // Couleur du soleil avec météo
   Color couleur_meteo = Color{255, 255, 255, 255}; // Par défaut
   
   lightColor = (Color){
       MathUtils::getSunColor(timeOfDay).r/2 + couleur_meteo.r/2,
       MathUtils::getSunColor(timeOfDay).g/2 + couleur_meteo.g/2,
       MathUtils::getSunColor(timeOfDay).b/2 + couleur_meteo.b/2,
       MathUtils::getSunColor(timeOfDay).a/2 + couleur_meteo.a/2
   };
   
   lightColorNormalized = ColorNormalize(lightColor);

   SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
   SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);

   SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightPos"), &lightDir, SHADER_UNIFORM_VEC3);
   SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightColor"), &lightColorNormalized, SHADER_UNIFORM_VEC4);
   SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightDir"), &lightDir, SHADER_UNIFORM_VEC3);
   
   time += GetFrameTime();
   SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "time"), &time, SHADER_UNIFORM_FLOAT);
   SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "windStrength"), &windStrength, SHADER_UNIFORM_FLOAT);
   SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "windSpeed"), &windSpeed, SHADER_UNIFORM_FLOAT);
   SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "noiseScale"), &noiseScale, SHADER_UNIFORM_FLOAT);
   SetShaderValueTexture(herbe_shader, GetShaderLocation(herbe_shader, "noiseTexture"), noiseTexture);
}

void Renderer::renderShadowMap(const Simulation& simulation) {
   // Rendu shadow map (depuis main.cpp)
   Matrix lightView;
   Matrix lightProj;
   BeginTextureMode(shadowMap);
   ClearBackground(SKYBLUE);

   BeginMode3D(lightCam);
       lightView = rlGetMatrixModelview();
       lightProj = rlGetMatrixProjection();
       dessine_scene(camera, simulation);
       
       int isGrass = 1;
       SetShaderValue(herbe_shader, timeLocation, &time, SHADER_UNIFORM_FLOAT);
       SetShaderValue(herbe_shader, windStrengthLocation, &windStrength, SHADER_UNIFORM_FLOAT);
       SetShaderValue(herbe_shader, windSpeedLocation, &windSpeed, SHADER_UNIFORM_FLOAT);
       SetShaderValue(herbe_shader, isGrassLocation, &isGrass, SHADER_UNIFORM_INT);
       SetShaderValue(herbe_shader, windTextureTileSizeLocation, &windTextureTileSize, SHADER_UNIFORM_FLOAT);
       SetShaderValue(herbe_shader, windVerticalStrengthLocation, &windVerticalStrength, SHADER_UNIFORM_FLOAT);
       SetShaderValueTexture(herbe_shader, noiseTextureLoc, noiseTexture);
       SetShaderValue(herbe_shader, windHorizontalDirectionLocation, &windHorizontalDirection, SHADER_UNIFORM_VEC2);
       SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "isGrass"), &isGrass, SHADER_UNIFORM_INT);
       SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "time"), &time, SHADER_UNIFORM_FLOAT);
       SetShaderValue(herbe_shader, noiseScaleLoc, &noiseScale, SHADER_UNIFORM_FLOAT);

       rlEnableBackfaceCulling();
       renderGrass(simulation);
       isGrass = 0;
       SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "isGrass"), &isGrass, SHADER_UNIFORM_INT);
       rlDisableBackfaceCulling();
       
       renderClouds();
       
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

void Renderer::renderMainScene(const Simulation& simulation) {
   // Rendu scène principale (depuis main.cpp)
   ClearBackground(SKYBLUE);

   BeginMode3D(camera);
       dessine_scene(camera, simulation);
       
       int isGrass = 1;
       SetShaderValue(herbe_shader, timeLocation, &time, SHADER_UNIFORM_FLOAT);
       SetShaderValue(herbe_shader, windStrengthLocation, &windStrength, SHADER_UNIFORM_FLOAT);
       SetShaderValue(herbe_shader, windSpeedLocation, &windSpeed, SHADER_UNIFORM_FLOAT);
       SetShaderValue(herbe_shader, isGrassLocation, &isGrass, SHADER_UNIFORM_INT);
       SetShaderValue(herbe_shader, windTextureTileSizeLocation, &windTextureTileSize, SHADER_UNIFORM_FLOAT);
       SetShaderValue(herbe_shader, windVerticalStrengthLocation, &windVerticalStrength, SHADER_UNIFORM_FLOAT);
       SetShaderValueTexture(herbe_shader, noiseTextureLoc, noiseTexture);
       SetShaderValue(herbe_shader, windHorizontalDirectionLocation, &windHorizontalDirection, SHADER_UNIFORM_VEC2);
       SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "isGrass"), &isGrass, SHADER_UNIFORM_INT);
       SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "time"), &time, SHADER_UNIFORM_FLOAT);
       SetShaderValue(herbe_shader, noiseScaleLoc, &noiseScale, SHADER_UNIFORM_FLOAT);

       renderGrass(simulation);
       isGrass = 0;
       SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "isGrass"), &isGrass, SHADER_UNIFORM_INT);
       rlDisableBackfaceCulling();
       
       renderClouds();
       
   EndMode3D();
}

void Renderer::renderGrass(const Simulation& simulation) {
   // Rendu herbe (depuis main.cpp)
   Vector3 taille_terrain = simulation.getTerrainSize();
   const auto& grille = simulation.getGrid();
   
   for (int i = 0; i < herbeCount; i++) {
       int gridX = (int)((position_herbe[i].x + taille_terrain.x / 2) * GRID_SIZE / taille_terrain.x);
       int gridZ = (int)((position_herbe[i].z + taille_terrain.z / 2) * GRID_SIZE / taille_terrain.z);

       gridX = Clamp(gridX, 0, GRID_SIZE - 1);
       gridZ = Clamp(gridZ, 0, GRID_SIZE - 1);

       if (grille[gridX][gridZ].temperature > -20 && grille[gridX][gridZ].temperature < 50) {
           DrawModel(models_herbe_vecteur[i], position_herbe[i], 0.05f, WHITE);
       }
   }
}

void Renderer::renderClouds() {
   // Rendu nuages (depuis main.cpp)
   for (auto& nuage : grandsNuages) {
       for (size_t i = 0; i < nuage.plans.size(); i++) {
           nuage.plans[i].materials[0].shader = shadowShader;
           nuage.plans[i].materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
       }
   }

   rlEnableColorBlend();
   for (auto& nuage : grandsNuages) {
       for (size_t i = 0; i < nuage.plans.size(); i++) {
           DrawModelEx(nuage.plans[i], nuage.positions[i], (Vector3){0, 1, 0}, nuage.rotations[i], 
                      (Vector3){nuage.scales[i], nuage.scales[i], nuage.scales[i]}, WHITE);
       }
   }
}

void Renderer::updateClouds(float deltaTime) {
   // Mise à jour nuages (depuis main.cpp)
   static float accumulatedTime = 0.0f;
   accumulatedTime += deltaTime * 0.5f;
   float timeValue = accumulatedTime;

   const float driftSpeed = 0.2f;
   Vector3 taille_terrain = { 4, 2, 4 }; // Valeur par défaut

   for (auto& nuage : grandsNuages) {
       static float distanceParcourue = 0.0f;
       float distanceDeReset = taille_terrain.x * 3.0f;

       for (size_t i = 0; i < nuage.positions.size(); i++) {
           nuage.positions[i].x += deltaTime * nuage.vitesseDefile;
       }

       distanceParcourue += deltaTime * nuage.vitesseDefile;

       if (distanceParcourue >= distanceDeReset) {
           for (size_t i = 0; i < nuage.positions.size(); i++) {
               nuage.positions[i].x = -taille_terrain.x;
           }
           distanceParcourue = 0.0f;
       }
   }
}

void Renderer::updateTemperatureTexture(const Simulation& simulation) {
   // Mise à jour texture température (depuis main.cpp)
   Image tempImage = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);
   const auto& grille = simulation.getGrid();

   for (int x = 0; x < GRID_SIZE; x++) {
       for (int z = 0; z < GRID_SIZE; z++) {
           Color tempColor = MathUtils::getTemperatureColor(grille[x][z].temperature, 
                                                          simulation.getMinTemp(), 
                                                          simulation.getMaxTemp());
           ImageDrawPixel(&tempImage, x, z, tempColor);
       }
   }
   UpdateTexture(temperatureTexture, tempImage.data);
   UnloadImage(tempImage);
   model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = temperatureTexture;
}

void Renderer::updateHumidityTexture(const Simulation& simulation) {
   // Mise à jour texture humidité (depuis main.cpp)
   Image humImage = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);
   const auto& grille = simulation.getGrid();

   for (int x = 0; x < GRID_SIZE; x++) {
       for (int z = 0; z < GRID_SIZE; z++) {
           Color humColor = MathUtils::getHumidityColor(grille[x][z].humidite, 
                                                       simulation.getMinHum(), 
                                                       simulation.getMaxHum());
           ImageDrawPixel(&humImage, x, z, humColor);
       }
   }
   UpdateTexture(temperatureTexture, humImage.data);
   UnloadImage(humImage);
   model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = temperatureTexture;
}

void Renderer::dessine_scene(Camera camera, const Simulation& simulation) {
   // Fonction dessine_scene (depuis main.cpp)
   const auto& grille = simulation.getGrid();
   Vector3 mapPosition = simulation.getMapPosition();
   int viewMode = simulation.getViewMode();
   int minTemp = simulation.getMinTemp();
   int maxTemp = simulation.getMaxTemp();
   int minHum = simulation.getMinHum();
   int maxHum = simulation.getMaxHum();
   
   struct SceneObject {
       Vector3 position;
       Model *model;
       float depth;
       Color color = WHITE;
   };
   
   SceneObject sceneObjects[GRID_SIZE * GRID_SIZE + 1];
   int objectCount = 0;
   
   // Ajouter le sol
   sceneObjects[objectCount].position = mapPosition;
   sceneObjects[objectCount].model = &model_sol;
   sceneObjects[objectCount].depth = Vector3Distance(camera.position, mapPosition);
   objectCount++;
   
   // Ajouter les plantes
   for (int x = 0; x < GRID_SIZE; x++) {
       for (int z = 0; z < GRID_SIZE; z++) {
           if (grille[x][z].active) {
               sceneObjects[objectCount].position = grille[x][z].position;
               sceneObjects[objectCount].model = const_cast<Model*>(&grille[x][z].plante.model);
               float scale = grille[x][z].plante.taille;
               sceneObjects[objectCount].model->transform = MatrixScale(scale, scale, scale);
               sceneObjects[objectCount].depth = Vector3Distance(camera.position, grille[x][z].position);
               sceneObjects[objectCount].color = grille[x][z].plante.couleur;
               objectCount++;
           }
       }
   }
   
   // Fonction de comparaison pour qsort
   auto CompareSceneObjects = [](const void *a, const void *b) -> int {
       SceneObject *objA = (SceneObject *)a;
       SceneObject *objB = (SceneObject *)b;
       return (objA->depth < objB->depth) - (objA->depth > objB->depth);
   };
   
   // Trier les objets
   qsort(sceneObjects, objectCount, sizeof(SceneObject), CompareSceneObjects);
   
   // Dessiner les objets
   for (int i = 0; i < objectCount; i++) {
       if (viewMode == MODE_NORMAL) {
           if (sceneObjects[i].model == &model_sol) {
               DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 0.1f, WHITE);
           } else {
               DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 1.0f, sceneObjects[i].color);
           }
       } else if (viewMode == MODE_TEMPERATURE) {
           if (sceneObjects[i].model == &model_sol) {
               DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 0.1f, WHITE);
           } else {
               for (int x = 0; x < GRID_SIZE; x++) {
                   for (int z = 0; z < GRID_SIZE; z++) {
                       if (Vector3Equals(grille[x][z].position, sceneObjects[i].position)) {
                           Color tempColor = MathUtils::getTemperatureColor(grille[x][z].temperature, minTemp, maxTemp);
                           DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 1.0f, tempColor);
                           break;
                       }
                   }
               }
           }
       } else if (viewMode == MODE_HUMIDITE) {
           if (sceneObjects[i].model == &model_sol) {
               DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 0.1f, WHITE);
           } else {
               for (int x = 0; x < GRID_SIZE; x++) {
                   for (int z = 0; z < GRID_SIZE; z++) {
                       if (Vector3Equals(grille[x][z].position, sceneObjects[i].position)) {
                           Color humColor = MathUtils::getHumidityColor(grille[x][z].humidite, minHum, maxHum);
                           DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 1.0f, humColor);
                           break;
                       }
                   }
               }
           }
       }
   }
}

void Renderer::renderUI(Simulation& simulation) {
   // Interface utilisateur (depuis main.cpp)
   if (simulation.getViewMode() == MODE_TEMPERATURE) {
       DrawText("Mode température - Appuyez sur T pour revenir", 10, 60, 20, BLACK);
       DrawText(TextFormat("Min: %d°C", simulation.getMinTemp()), 10, 80, 20, BLUE);
       DrawText(TextFormat("Max: %d°C", simulation.getMaxTemp()), 10, 100, 20, RED);
   }
   if (simulation.getViewMode() == MODE_HUMIDITE) {
       DrawText("Mode humidite - Appuyez sur Y pour revenir", 10, 60, 20, BLACK);
       DrawText(TextFormat("Min: %d", simulation.getMinHum()), 10, 80, 20, BLUE);
       DrawText(TextFormat("Max: %d", simulation.getMaxHum()), 10, 100, 20, RED);
   }
   
   DrawText(" d'objets 3D - Utilisez la souris pour naviguer", 10, 10, 20, DARKGRAY);
   DrawText("Maintenez le clic droit pour tourner la scène", 10, 25, 20, DARKGRAY);
   
   if (GuiButton((Rectangle){ 100, 370, 200, 30 }, "Soleil")) {
       simulation.changeWeather("Soleil");
   }

   if (GuiButton((Rectangle){ 100, 410, 200, 30 }, "Pluie")) {
       simulation.changeWeather("Pluie");
   }

   if (GuiButton((Rectangle){ 100, 450, 200, 30 }, "Neige")) {
       simulation.changeWeather("Neige");
   }
   
   GuiSliderBar((Rectangle){ 100, 310, 200, 20 }, "Wind Speed", TextFormat("%.2f", windSpeed), &windSpeed, 0.0f, 3.0f);
   GuiSliderBar((Rectangle){ 100, 340, 200, 20 }, "Wind Strength", TextFormat("%.2f", windStrength), &windStrength, 0.0f, 2.0f);
   
   // Mise à jour nuages si paramètres changent
   static float lastCloudThreshold = cloudThreshold;
   static float lastNoiseScale = noiseScale;
   if (fabsf(cloudThreshold - lastCloudThreshold) > 0.01f || fabsf(noiseScale - lastNoiseScale) > 0.1f) {
       for (auto& nuage : grandsNuages) {
           for (size_t i = 0; i < nuage.textures.size(); i++) {
               UnloadTexture(nuage.textures[i]);
               nuage.textures[i] = GenererTextureNuage(256, 256, GetRandomValue(0, 1000), cloudThreshold, noiseScale);
               nuage.plans[i].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = nuage.textures[i];
           }
       }
       lastCloudThreshold = cloudThreshold;
       lastNoiseScale = noiseScale;
   }
   
   DrawFPS(10, 40);
   
   GuiSliderBar((Rectangle){ 100, 100, 300, 20 }, "Time of Day", TextFormat("%.0f:00", timeOfDay), &timeOfDay, 0.0f, 24.0f);
   DrawText(TextFormat("Time: %.0f:00", timeOfDay), 310, 20, 20, DARKGRAY);
}

// Fonction helper pour shadow mapping (depuis main.cpp)
RenderTexture2D LoadShadowmapRenderTexture(int width, int height) {
   RenderTexture2D target = { 0 };

   target.id = rlLoadFramebuffer();
   target.texture.width = width;
   target.texture.height = height;

   if (target.id > 0) {
       rlEnableFramebuffer(target.id);

       target.depth.id = rlLoadTextureDepth(width, height, false);
       target.depth.width = width;
       target.depth.height = height;
       target.depth.format = 19;
       target.depth.mipmaps = 1;

       rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

       if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

       rlDisableFramebuffer();
   } else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

   return target;
}

void UnloadShadowmapRenderTexture(RenderTexture2D target) {
   if (target.id > 0) {
       rlUnloadFramebuffer(target.id);
   }
}