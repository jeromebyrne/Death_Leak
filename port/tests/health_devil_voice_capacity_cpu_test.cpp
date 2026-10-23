// Build standalone against the production SDL backend; test seam only pauses
// the dummy device and advances its real mixer callback on this thread.
#define DEATHLEAK_AUDIO_TESTING
#include "../linux/SDLSoundPlayer.cpp"
#include <cassert>
#include <iostream>

using namespace deathleak::audio;

static void finishVoice(SDLSoundPlayer& player, unsigned id) {
    assert(id != 0);
    assert(!player.VoiceFinished(id));
    for (int i = 0; i < 1024 && !player.VoiceFinished(id); ++i)
        player.AdvanceForTest(4096);
    assert(player.VoiceFinished(id));
}

// The live-dialog branch in main.cpp gates GiveReward on this exact audio
// predicate, followed by the HealthDevil update/manager-zone checks. Track a
// reward state so this catches an early spawn, not only an early predicate.
struct RewardGate {
    bool spawned = false;
    void Update(SDLSoundPlayer& player, unsigned voice, bool updateable, bool inManagerZone) {
        if (!spawned && player.VoiceFinished(voice) && updateable && inManagerZone)
            spawned = true;
    }
};

int main() {
    std::string error;
    SDLSoundPlayer player;
    if (!player.Initialize(error)) {
        std::cerr << "SDL audio initialization failed: " << error << '\n';
        return 1;
    }
    player.PauseForTest();

    // Unsaturated control: a live dialog must hold the reward boundary.
    const unsigned normal = player.PlayDevilVoice(false);
    assert(normal != 0 && !player.VoiceFinished(normal));
    finishVoice(player, normal);
    std::cout << "unsaturated=pass\n";

    // Saturate with 16 ordinary one-shots while the callback is paused.
    for (int i = 0; i < 16; ++i) player.Play(Cue::FinalTitle, 0);
    assert(player.ActiveVoicesForTest() == 16);
    const unsigned saturated = player.PlayDevilVoice(false);
    assert(saturated != 0);
    assert(player.ActiveVoicesForTest() == 16); // Oldest ordinary voice displaced.
    assert(!player.VoiceFinished(saturated));
    RewardGate reward;
    reward.Update(player, saturated, true, true);
    assert(!reward.spawned);
    // Subsequent ordinary SFX bursts must not evict the tracked dialog.
    for (int i = 0; i < 32; ++i) player.Play(Cue::FinalTitle, 0);
    assert(player.ActiveVoicesForTest() == 16);
    reward.Update(player, saturated, true, true);
    assert(!reward.spawned);
    player.AdvanceForTest(4096);
    assert(!player.VoiceFinished(saturated));
    reward.Update(player, saturated, true, true);
    assert(!reward.spawned);
    finishVoice(player, saturated);
    reward.Update(player, saturated, true, true);
    assert(reward.spawned);
    std::cout << "saturated_and_reward_boundary=pass\n";

    // Muting SFX intentionally completes dialog immediately, as in original.
    const unsigned mutedVoice = player.PlayDevilVoice(false);
    assert(mutedVoice != 0 && !player.VoiceFinished(mutedVoice));
    player.SetSfxEnabled(false);
    assert(player.VoiceFinished(mutedVoice));
    assert(player.PlayDevilVoice(false) == 0);
    assert(player.VoiceFinished(0));
    std::cout << "sfx_muted=pass\n";
}
