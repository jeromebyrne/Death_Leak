// Backend diagnostics only: dummy audio is not audible/release acceptance.
#define DEATHLEAK_AUDIO_TESTING
#include "../linux/SDLSoundPlayer.cpp"
#include "../../Game/tinyxml/tinyxml.h"
#include <cassert>

using namespace deathleak::audio;

int main() {
    SDLSoundPlayer player;
    std::string error;
    if (!player.Initialize(error)) { std::cerr << error << '\n'; return 1; }
    player.PauseForTest();
    // Original SfxPickup::Initialise and SaveShrine::Update assets.
    const std::vector<std::string> feedback = {
        "koto/koto.wav", "koto/koto_2.wav", "koto/koto_3.wav", "koto/koto_4.wav",
        "koto/koto_5.wav", "save_game.wav", "water/steam_hissing.wav"};
    for (const auto& name : feedback) {
        const size_t cached = player.CachedFilesForTest();
        player.PlayFile("Game/Media/Audio/" + name);
        assert(player.CachedFilesForTest() == cached + 1);
        assert(player.ActiveVoicesForTest() == 1);
        bool signal = false;
        for (int i = 0; i < 2048 && player.ActiveVoicesForTest(); ++i)
            signal = player.AdvanceForTest(16384) || signal;
        assert(signal && player.ActiveVoicesForTest() == 0);
        player.PlayFile("Game/Media/Audio/" + name);
        assert(player.CachedFilesForTest() == cached + 1); // Reuse converted PCM.
        player.SetSfxEnabled(false);
        assert(player.ActiveVoicesForTest() == 0);
        player.PlayFile("Game/Media/Audio/" + name);
        assert(player.ActiveVoicesForTest() == 0);
        player.SetSfxEnabled(true);
    }
    const size_t cached = player.CachedFilesForTest();
    for (const auto& bad : {"/tmp/test.wav", "koto/koto.wav", "Game/Media/Audio/../Audio/koto/koto.wav",
         "Game/Media/AudioX/koto/koto.wav", "Game/Media/Audio/koto/missing.wav",
         "Game/Media/Audio/koto/koto.ogg", "Game/Media/Audio/../../../../etc/passwd",
         "Game/Media/Audio/koto\\koto.wav"}) player.PlayFile(bad);
    assert(player.CachedFilesForTest() == cached && player.ActiveVoicesForTest() == 0);
    // Generic object startup delay follows gameplay time; source position can
    // follow a moving player without rebuilding/restarting ambient streams.
    AmbientSource delayed;delayed.file="Game/Media/Audio/koto/koto.wav";
    delayed.startDelay=.1f;delayed.followPlayer=true;
    player.SetAmbience({delayed});
    assert(!player.AdvanceForTest(16384));
    player.UpdateObjectAmbience(0,0,5000,0,.05f);assert(!player.AdvanceForTest(16384));
    player.UpdateObjectAmbience(0,0,5000,0,.051f);assert(player.AdvanceForTest(16384));
    player.SetAmbience({});
    // All enabled generic positional properties (not the dedicated AudioObjects).
    std::vector<AmbientSource> sources;
    for (const auto& entry : std::filesystem::directory_iterator("Game/XmlFiles/levels")) {
        if (entry.path().extension() != ".xml") continue;
        TiXmlDocument document(entry.path().c_str());
        assert(document.LoadFile());
        auto* level = document.RootElement();
        assert(level);
        for (auto* object = level->FirstChildElement(); object; object = object->NextSiblingElement()) {
            auto* props = object->FirstChildElement("pos_audio_props");
            if (!props || !props->Attribute("enabled") || std::string(props->Attribute("enabled")) != "true") continue;
            assert(props->Attribute("file"));
            std::string file = props->Attribute("file");
            std::replace(file.begin(), file.end(), '\\', '/');
            AmbientSource source;
            source.file = "Game/Media/Audio/" + file;
            source.adjustToCamera = true;
            source.repeat = true;
            sources.push_back(source);
        }
    }
    assert(sources.size() == 13);
    player.SetAmbience(sources);
    assert(player.AmbientVoicesForTest() == sources.size());
    player.UpdateAmbience(0, 0);
    assert(player.AdvanceForTest(16384));
    player.SetAmbience({});
    assert(player.AmbientVoicesForTest() == 0);
    std::cout << "PASS: seven authored interaction WAVs decode/mix/cache/mute; invalid paths ignored; all 13 enabled generic positional WAVs stream and clear\n";
}
