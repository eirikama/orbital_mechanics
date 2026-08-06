#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct BodyData {
  std::vector<double> x, y, z;
};

struct Star {
  Vector3 position;
  Color color;
};

void DrawFlatRing(float innerRadius, float outerRadius, int segments,
                  Color color) {
  rlBegin(RL_TRIANGLES);
  for (int i = 0; i < segments; i++) {
    float angle1 = (float)i / segments * 2.0f * PI;
    float angle2 = (float)(i + 1) / segments * 2.0f * PI;

    // Inner vertices
    Vector3 v1 = {innerRadius * cosf(angle1), innerRadius * sinf(angle1), 0.0f};
    Vector3 v2 = {innerRadius * cosf(angle2), innerRadius * sinf(angle2), 0.0f};

    // Outer vertices
    Vector3 v3 = {outerRadius * cosf(angle1), outerRadius * sinf(angle1), 0.0f};
    Vector3 v4 = {outerRadius * cosf(angle2), outerRadius * sinf(angle2), 0.0f};

    rlColor4ub(color.r, color.g, color.b, color.a);

    // Triangle 1
    rlVertex3f(v1.x, v1.y, v1.z);
    rlVertex3f(v3.x, v3.y, v3.z);
    rlVertex3f(v2.x, v2.y, v2.z);

    // Triangle 2
    rlVertex3f(v2.x, v2.y, v2.z);
    rlVertex3f(v3.x, v3.y, v3.z);
    rlVertex3f(v4.x, v4.y, v4.z);
  }
  rlEnd();
}

void DrawRing3D(Vector3 center, float innerRadius, float outerRadius,
                float startAngle, float endAngle, int segments, Color color) {
  float step = (endAngle - startAngle) / segments;
  for (int i = 0; i < segments; i++) {
    float a1 = (startAngle + i * step) * DEG2RAD;
    float a2 = (startAngle + (i + 1) * step) * DEG2RAD;

    // X and Y change, Z stays at the planet's depth
    Vector3 v1 = {center.x + cosf(a1) * outerRadius,
                  center.y + sinf(a1) * outerRadius, center.z};
    Vector3 v2 = {center.x + cosf(a2) * outerRadius,
                  center.y + sinf(a2) * outerRadius, center.z};

    DrawLine3D(v1, v2, color);
  }
}

int main() {
  // --- Constants & Config ---
  const int CelestialBodies = 12;

  const std::array<std::string, CelestialBodies> names = {
      "Mercury", "Venus",  "Earth",   "Moon",  "Mars",   "Jupiter",
      "Saturn",  "Uranus", "Neptune", "Pluto", "Halley", "Sun"};

  std::map<std::string, float> proportion = {
      {"Sun", 1.0f},       {"Mercury", 0.0035f}, {"Venus", 0.0087f},
      {"Earth", 0.0092f},  {"Mars", 0.0049f},    {"Jupiter", 0.1004f},
      {"Saturn", 0.0836f}, {"Uranus", 0.0364f},  {"Neptune", 0.0354f},
      {"Pluto", 0.002f},   {"Halley", 0.0018f},  {"Moon", 0.0025f}};

  std::map<std::string, Color> colorMap = {
      {"Sun", YELLOW},      {"Mercury", GRAY},   {"Venus", ORANGE},
      {"Earth", BLUE},      {"Mars", RED},       {"Jupiter", BROWN},
      {"Saturn", GOLD},     {"Uranus", SKYBLUE}, {"Neptune", DARKBLUE},
      {"Pluto", LIGHTGRAY}, {"Halley", PINK},    {"Moon", LIGHTGRAY}};

  std::map<std::string, float> axialTilt = {
      {"Sun", 7.25f},     {"Mercury", 0.03f}, {"Venus", 177.3f},
      {"Earth", 23.44f},  {"Mars", 25.19f},   {"Jupiter", 3.13f},
      {"Saturn", 26.73f}, {"Uranus", 97.77f}, {"Neptune", 28.32f},
      {"Pluto", 122.5f},  {"Halley", 18.0f},  {"Moon", 6.68f}};

  std::map<std::string, float> rotSpeed = {
      {"Sun", 0.05f},   {"Mercury", 0.1f}, {"Venus", -0.05f},
      {"Earth", 1.0f},  {"Mars", 0.9f},    {"Jupiter", 2.5f},
      {"Saturn", 2.2f}, {"Uranus", -1.5f}, {"Neptune", 1.8f},
      {"Pluto", 0.2f},  {"Halley", 0.5f},  {"Moon", 0.03f}};

  // Orbital periods in Earth Years
  std::map<std::string, float> orbitalPeriods = {
      {"Mercury", 0.241f}, {"Venus", 0.615f},   {"Earth", 1.0f},
      {"Mars", 1.881f},    {"Jupiter", 11.86f}, {"Saturn", 29.45f},
      {"Uranus", 84.01f},  {"Neptune", 164.8f}, {"Pluto", 248.1f},
      {"Halley", 75.3f},   {"Moon", 0.0748f} // Moon is relative to Earth, but
                                             // works here
  };

  std::ifstream infile("../data/RK4Data.bin", std::ios::binary | std::ios::ate);
  if (!infile.is_open()) {
    std::cerr << "Error: Could not find ../data/RK4Data.bin\n";
    return 1;
  }

  // Determine file size and number of frames
  std::streamsize fileSize = infile.tellg();
  infile.seekg(0, std::ios::beg);

  int numBodiesInFile;
  infile.read(reinterpret_cast<char *>(&numBodiesInFile), sizeof(int));

  // Calculate how many doubles are in the file
  size_t dataSize = (fileSize - sizeof(int)) / sizeof(double);
  std::vector<double> allData(dataSize);
  infile.read(reinterpret_cast<char *>(allData.data()),
              dataSize * sizeof(double));
  infile.close();

  const int sampleStep = 1;
  std::map<std::string, BodyData> orbit;

  // Organize into BodyData
  int totalFrames = dataSize / (6 * numBodiesInFile);
  for (int i = 0; i < CelestialBodies; ++i) {
    BodyData bd;
    for (int f = 0; f < totalFrames; f += sampleStep) {
      int offset = f * (6 * numBodiesInFile) + (i * 6);
      bd.x.push_back((float)allData[offset]);
      bd.y.push_back((float)allData[offset + 1]);
      bd.z.push_back((float)allData[offset + 2]);
    }
    orbit[names[i]] = std::move(bd);
  }

  // 1. Temporary structure to hold the name and its furthest recorded point
  struct OrbitInfo {
    std::string name;
    float maxReach;
  };
  std::vector<OrbitInfo> reachList;

  // Reference the Sun's data
  BodyData &sun = orbit["Sun"];

  // 2. Calculate the maximum distance each body ever reaches from the Sun
  for (const auto &name : names) {
    if (name == "Sun") {
      reachList.push_back({name, 0.0f}); // Sun is the center
      continue;
    }

    BodyData &bd = orbit[name];
    float maxR = 0.0f;

    // Loop through every frame to find the absolute furthest point (Aphelion)
    for (size_t f = 0; f < bd.x.size(); ++f) {
      float dx = bd.x[f] - sun.x[f];
      float dy = bd.y[f] - sun.y[f];
      float dz = bd.z[f] - sun.z[f];
      float dist = sqrtf(dx * dx + dy * dy + dz * dz);

      if (dist > maxR)
        maxR = dist;
    }
    reachList.push_back({name, maxR});
  }

  // 3. Sort the list in Ascending Order (Closest Max -> Furthest Max)
  std::sort(reachList.begin(), reachList.end(),
            [](const OrbitInfo &a, const OrbitInfo &b) {
              return a.maxReach > b.maxReach;
            });

  // 4. Overwrite the displayOrder with the sorted names
  std::vector<std::string> displayOrder;
  for (const auto &item : reachList) {
    displayOrder.push_back(item.name);
  }

  // --- Raylib Setup ---
  const int screenW = 1280, screenH = 720;
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(screenW, screenH, "Solar System Sim - Textured Edition");
  SetTargetFPS(60);

  // Load shader
  Shader lightShader = LoadShader("../data/lighting.vs", "../data/lighting.fs");

  // Get uniform locations
  int sunPosLoc = GetShaderLocation(lightShader, "sunPos");
  // int planetPosLoc = GetShaderLocation(lightShader, "planetPos");

  Camera3D camera = {0};
  camera.position = {0.0f, 0.0f, 100.0f};
  camera.target = {0.0f, 0.0f, 0.0f};
  camera.up = {0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  Camera3D legendCamera = {0};
  legendCamera.position = {0.0f, 0.0f, 100.0f};
  legendCamera.target = {0.0f, 0.0f, 0.0f};
  legendCamera.up = {0.0f, 1.0f, 0.0f};
  legendCamera.fovy = 45.0f;
  legendCamera.projection = CAMERA_PERSPECTIVE;

  // --- 3D Model & Texture Loading ---
  Mesh sphereMesh = GenMeshSphere(1.0f, 128, 128);
  std::map<std::string, Model> planetModels;
  std::map<std::string, bool>
      hasTexture; // Track if a texture loaded successfully

  for (const auto &name : names) {
    planetModels[name] = LoadModelFromMesh(sphereMesh);

    if (name != "Sun") {
      planetModels[name].materials[0].shader = lightShader;
    }

    // Attempt to load texture (Supports .png or .jpg)
    std::string texPath = TextFormat("../data/textures/%s.jpg", name.c_str());
    Texture2D tex = LoadTexture(texPath.c_str());

    if (tex.id != 0) {         // If texture loaded successfully
      GenTextureMipmaps(&tex); // Generate the smaller versions
      SetTextureFilter(
          tex, TEXTURE_FILTER_TRILINEAR); // Smooth transitions between them
      planetModels[name].materials[0].maps[MATERIAL_MAP_ALBEDO].texture = tex;
      hasTexture[name] = true;
    } else {
      hasTexture[name] = false;
    }
  }

  std::vector<Star> stars;
  for (int i = 0; i < 2500; i++) {
    float theta = GetRandomValue(0, 360) * DEG2RAD;
    float phi = GetRandomValue(0, 180) * DEG2RAD;
    float radius = GetRandomValue(500, 800);
    stars.push_back({{radius * sinf(phi) * cosf(theta),
                      radius * sinf(phi) * sinf(theta), radius * cosf(phi)},
                     (Color){(unsigned char)GetRandomValue(150, 255),
                             (unsigned char)GetRandomValue(150, 255),
                             (unsigned char)GetRandomValue(200, 255), 255}});
  }

  float bodyScaleFactor = 0.5f; // This is your default "half-size" factor
  auto GetBodySize = [&](const std::string &name) {
    if (name == "Sun")
      return 0.2f;
    float baseSize = (name == "Moon") ? 1.0f : 1.0f;
    return bodyScaleFactor * baseSize *
           (0.2f * std::log10(proportion[name] * 100.0f + 1.0f));
  };

  std::map<std::string, std::vector<Vector3>> traj;
  RenderTexture2D target = LoadRenderTexture(screenW, screenH);
  int frameIndex = 0, focusIndex = -1, Nframes = orbit["Sun"].x.size();
  float rotAngle = 0.0f;

  double currentFrameTime = 0.0;
  float playbackSpeed = 1000.0f; // Adjust this to fly through time!
  bool isPaused = false;
  float cameraDistance = 40.0f; // Initial distance
  bool isMoonCamMode = false;

  // --- Main Loop ---
  while (!WindowShouldClose()) {
    rotAngle += 0.4f;
    float pulse = sinf(GetTime() * 2.0f) * 0.005f;

    // Controls
    if (IsKeyPressed(KEY_TAB))
      focusIndex = (focusIndex + 1) % CelestialBodies;
    if (IsKeyPressed(KEY_BACKSPACE))
      focusIndex = -1;

    // Camera Focus
    // --- Camera Focus Logic ---
    if (IsKeyPressed(KEY_M))
      isMoonCamMode = !isMoonCamMode;

    // Pan / Zoom
    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
      Vector2 d = GetMouseDelta();
      camera.position =
          Vector3Add(camera.position, Vector3Scale(camera.up, d.y * 0.1f));
    }

    float targetCameraDistance = 40.0f;

    // --- 1. Zoom Logic (Modify cameraDistance instead of position directly)
    float w = GetMouseWheelMove();
    if (w != 0) {
      float zoomSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 0.05f : 0.2f;
      cameraDistance -= w * cameraDistance * zoomSpeed;
    }

    // Keep distance within reasonable bounds
    if (cameraDistance < 0.001f)
      cameraDistance = 0.001f;
    if (cameraDistance > 1500.0f)
      cameraDistance = 1500.0f;

    // --- 1. Determine the Global Reference Position in Double Precision ---
    // Use double for x,y,z here to match your new BodyData
    struct Double3 {
      double x, y, z;
    };
    Double3 refPos = {0.0, 0.0, 0.0};

    if (focusIndex != -1) {
      std::string focusName = names[focusIndex];
      // We use the same interpolation logic as the planet rendering for
      // perfectly smooth focus
      int idx = (int)currentFrameTime;
      double fraction = currentFrameTime - (double)idx;

      refPos.x =
          orbit[focusName].x[idx] +
          (orbit[focusName].x[idx + 1] - orbit[focusName].x[idx]) * fraction;
      refPos.y =
          orbit[focusName].y[idx] +
          (orbit[focusName].y[idx + 1] - orbit[focusName].y[idx]) * fraction;
      refPos.z =
          orbit[focusName].z[idx] +
          (orbit[focusName].z[idx + 1] - orbit[focusName].z[idx]) * fraction;
    }

    // --- 2. Update Camera Target ---
    if (focusIndex != -1) {
      // When focused, the body is rendered at (0,0,0) relative to refPos
      camera.target = Vector3Lerp(camera.target, {0.0f, 0.0f, 0.0f}, 0.2f);
    } else {
      // If following nothing, target the Sun's precise interpolated position
      int idx = (int)currentFrameTime;
      double fraction = currentFrameTime - (double)idx;

      double sx = orbit["Sun"].x[idx] +
                  (orbit["Sun"].x[idx + 1] - orbit["Sun"].x[idx]) * fraction;
      double sy = orbit["Sun"].y[idx] +
                  (orbit["Sun"].y[idx + 1] - orbit["Sun"].y[idx]) * fraction;
      double sz = orbit["Sun"].z[idx] +
                  (orbit["Sun"].z[idx + 1] - orbit["Sun"].z[idx]) * fraction;

      Vector3 sunGlobal = {(float)sx, (float)sy, (float)sz};
      camera.target = Vector3Lerp(camera.target, sunGlobal, 0.2f);
    }

    // --- 3. Update Camera Position ---
    // This part MUST run every frame, even if focusIndex == -1
    if (isMoonCamMode && focusIndex != -1 && names[focusIndex] == "Moon") {
      // Top-down Z-axis view for the Moon
      camera.position = {camera.target.x, camera.target.y,
                         camera.target.z + cameraDistance};
      camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    } else {
      // Normal 3D Orbit view
      camera.up = (Vector3){0.0f, 1.0f, 0.0f};
      Vector3 dir =
          Vector3Normalize(Vector3Subtract(camera.position, camera.target));

      // Safety check: if dir is zero (camera is on top of target), push it back
      // slightly
      if (fabs(dir.z) < 0.1f)
        dir.z = 0.1f;

      camera.position =
          Vector3Add(camera.target, Vector3Scale(dir, cameraDistance));
    }
    BeginTextureMode(target);
    ClearBackground((Color){5, 5, 10, 255});

    // ----------------------------------------------------
    // 1. MAIN SIMULATION DRAWING
    // ----------------------------------------------------
    BeginMode3D(camera);

    float nearPlane =
        cameraDistance * 0.01f; // Always see things up to 1% of your distance
    if (nearPlane < 0.0001f)
      nearPlane = 0.0001f;

    Matrix proj =
        MatrixPerspective(camera.fovy * DEG2RAD, (float)screenW / screenH,
                          nearPlane, // near plane
                          2000.0f    // far plane
        );
    rlSetMatrixProjection(proj);

    for (const auto &s : stars)
      DrawPoint3D(s.position, s.color);

    // --- 2. Calculate Precise Sun Position for Lighting ---
    int idx = (int)currentFrameTime;
    double fraction = currentFrameTime - (double)idx;

    // Interpolate Sun's global position in double
    double sunGX =
        (double)orbit["Sun"].x[idx] +
        ((double)orbit["Sun"].x[idx + 1] - (double)orbit["Sun"].x[idx]) *
            fraction;
    double sunGY =
        (double)orbit["Sun"].y[idx] +
        ((double)orbit["Sun"].y[idx + 1] - (double)orbit["Sun"].y[idx]) *
            fraction;
    double sunGZ =
        (double)orbit["Sun"].z[idx] +
        ((double)orbit["Sun"].z[idx + 1] - (double)orbit["Sun"].z[idx]) *
            fraction;

    // Convert to relative float for the shader and rendering
    Vector3 sunRenderPos = {(float)(sunGX - refPos.x),
                            (float)(sunGY - refPos.y),
                            (float)(sunGZ - refPos.z)};

    // --- 3. Moon Camera Mode Helper (High Precision) ---
    if (isMoonCamMode) {
      // Helper to get interpolated, RELATIVE positions in double precision
      auto GetPreciseRenderPos = [&](const std::string &name) -> Vector3 {
        // 1. Interpolate global position in double
        double gx =
            (double)orbit[name].x[idx] +
            ((double)orbit[name].x[idx + 1] - (double)orbit[name].x[idx]) *
                fraction;
        double gy =
            (double)orbit[name].y[idx] +
            ((double)orbit[name].y[idx + 1] - (double)orbit[name].y[idx]) *
                fraction;
        double gz =
            (double)orbit[name].z[idx] +
            ((double)orbit[name].z[idx + 1] - (double)orbit[name].z[idx]) *
                fraction;

        // 2. Subtract focus reference in double, then cast to float
        return (Vector3){(float)(gx - refPos.x), (float)(gy - refPos.y),
                         (float)(gz - refPos.z)};
      };

      Vector3 earthRel = GetPreciseRenderPos("Earth");
      Vector3 moonRel = GetPreciseRenderPos("Moon");

      // 4. Calculate Distance and Draw (The ring will now be perfectly
      // centered)
      float orbitalDist = Vector3Distance(
          earthRel,
          moonRel); // 4. Draw using the Render Positions (not global ones!)
      // This ensures the ring and line stay "glued" to the planet models
      DrawRing3D(earthRel, orbitalDist, orbitalDist, 0, 360, 100,
                 Fade(WHITE, 0.4f));
      DrawLine3D(earthRel, moonRel, Fade(GOLD, 0.6f));
    }

    // 1. Controls
    if (IsKeyDown(KEY_PERIOD))
      playbackSpeed *= 1.05f;
    if (IsKeyDown(KEY_COMMA))
      playbackSpeed *= 0.95f;
    if (IsKeyPressed(KEY_SPACE))
      isPaused = !isPaused;
    // --- Body Scaling Controls ---
    if (IsKeyDown(KEY_UP))
      bodyScaleFactor += 0.01f;
    if (IsKeyDown(KEY_DOWN))
      bodyScaleFactor -= 0.01f;

    // Clamp the scale so things don't disappear or get too huge
    if (bodyScaleFactor < 0.01f)
      bodyScaleFactor = 0.01f;
    if (bodyScaleFactor > 5.0f)
      bodyScaleFactor = 5.0f;

    // 2. Logic
    int Nframes = orbit["Sun"].x.size();
    frameIndex = idx;

    float saveDt = 0.00004f;

    for (const auto &n : names) {
      // 1. Calculate global position in double precision
      double gx, gy, gz;

      if (idx < Nframes - 1) {
        // Interpolation using double to keep every decimal of precision
        gx = (double)orbit[n].x[idx] +
             ((double)orbit[n].x[idx + 1] - (double)orbit[n].x[idx]) * fraction;
        gy = (double)orbit[n].y[idx] +
             ((double)orbit[n].y[idx + 1] - (double)orbit[n].y[idx]) * fraction;
        gz = (double)orbit[n].z[idx] +
             ((double)orbit[n].z[idx + 1] - (double)orbit[n].z[idx]) * fraction;
      } else {
        gx = (double)orbit[n].x[Nframes - 1];
        gy = (double)orbit[n].y[Nframes - 1];
        gz = (double)orbit[n].z[Nframes - 1];
      }

      // 2. KEY FIX: Subtract refPos (double) FROM global position (double)
      // This removes the large orbital numbers and leaves only the small
      // relative distance
      Vector3 renderPos = {(float)(gx - refPos.x), (float)(gy - refPos.y),
                           (float)(gz - refPos.z)};

      // 3. Update Shader with the Sun's position relative to the camera focus
      // We already calculated the Sun's global pos (sunGX, sunGY, sunGZ)
      // earlier
      Vector3 relativeSunPos = {(float)(sunGX - refPos.x),
                                (float)(sunGY - refPos.y),
                                (float)(sunGZ - refPos.z)};
      SetShaderValue(lightShader, sunPosLoc, &relativeSunPos,
                     SHADER_UNIFORM_VEC3);

      float animSize = GetBodySize(n);

      // APPLY PULSE TO SIZE if it's the Sun
      float currentAnimSize = animSize;
      if (n == "Sun") {
        currentAnimSize *= (1.0f + pulse * 2.0f);
      }

      float currentRot = rotAngle * rotSpeed[n];
      float tiltRad = axialTilt[n] * DEG2RAD;
      Vector3 tiltAxis = {sinf(tiltRad), cosf(tiltRad), 0.0f};
      Color modelTint = hasTexture[n] ? WHITE : colorMap[n];

      // Inside the planet rendering loop:
      DrawModelEx(planetModels[n], renderPos, tiltAxis, currentRot,
                  {currentAnimSize, currentAnimSize, currentAnimSize},
                  (n == "Sun" ? modelTint : WHITE));

      if (n == "Sun") {
        // STUNNING SUN GLOW using Additive Blending
        BeginBlendMode(BLEND_ADDITIVE);
        rlDisableDepthMask(); // Don't write to depth buffer so it looks like
                              // light

        // Core intense heat
        DrawSphere(renderPos, currentAnimSize * 1.05f, Fade(WHITE, 0.1f));
        // Mid glow
        DrawSphere(renderPos, currentAnimSize * 1.1f,
                   Fade(YELLOW, 0.1f + (pulse * 0.2f)));
        // Outer corona
        DrawSphere(renderPos, currentAnimSize * 1.15f,
                   Fade(ORANGE, 0.1f + (pulse * 0.1f)));

        rlEnableDepthMask();
        EndBlendMode();
        continue; // Skip trail drawing for the Sun
      }

      // FAKED ATMOSPHERES
      if (n == "Earth" || n == "Venus" || n == "Uranus") {
        BeginBlendMode(BLEND_ADDITIVE);
        Color atmosphereColor =
            (n == "Earth") ? SKYBLUE : (n == "Venus") ? ORANGE : LIGHTGRAY;
        DrawSphere(renderPos, currentAnimSize * 1.08f,
                   Fade(atmosphereColor, 0.3f));
        EndBlendMode();
      }

      if (n == "Saturn") {
        rlPushMatrix();
        rlTranslatef(renderPos.x, renderPos.y, renderPos.z);
        rlRotatef(axialTilt[n], 0, 0, 1);

        // Ring A (Outer)
        DrawFlatRing(currentAnimSize * 1.5f, currentAnimSize * 2.2f, 128,
                     Fade(GOLD, 0.4f));
        // Ring B (Bright/Dense)
        DrawFlatRing(currentAnimSize * 1.2f, currentAnimSize * 1.45f, 128,
                     Fade(WHITE, 0.6f));
        // Ring C (Darker inner)
        DrawFlatRing(currentAnimSize * 1.0f, currentAnimSize * 1.15f, 128,
                     Fade(GRAY, 0.3f));

        rlPopMatrix();
      }

      BodyData &bd = orbit[n];
      float period = orbitalPeriods[n];

      // 1. Calculate the window
      int windowSize = (int)((period * 0.5f) / saveDt);
      int endFrame =
          currentFrameTime; // The "head" of the trail is the current frame
      int startFrame = endFrame - windowSize;

      if (startFrame < 0)
        startFrame = 0;
      if (endFrame >= bd.x.size())
        endFrame = bd.x.size() - 1;

      rlBegin(RL_LINES);
      for (int i = startFrame; i < endFrame; i++) {
        float alpha = (float)(i - startFrame) / (endFrame - startFrame + 1);
        Color c = ColorAlpha(colorMap[n], alpha);

        rlColor4ub(c.r, c.g, c.b, c.a);

        // SUBTRACT refPos from every vertex to keep it relative to the focus
        rlVertex3f(bd.x[i] - refPos.x, bd.y[i] - refPos.y, bd.z[i] - refPos.z);
        rlVertex3f(bd.x[i + 1] - refPos.x, bd.y[i + 1] - refPos.y,
                   bd.z[i + 1] - refPos.z);
      }
      rlEnd();
    }
    // 4. Advance Time
    if (!isPaused && idx < Nframes - 1) {
      currentFrameTime += (double)playbackSpeed * (double)GetFrameTime();
      if (currentFrameTime > (double)Nframes - 1)
        currentFrameTime = (double)(Nframes - 1);
    }

    EndMode3D();

    // ----------------------------------------------------
    // 2. STATIC LEGEND DRAWING
    // ----------------------------------------------------

    Rectangle legRec = {10, 40, 240, (float)screenH - 20};
    // DrawRectangleRounded(legRec, 0.05f, 10, (Color){60, 60, 60, 180});

    for (const auto &s : stars) {
      if (s.position.x > -60.0f && s.position.x < 60.0f &&
          s.position.y > -50.0f)
        continue;
      DrawPoint3D(s.position, s.color);
    }

    // Start Clipping: This chops off any pixels drawn outside the legend box
    BeginScissorMode((int)legRec.x, (int)legRec.y, (int)legRec.width,
                     (int)legRec.height);

    BeginMode3D(legendCamera);

    const float legX = -55.0f;
    const float legY = 35.0f;
    const float legSp = 6.0f;

    const float legendScale = 31.0f;

    for (int i = 0; i < displayOrder.size(); ++i) {
      std::string name = displayOrder[i];

      Vector3 fakeSun = {legX, legY, 100.0f}; // Light from the front/camera
      SetShaderValue(lightShader, sunPosLoc, &fakeSun, SHADER_UNIFORM_VEC3);

      float s = std::max(proportion[name] * legendScale, 0.1f);
      if (name == "Sun")
        s *= (1.0 + pulse);

      // 1. Define the offset amount
      float sunDownOffset = 30.0f;

      // 2. Calculate position with the conditional offset
      float currentY = legY - (i * legSp);
      if (name == "Sun") {
        currentY -= sunDownOffset; // Move the Sun specifically
      }

      Vector3 lp = {legX, currentY, 0.0f};

      Color modelTint = hasTexture[name] ? WHITE : colorMap[name];
      float currentRot = rotAngle * rotSpeed[name];
      float tiltRad = axialTilt[name] * DEG2RAD;
      Vector3 tiltAxis = {sinf(tiltRad), cosf(tiltRad), 0.0f};

      DrawModelEx(planetModels[name], lp, tiltAxis, currentRot, {s, s, s},
                  modelTint);
      if (names[i] == "Sun")
        DrawSphere(lp, s * 1.025f, Fade(YELLOW, 0.3f));
    }
    EndMode3D();
    EndScissorMode();

    for (int i = 0; i < displayOrder.size(); ++i) {
      std::string name = displayOrder[i];

      int originalIdx = -1;
      for (int j = 0; j < names.size(); j++) {
        if (names[j] == name) {
          originalIdx = j;
          break;
        }
      }

      Vector2 p =
          GetWorldToScreen({legX, legY - (i * legSp), 0.0f}, legendCamera);

      Color textColor = (focusIndex == originalIdx) ? GOLD : WHITE;
      int fontSize = (focusIndex == originalIdx) ? 22 : 18;
      DrawText(name.c_str(), 56, (int)p.y - 9, fontSize, BLACK); // Shadow
      DrawText(name.c_str(), 55, (int)p.y - 10, fontSize, textColor);
    }

    EndTextureMode();
    // ----------------------------------------------------
    // 4. FINAL SCREEN DRAW
    // ----------------------------------------------------
    BeginDrawing();
    DrawTextureRec(
        target.texture,
        {0, 0, (float)target.texture.width, (float)-target.texture.height},
        {0, 0}, WHITE);

    // Stats Panel
    DrawRectangleRounded({(float)screenW - 220, 10, 210, 90}, 0.2f, 10,
                         Fade(BLACK, 0.6f));
    DrawText(TextFormat("Year: %0.2f", frameIndex * 0.01f), screenW - 205, 25,
             20, WHITE);
    if (focusIndex != -1)
      DrawText(TextFormat("Target: %s", names[focusIndex].c_str()),
               screenW - 205, 50, 18, GOLD);

    DrawFPS(10, 10);
    EndDrawing();
  }

  // Cleanup models and textures
  UnloadRenderTexture(target);
  for (auto &m : planetModels) {
    if (hasTexture[m.first]) {
      UnloadTexture(m.second.materials[0].maps[MATERIAL_MAP_ALBEDO].texture);
    }
    UnloadModel(m.second);
  }
  CloseWindow();
  return 0;
}
