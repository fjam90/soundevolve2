#!/bin/bash
# Crea el instalador .pkg de macOS (VST3 + AU + app Standalone)
set -euo pipefail

NAME="Aranguiz Soundevolve"
VERSION="0.1.0"
ART="${ARTEFACTS:-build/MasterPreset_artefacts/Release}"
OUT="${OUT_DIR:-installer-out}"
STAGE="$(mktemp -d)"

mkdir -p "$STAGE/Library/Audio/Plug-Ins/VST3" \
         "$STAGE/Library/Audio/Plug-Ins/Components" \
         "$STAGE/Applications" "$OUT"

cp -R "$ART/VST3/$NAME.vst3"         "$STAGE/Library/Audio/Plug-Ins/VST3/"
cp -R "$ART/AU/$NAME.component"      "$STAGE/Library/Audio/Plug-Ins/Components/"
cp -R "$ART/Standalone/$NAME.app"    "$STAGE/Applications/"

PKG="$OUT/Aranguiz-Soundevolve-$VERSION-macOS.pkg"
pkgbuild --root "$STAGE" \
         --identifier com.aranguiz.soundevolve \
         --version "$VERSION" \
         --install-location / \
         --ownership recommended \
         "$PKG"

# Firma opcional del instalador (requiere certificado "Developer ID Installer")
if [ -n "${INSTALLER_SIGN_ID:-}" ]; then
  productsign --sign "$INSTALLER_SIGN_ID" "$PKG" "$PKG.signed"
  mv "$PKG.signed" "$PKG"
fi

echo "Instalador creado: $PKG"
