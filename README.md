# Aranguiz Soundevolve (VST3 / AU / Standalone)

Cadena: Input -> EQ gráfico de 10 bandas (31 Hz a 16 kHz, +-12 dB) -> Compresor + makeup -> Saturación -> Ancho M/S -> Ganancia + Limitador (ceiling -0.1 dB por defecto) -> Output (-24 a 0 dB, solo atenúa para no pasar el ceiling) -> Medidor LUFS.

## Presets (menú con dos submenús)
- **Mix / Master:** Init, Streaming (-14), Apple Music (-16), Broadcast EBU R128 (-23), Club/EDM, House/Tech House, Techno, Hip-Hop/Trap, Rock/Pop, Acoustic, Classical/Jazz.
- **Instruments:** Sub Bass (mono), Bass (Reese/Mid), Kick (Punchy), Kick (Sub-Heavy), Drum Bus, Snare/Clap, Hats & Percussion, Lead (Supersaw/Wide), Lead (Pluck/Focus).

### Calibración
Los presets asumen una mezcla con **picos en -6 dBFS** (~ -18 LUFS integrados). Los
objetivos de LUFS son aproximados (+-1.5 LU): termina el ajuste con Input, Output y Limiter In.
Si tu mezcla llega más alta o más baja, compénsalo con la perilla Input.
Los presets de instrumentos usan el EQ gráfico como filtro suave (no es un pasa-altos real).

## Compilar en macOS
Requisitos: Xcode (o Command Line Tools: `xcode-select --install`), CMake 3.22+ (`brew install cmake`) y Git.

    cmake -B build -G Xcode
    cmake --build build --config Release

Genera un binario universal (Apple Silicon + Intel) y copia automáticamente:
- VST3 -> ~/Library/Audio/Plug-Ins/VST3
- AU   -> ~/Library/Audio/Plug-Ins/Components
- También crea la app Standalone en build/MasterPreset_artefacts/Release/Standalone

Si tu DAW no ve el AU, reinícialo; en Logic puede hacer falta validar:
`auval -v aufx Asev Aran`

### Distribuir a otros usuarios
Para que otros Mac abran el plugin sin avisos de Gatekeeper necesitas firmarlo
(Apple Developer ID) y notarizarlo. Solo para pruebas locales:
`xattr -cr ~/Library/Audio/Plug-Ins/VST3/"Aranguiz Soundevolve.vst3"`

## Compilar en Windows / Linux
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release
(En Windows y Linux el formato AU no se genera; CMake lo ignora.)

## Uso
Elige un preset, reproduce tu mezcla y sube/baja "Limiter In" hasta que el
medidor Integrated marque el LUFS objetivo del preset.

## Diseño
La interfaz usa Assets/background.png (1800x1200, 900x600 @2x), generado con
Assets/make_background.py a partir de tu logo (LOGO2024.PNG). Si cambias la
posición de los paneles, actualiza PANELS en el script y `panels` en
Source/PluginEditor.cpp. Assets/preview.png muestra cómo se verá con controles.

## Compilar en la nube (sin instalar nada)
Sube la carpeta a un repositorio de GitHub. El workflow `.github/workflows/build.yml`
compila Windows y macOS automáticamente; descarga los .vst3 (y .component en Mac)
desde la pestaña Actions > el último run > Artifacts.

## Instaladores (Windows .exe y macOS .pkg)
El workflow de GitHub Actions genera también los instaladores:
- Windows: `Aranguiz-Soundevolve-Setup-0.1.0-Windows.exe` (Inno Setup). Instala el VST3 en
  `C:\Program Files\Common Files\VST3` y, opcionalmente, la app Standalone. Incluye desinstalador.
- macOS: `Aranguiz-Soundevolve-0.1.0-macOS.pkg`. Instala VST3, AU y la app Standalone.
Descárgalos en Actions > último run > Artifacts > `Installer-windows-latest` / `Installer-macos-latest`.
Scripts: `installer/windows/installer.iss` e `installer/mac/build_pkg.sh`.
Para cambiar la versión edita `AppVersion` (.iss) y `VERSION` (.sh).

Los instaladores salen SIN firmar: Windows mostrará SmartScreen ("Más información > Ejecutar
de todos modos") y macOS pedirá abrir con clic derecho > Abrir. Para quitar esos avisos hace
falta un certificado de firma de código (Windows) y Apple Developer ID + notarización (Mac).
