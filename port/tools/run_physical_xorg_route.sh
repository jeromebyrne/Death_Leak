#!/bin/sh
# Physical-display GPU route gate. Run from the repository root on the Docker host.
# This is scripted/dummy-audio evidence, never controller, speaker or human acceptance.
set -eu
mode=${1:-prefix}
case "$mode" in
  probe) frames=2; limit=60s ;;
  prefix) frames=13000; limit=360s ;;
  full) frames=60000; limit=1800s ;;
  *) echo 'usage: port/tools/run_physical_xorg_route.sh [probe|prefix|full]' >&2; exit 2 ;;
esac
root=$(pwd -P)
test -x "$root/port/build/deathleak-linux" || { echo 'build port/build/deathleak-linux first' >&2; exit 2; }
test -f "$root/port/tests/routes/earned_full_ending.script" || exit 2
mkdir -p "$root/port/build/physical-xorg-route"
log="$root/port/build/physical-xorg-route/$mode.log"
container="deathleak-xorg-route-$$"
trap "docker stop $container >/dev/null 2>&1 || true" EXIT HUP INT TERM
# LightDM's authority record is host-owned; --hostname matches its local X cookie.
# Explicit x11 is critical: SDL otherwise silently selects offscreen OpenGL.
if ! timeout --signal=TERM --kill-after=10s "$limit" docker run --rm --name "$container" --network host --hostname jerome-Linux-MacBookPro \
  --device /dev/dri \
  --mount "type=bind,src=$root,dst=/src" \
  --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix \
  --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly \
  -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority \
  -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl -e SDL_AUDIODRIVER=dummy \
  -w /src deathleak-linux-build:bookworm \
  sh -c 'port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$(cat port/tests/routes/earned_full_ending.script)" --frames "$1" --fixed-dt 0.0166667 --profile' sh "$frames" >"$log" 2>&1; then
  tail -n 20 "$log" >&2
  echo "Physical Xorg run failed; inspect $log" >&2
  exit 1
fi
if ! grep -q 'video_driver=x11 name=opengl .*accelerated=1 software=0 .*mode=strict-accelerated' "$log" ||
   ! grep -q 'OpenGL device vendor=nouveau renderer=NVE7' "$log"; then
  tail -n 20 "$log" >&2
  echo "Physical Xorg/NVE7 gate failed; inspect $log" >&2
  exit 1
fi
if grep -q 'Player died at frame' "$log"; then
  grep 'Player died at frame' "$log" >&2
  echo "Earned route died; inspect $log" >&2
  exit 1
fi
# A renderer probe alone is not a route gate: require real scene progression,
# the complete frame budget and a post-warmup profile before accepting a run.
if [ "$mode" != probe ]; then
  if [ "$(grep -c '^Entered Game/XmlFiles/levels/village_central.xml$' "$log")" -lt 2 ] ||
     ! grep -q '^Entered Game/XmlFiles/levels/village_agri.xml$' "$log" ||
     ! grep -q "^Profile frame_ms warmup=120 samples=$((frames - 120)) " "$log" ||
     ! grep -q '^Simulation ended at ' "$log"; then
    tail -n 20 "$log" >&2
    echo "Earned route progress/profile incomplete; inspect $log" >&2
    exit 1
  fi
fi
if [ "$mode" = prefix ] && ! grep -q '^Simulation ended at Game/XmlFiles/levels/village_central.xml .*health=[1-9]' "$log"; then
  tail -n 20 "$log" >&2
  echo "Physical prefix did not survive the central return; inspect $log" >&2
  exit 1
fi
if [ "$mode" = full ] && { ! grep -q '^EndStory at frame ' "$log" ||
   ! grep -q '^Simulation ended at .*orbs=[0-9][0-9][0-9] keys=4 health=[1-9]' "$log"; }; then
  tail -n 20 "$log" >&2
  echo "Earned full-route ending/progression missing; inspect $log" >&2
  exit 1
fi
grep -E 'SDL renderer selected|OpenGL device|EndStory at frame|Simulation ended at|Frame wall' "$log" | tail -n 12
printf 'Physical Xorg %s passed scripted GPU gate; log: %s\n' "$mode" "$log"
