#include "../audio/SoundPlayer.h"
#include <SDL.h>
#include <iostream>
#include <string>
int main(int argc, char** argv) {
    auto sound = deathleak::audio::CreateSoundPlayer(); std::string error;
    if (!sound->Initialize(error)) { std::cerr << error << '\n'; return 1; }
    if (argc > 1 && std::string(argv[1]) == "mute") sound->SetSfxEnabled(false);
    if (argc > 1 && std::string(argv[1]) != "idle")
        deathleak::audio::DispatchJumpRollVocal(*sound, 0);
    SDL_Delay(300);
}
