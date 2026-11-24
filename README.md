# BassCream

Professionell bass-thickener för **Pro Tools (AAX)**, **VST3** och standalone.

Två karaktärer:

| Läge | Känsla | Bra för |
|------|--------|---------|
| **Slap** | Transient-punch, strängklarhet, kontrollerad sub | Slap & pop, funky bas |
| **Rock** | Varm even-harmonic “kräm”, low-mid body, mjuk glue | Rock, indie, tjock studio-bas |

## Parametrar

- **Cream** — saturation / tjocklek  
- **Punch** — dynamik & transienter  
- **Tone** — mörkare ↔ ljusare  
- **Output** — nivå i dB  
- **Mix** — dry/wet  

Presets i program-menyn: *Slap Pop Ready*, *Rock Cream*, *Subtle Glue*.

## Kloning

```powershell
git clone --recurse-submodules https://github.com/h4nsln3r/BassCream.git
cd BassCream
```

## Bygga (Windows)

### Krav

- CMake 3.22+
- Visual Studio 2022 Build Tools (C++ workload)
- JUCE 8 (submodule under `libs/JUCE`)

```powershell
cd C:\Dev\2026\BassCream
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

VST3: `build/BassCream_artefacts/Release/VST3/BassCream.vst3`  
Standalone: `build/BassCream_artefacts/Release/Standalone/BassCream.exe`

### Pro Tools (AAX)

Modern Pro Tools laddar **bara AAX** (inte VST3). Osignerad AAX testas i **Pro Tools Developer**.

```powershell
$env:AAX_SDK_PATH = "C:\SDKs\AAX_SDK\aax-sdk-2-10-0"
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target BassCream_AAX
```

Installera till:

`C:\Program Files\Common Files\Avid\Audio\Plug-Ins\BassCream.aaxplugin`

Utan AAX SDK byggs **VST3 + Standalone**. Retail Pro Tools kräver PACE-signering för distribution.

## Användning i mix

1. Lägg BassCream **efter** DI/amp (eller på bus efter bas-plugin-kedjan).  
2. Börja med preset, justera **Cream** tills basen känns tjock — inte fuzzig.  
3. **Punch** uppåt för slap-transienter; lägre för limmad rock-bas.  
4. Matcha nivå med **Output**, eller kör **Mix** ~70 % för parallel “push”.

## Tech

- JUCE 8 + `juce_dsp`
- Noll avsiktlig latency
- Mono/stereo
- State save/recall via APVTS
