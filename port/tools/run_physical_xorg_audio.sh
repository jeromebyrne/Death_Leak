#!/bin/sh
# Physical Xorg/ALSA smoke: verifies real device open and gameplay cue dispatch.
# Does not prove that a human heard correct sound from the connected speakers.
set -eu
root=$(pwd -P)
test -x "$root/port/build/deathleak-linux" || { echo 'build port/build/deathleak-linux first' >&2; exit 2; }
base="$root/port/build/physical-xorg-audio"
mkdir -p "$base"
container="deathleak-xorg-audio-$$"
trap 'docker stop "$container" >/dev/null 2>&1 || true' EXIT HUP INT TERM
timeout --signal=TERM --kill-after=10s 45s docker run --rm --name "$container" --network host \
  --device /dev/dri --device /dev/snd \
  --mount "type=bind,src=$root,dst=/src" \
  --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix \
  --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly \
  -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority \
  -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl \
  -e SDL_AUDIODRIVER=alsa -e AUDIODEV=hw:1,0 \
  -e SDL_AUDIO_ALSA_DEFAULT_PLAYBACK_DEVICE=hw:1,0 \
  -w /src deathleak-linux-build:bookworm port/build/deathleak-linux \
  --play Game/XmlFiles/levels/forest_exploration_1.xml --script B:120 \
  --frames 120 --fixed-dt 0.0166667 >"$base/run.log" 2>&1 || { tail -n 20 "$base/run.log" >&2; exit 1; }
grep -q 'video_driver=x11 name=opengl .*accelerated=1 software=0 .*mode=strict-accelerated' "$base/run.log" || { echo 'physical accelerated Xorg renderer absent' >&2; exit 1; }
grep -q 'OpenGL device vendor=nouveau renderer=NVE7' "$base/run.log" || { echo 'physical NVE7 GPU absent' >&2; exit 1; }
if grep -q 'Audio unavailable:' "$base/run.log"; then tail -n 20 "$base/run.log" >&2; exit 1; fi
grep -q 'Audio ambient streamed WAV voices=2 authored=2' "$base/run.log" || { echo 'real ALSA ambience playback path absent' >&2; exit 1; }
grep -q 'Simulation ended at Game/XmlFiles/levels/forest_exploration_1.xml .*fired=8 ' "$base/run.log" || { echo 'active gameplay/cue coverage absent' >&2; exit 1; }
printf 'Physical Xorg/ALSA smoke passed: NVE7 rendering, ALSA hw:1,0 open, ambience streaming and knife cues dispatched. Speaker audibility remains unverified.\n'
