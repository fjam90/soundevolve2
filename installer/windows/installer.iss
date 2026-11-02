; Instalador de Windows (Inno Setup 6) - Aranguiz Soundevolve
#define AppName "Aranguiz Soundevolve"
#define AppVersion "0.1.0"
#ifndef Artefacts
  #define Artefacts "..\..\build\MasterPreset_artefacts\Release"
#endif
#ifndef OutDir
  #define OutDir "..\..\installer-out"
#endif

[Setup]
AppId={{B6B3B9E2-5C1D-4F7B-9E52-1A7D6C0A4A11}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=Aranguiz
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
OutputDir={#OutDir}
OutputBaseFilename=Aranguiz-Soundevolve-Setup-{#AppVersion}-Windows
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
UninstallDisplayName={#AppName}

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full"; Description: "Instalación completa"
Name: "custom"; Description: "Personalizada"; Flags: iscustom

[Components]
Name: "vst3"; Description: "Plugin VST3 (carpeta de plugins del sistema)"; Types: full custom; Flags: fixed
Name: "standalone"; Description: "Aplicación independiente (Standalone)"; Types: full

[Files]
; El VST3 se instala en C:\Program Files\Common Files\VST3 (lo detectan todos los DAW)
Source: "{#Artefacts}\VST3\{#AppName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#AppName}.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#Artefacts}\Standalone\{#AppName}.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppName}.exe"; Components: standalone
