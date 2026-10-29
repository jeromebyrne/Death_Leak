#pragma once
#include <SDL.h>
#include <array>

namespace deathleak::linux_port {
// Remote desktops may send a press and release between fixed updates. Keep
// one update's press edge as well as held state; polling only SDL's final
// snapshot silently drops those pairs. Keysyms also cover remote keymaps with
// missing/different hardware scancodes. Focus loss must never leave movement held.
class KeyboardState {
public:
    void BeginUpdate() { pressed_.fill(false); }
    void Clear() { held_.fill(false); pressed_.fill(false); }
    void Handle(const SDL_KeyboardEvent& event) {
        const bool down = event.type == SDL_KEYDOWN;
        Set(event.keysym.scancode, down);
        Set(SDL_GetScancodeFromKey(event.keysym.sym), down);
    }
    bool Down(SDL_Scancode code) const {
        return code > SDL_SCANCODE_UNKNOWN && code < SDL_NUM_SCANCODES &&
               (held_[code] || pressed_[code]);
    }
private:
    void Set(SDL_Scancode code, bool down) {
        if (code <= SDL_SCANCODE_UNKNOWN || code >= SDL_NUM_SCANCODES) return;
        held_[code] = down;
        if (down) pressed_[code] = true;
    }
    std::array<bool, SDL_NUM_SCANCODES> held_{}, pressed_{};
};
}
