#!/bin/sh
# Desktop launcher: isolated, backed-up progress/settings; normal accelerated renderer.
set -eu
unset DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER
base=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$base"
if [ -z "${DISPLAY:-}" ] && [ -z "${WAYLAND_DISPLAY:-}" ]; then
    printf '%s\n' 'Run this launcher in a terminal inside your desktop session.' >&2
    exit 1
fi
state=${XDG_DATA_HOME:-"$HOME/.local/share"}/deathleak-playtest
umask 077
mkdir -p "$state"
save="$state/progression.dl"
settings="$state/settings.xml"
resume=false
if [ -s "$save" ]; then
    {
        IFS= read -r version
        IFS= read -r destination || destination=
    } < "$save"
    case "$version" in
        DL1|DL2|DL3|DL4|DL5) resume=true ;;
        DL6) if [ "$destination" != MENU ]; then resume=true; fi ;;
        *) printf 'Unrecognized save format; leaving save untouched: %s\n' "$save" >&2; exit 1 ;;
    esac
    cp -p "$save" "$save.backup-$(date +%Y%m%d-%H%M%S)-$$"
fi
if "$resume"; then
    exec "$base/port/build/deathleak-linux" --load-save "$save" --save-file "$save" --settings-file "$settings" "$@"
fi
exec "$base/port/build/deathleak-linux" --menu-loop --save-file "$save" --settings-file "$settings" "$@"
