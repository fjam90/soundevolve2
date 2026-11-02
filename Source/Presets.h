#pragma once
#include <array>

namespace mp
{
inline constexpr int numEq = 10;
inline constexpr int numParams = 21;

// Índices de parámetros: 0..9 = bandas del EQ gráfico
inline constexpr int pThresh = 10, pRatio = 11, pAttack = 12, pRelease = 13, pMakeup = 14,
                     pDrive = 15, pWidth = 16, pLimGain = 17, pCeiling = 18,
                     pInput = 19, pOutput = 20;

inline constexpr int catMix = 0, catInstrument = 1;

inline const std::array<float, numEq> eqFreqs {
    31.25f, 62.5f, 125.f, 250.f, 500.f, 1000.f, 2000.f, 4000.f, 8000.f, 16000.f };

inline const std::array<const char*, numParams> paramIds {
    "eq31", "eq62", "eq125", "eq250", "eq500", "eq1k", "eq2k", "eq4k", "eq8k", "eq16k",
    "thresh", "ratio", "attack", "release", "makeup", "drive", "width", "limGain", "ceiling",
    "input", "output" };

inline const std::array<const char*, numParams> paramNames {
    "31", "62", "125", "250", "500", "1k", "2k", "4k", "8k", "16k",
    "Comp Thresh", "Comp Ratio", "Attack", "Release", "Makeup", "Drive", "Width %",
    "Limiter In", "Ceiling", "Input", "Output" };

// min, max, step
inline const std::array<std::array<float, 3>, numParams> ranges {{
    { -12.f, 12.f, 0.1f }, { -12.f, 12.f, 0.1f }, { -12.f, 12.f, 0.1f }, { -12.f, 12.f, 0.1f },
    { -12.f, 12.f, 0.1f }, { -12.f, 12.f, 0.1f }, { -12.f, 12.f, 0.1f }, { -12.f, 12.f, 0.1f },
    { -12.f, 12.f, 0.1f }, { -12.f, 12.f, 0.1f },
    { -40.f, 0.f, 0.1f },  { 1.f, 10.f, 0.1f },  { 1.f, 100.f, 0.1f }, { 10.f, 500.f, 1.f },
    { 0.f, 12.f, 0.1f },   { 0.f, 12.f, 0.1f },  { 0.f, 200.f, 1.f },  { 0.f, 15.f, 0.1f },
    { -6.f, 0.f, 0.1f },   { -12.f, 12.f, 0.1f }, { -24.f, 0.f, 0.1f } }};

struct Preset { const char* name; int category; std::array<float, numParams> v; };

// CALIBRACIÓN: los presets asumen una mezcla con picos en -6 dBFS y ~ -18 LUFS
// integrados (PLR ~ 12 dB). Los objetivos de LUFS son aproximados (+-1.5 LU);
// termina el ajuste con las perillas Input / Output / Limiter In.
//
// Orden: EQ 31, 62, 125, 250, 500, 1k, 2k, 4k, 8k, 16k |
//        thresh, ratio, attack, release, makeup | drive, width | limGain, ceiling | input, output
inline const std::array<Preset, 20> presets {{
    // ---------------- MEZCLA / MASTER ----------------
    { "Init (Flat)",              catMix, { 0,0,0,0,0,0,0,0,0,0,                              0,1,30,150,0,        0,100,   0,-0.1f,    0,0 } },
    { "Streaming (-14 LUFS)",     catMix, { 0.5f,0.5f,0,-0.5f,-0.5f,0,0.5f,0.5f,1,1,          -14,2,30,150,1.5f,   2,105,   5,-0.1f,    0,0 } },
    { "Apple Music (-16 LUFS)",   catMix, { 0.5f,0.5f,0,-0.5f,-0.5f,0,0.5f,0.5f,1,1,          -15,1.8f,30,150,1,  2,105,   3.5f,-0.1f,  0,0 } },
    { "Broadcast EBU R128 (-23)", catMix, { 0,0,0,0,0,0,0,0.5f,0.5f,0,                        -16,1.8f,40,200,0.5f, 1,100,  0,-0.1f,    0,-4.5f } },
    { "Club / EDM (-8 LUFS)",     catMix, { 1.5f,1.5f,0.5f,-0.5f,-1,0,0.5f,1,1.5f,1.5f,       -14,3,20,100,2.5f,  4,115,   10,-0.1f,   0,0 } },
    { "House / Tech House (-9)",  catMix, { 1,1.5f,1,-0.5f,-1,-0.5f,0.5f,1,1.5f,1,            -14,2.5f,25,120,2,  3,110,   9,-0.1f,    0,0 } },
    { "Techno / Dark (-9)",       catMix, { 1.5f,1.5f,1,0,-1,-1,0,0.5f,0,-1,                  -14,2.8f,20,110,2,  4,105,   9,-0.1f,    0,0 } },
    { "Hip-Hop / Trap (-11)",     catMix, { 2,2,1,-0.5f,-1,-0.5f,0.5f,1,1,0.5f,               -14,2.5f,30,120,1.5f, 3,100, 8,-0.1f,    0,0 } },
    { "Rock / Pop (-11)",         catMix, { 0.5f,1,0.5f,-1,-0.5f,0.5f,1,1.5f,1.5f,1,          -15,2.5f,25,120,1.5f, 3,110, 8,-0.1f,    0,0 } },
    { "Acoustic / Folk (-14)",    catMix, { 0,0,0.5f,0,0,0,0.5f,1,1,1,                        -16,1.6f,40,200,0.5f, 1,105, 6,-0.1f,    0,0 } },
    { "Classical / Jazz (-18)",   catMix, { 0,0,0,0,0,0,0,0.5f,0.5f,0.5f,                     -22,1.3f,50,250,0,  0,100,   1,-0.1f,    0,0 } },
    // ---------------- INSTRUMENTOS (electrónica) ----------------
    { "Sub Bass (Mono)",          catInstrument, { 2,2,0,-4,-9,-12,-12,-12,-12,-12,           -14,3,10,120,1,     2,0,     2,-0.1f,    0,0 } },
    { "Bass (Reese / Mid)",       catInstrument, { -6,0,2,2,1,1,0,-1,-3,-6,                   -14,3.5f,15,100,2,  5,80,    3,-0.1f,    0,0 } },
    { "Kick (Punchy)",            catInstrument, { 0,3,2,-3,-2,1,3,2,0,-2,                    -12,4,30,60,1.5f,   3,0,     3,-0.1f,    0,0 } },
    { "Kick (Sub-Heavy)",         catInstrument, { 3,4,1,-3,-4,-1,1,1,0,-3,                   -12,4,25,80,1,      2,0,     3,-0.1f,    0,0 } },
    { "Drum Bus (Glue)",          catInstrument, { 0,1,0,-1,0,0,1,1,1,1,                      -14,3,30,80,2,      3,110,   2,-0.1f,    0,0 } },
    { "Snare / Clap",             catInstrument, { -12,-9,-3,1,2,1,2,3,2,1,                   -12,3.5f,20,80,1.5f, 2,105,  2,-0.1f,    0,0 } },
    { "Hats & Percussion",        catInstrument, { -12,-12,-9,-6,-3,0,1,2,3,3,                -14,2,5,60,1,       2,130,   1,-0.1f,    0,0 } },
    { "Lead (Supersaw / Wide)",   catInstrument, { -12,-9,-5,-2,0,1,2,2,2,1,                  -14,2.5f,15,120,1.5f, 3,140, 2,-0.1f,    0,0 } },
    { "Lead (Pluck / Focus)",     catInstrument, { -12,-12,-6,-1,1,2,2,1,0,-1,                -13,3,10,80,1.5f,   2,90,    2,-0.1f,    0,0 } },
}};
} // namespace mp
