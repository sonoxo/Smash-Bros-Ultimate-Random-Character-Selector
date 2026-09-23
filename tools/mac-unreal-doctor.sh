#!/bin/sh
# Sonoxo Unreal/macOS toolchain doctor
# Checks and repairs the Xcode + Metal prerequisites that Unreal Engine needs on macOS.
# Safe by default: use --repair to install/repair Apple components.

set -u

PROJECT_DEFAULT="/Volumes/Elements/Projects/Smash-Bros-Ultimate-Random-Character-Selector/SmashRandomiser.uproject"
PROJECT="$PROJECT_DEFAULT"
REPAIR=0
LAUNCH=0

usage() {
  cat <<'EOF'
Usage:
  ./tools/mac-unreal-doctor.sh [--project /path/Game.uproject] [--repair] [--launch]

Options:
  --project PATH  Unreal .uproject to validate.
  --repair        Repair Xcode selection/first-launch state and install MetalToolchain if needed.
  --launch        Launch the project after all checks pass.
  -h, --help      Show this help.

Examples:
  ./tools/mac-unreal-doctor.sh
  ./tools/mac-unreal-doctor.sh --repair
  ./tools/mac-unreal-doctor.sh --repair --launch
EOF
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --project)
      shift
      [ "$#" -gt 0 ] || { echo "ERROR: --project requires a path"; exit 2; }
      PROJECT="$1"
      ;;
    --repair) REPAIR=1 ;;
    --launch) LAUNCH=1 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "ERROR: unknown option: $1"; usage; exit 2 ;;
  esac
  shift
done

ok()   { printf 'OK   %s\n' "$*"; }
warn() { printf 'WARN %s\n' "$*"; }
fail() { printf 'FAIL %s\n' "$*"; }

echo "SONOXO // MAC UNREAL DOCTOR"
echo "=========================="

if [ "$(uname -s)" != "Darwin" ]; then
  fail "This doctor is for macOS."
  exit 1
fi

echo "macOS: $(sw_vers -productVersion 2>/dev/null || echo unknown)"
echo "Arch : $(uname -m)"

if [ ! -d "/Applications/Xcode.app" ]; then
  fail "Xcode.app was not found in /Applications."
  echo "Install a version of Xcode compatible with your Unreal Engine release."
  exit 1
fi

EXPECTED_DEV="/Applications/Xcode.app/Contents/Developer"
ACTIVE_DEV="$(xcode-select -p 2>/dev/null || true)"

if [ "$ACTIVE_DEV" != "$EXPECTED_DEV" ]; then
  warn "Active developer directory is: ${ACTIVE_DEV:-none}"
  warn "Expected: $EXPECTED_DEV"
  if [ "$REPAIR" -eq 1 ]; then
    echo "Repairing xcode-select..."
    sudo xcode-select --switch "$EXPECTED_DEV" || exit 1
    ACTIVE_DEV="$(xcode-select -p 2>/dev/null || true)"
  else
    echo "Run again with --repair, or:"
    echo "  sudo xcode-select --switch \"$EXPECTED_DEV\""
    exit 1
  fi
fi
ok "Xcode developer directory selected."

XCODE_VERSION="$(xcodebuild -version 2>/dev/null | sed -n '1p' || true)"
XCODE_BUILD="$(xcodebuild -version 2>/dev/null | sed -n '2p' || true)"
echo "Xcode: ${XCODE_VERSION:-unknown} ${XCODE_BUILD:-}"

XCODE_NUM="$(printf '%s' "$XCODE_VERSION" | awk '{print $2}')"
case "$XCODE_NUM" in
  26.4*)
    fail "Epic documents Xcode 26.4 as incompatible with Unreal Engine 5.8 on macOS."
    echo "Use a documented UE-compatible Xcode version (Epic recommends 26.1.1 for UE 5.8)."
    exit 1
    ;;
  27.*|28.*|29.*)
    warn "This Xcode is newer than Epic's documented UE 5.8 recommendation (26.1.1)."
    warn "Continue only if Metal and Unreal checks below pass; otherwise select a supported Xcode."
    ;;
esac

if [ "$REPAIR" -eq 1 ]; then
  echo "Completing Xcode first-launch tasks..."
  sudo xcodebuild -license accept >/dev/null 2>&1 || true
  sudo xcodebuild -runFirstLaunch || exit 1
fi

TMPROOT="$(mktemp -d "${TMPDIR:-/tmp}/sonoxo-metal.XXXXXX")" || exit 1
trap 'rm -rf "$TMPROOT"' EXIT HUP INT TERM

cat > "$TMPROOT/doctor.metal" <<'METAL'
#include <metal_stdlib>
using namespace metal;
kernel void sonoxo_doctor(device float *out [[buffer(0)]],
                          uint id [[thread_position_in_grid]])
{
    out[id] = 1.0f;
}
METAL

metal_test() {
  xcrun -sdk macosx metal -o "$TMPROOT/doctor.air" -c "$TMPROOT/doctor.metal"     >"$TMPROOT/metal.stdout" 2>"$TMPROOT/metal.stderr"
}

echo "Testing the Metal compiler with a real shader compile..."
if metal_test; then
  ok "Metal compiler works."
else
  fail "Metal compiler test failed."
  sed -n '1,20p' "$TMPROOT/metal.stderr"

  if [ "$REPAIR" -ne 1 ]; then
    echo
    echo "Run:"
    echo "  ./tools/mac-unreal-doctor.sh --repair"
    echo
    echo "The repair path installs Apple's optional Metal Toolchain."
    exit 1
  fi

  echo
  echo "Installing Apple's Metal Toolchain..."
  xcodebuild -downloadComponent metalToolchain || exit 1

  echo "Checking for additional Xcode components..."
  xcodebuild -runFirstLaunch -checkForNewerComponents || true

  echo "Re-testing Metal..."
  if metal_test; then
    ok "Metal compiler works after repair."
  else
    fail "Metal still cannot compile a test shader."
    sed -n '1,40p' "$TMPROOT/metal.stderr"
    echo
    echo "Open Xcode > Settings > Components and verify Metal Toolchain is installed."
    exit 1
  fi
fi

CLANG="$(xcrun --find clang 2>/dev/null || true)"
[ -n "$CLANG" ] && ok "clang: $CLANG" || warn "clang was not found through xcrun."

if [ -f "$PROJECT" ]; then
  ok "Project found: $PROJECT"
  ENGINE_ASSOC="$(grep -Eo '"EngineAssociation"[[:space:]]*:[[:space:]]*"[^"]+"' "$PROJECT" 2>/dev/null | sed -E 's/.*"([^"]+)"$/\1/' || true)"
  [ -n "$ENGINE_ASSOC" ] && echo "Project EngineAssociation: $ENGINE_ASSOC"

  if [ "$ENGINE_ASSOC" = "5.0" ]; then
    warn "This project identifies UE 5.0 but may be opened with a newer UE release."
    warn "Keep Git history/backups and let Unreal perform any project conversion deliberately."
  fi
else
  warn "Project not found at: $PROJECT"
  warn "Use --project /path/to/Game.uproject if the project moved."
fi

echo "Locating UnrealEditor..."
UE_BIN="$(find "/Users/Shared/Epic Games" "/Applications" "/Volumes/Elements"   -path "*/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"   -type f -print 2>/dev/null | sort -V | tail -1)"

if [ -n "$UE_BIN" ] && [ -x "$UE_BIN" ]; then
  ok "UnrealEditor: $UE_BIN"
else
  fail "UnrealEditor executable not found."
  echo "Install Unreal Engine with Epic Games Launcher or pass the project to a known editor manually."
  exit 1
fi

echo
echo "PRE-FLIGHT RESULT: PASS"

if [ "$LAUNCH" -eq 1 ]; then
  if [ ! -f "$PROJECT" ]; then
    fail "Cannot launch: project file is missing."
    exit 1
  fi
  LOG="$HOME/Desktop/sonoxo-fighter-unreal.log"
  echo "Launching Unreal..."
  echo "Log: $LOG"
  "$UE_BIN" "$PROJECT" -log 2>&1 | tee "$LOG"
else
  echo "Launch with:"
  echo "  ./tools/mac-unreal-doctor.sh --launch"
fi
