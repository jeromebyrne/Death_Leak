// Backend-only diagnostic. SDL dummy mixing is not speaker/device acceptance.
#define DEATHLEAK_AUDIO_TESTING
#include "../linux/SDLSoundPlayer.cpp"
#include <cassert>

using namespace deathleak::audio;
static void Near(float actual,float expected) { assert(std::abs(actual-expected)<.00001f); }
static AmbientSource Fuse() {
    AmbientSource a;
    a.file="Game/Media/Audio/fuse_burning.wav";
    a.width=a.height=600.f;a.fadeWidth=a.fadeHeight=3000.f;
    return a; // API forces repeat/positional; caller need not set booleans.
}
int main() {
    SDLSoundPlayer player;
    std::string error;
    if(!player.Initialize(error)) {std::cerr<<error<<'\n';return 1;}
    player.PauseForTest();
    assert(player.LiveStreamsForTest()==0);
    assert(player.StartPositionalLoop(AmbientSource{})==0);
    auto missing=Fuse();missing.file="Game/Media/Audio/missing.wav";
    assert(player.StartPositionalLoop(missing)==0);
    const auto first=player.StartPositionalLoop(Fuse());
    const auto second=player.StartPositionalLoop(Fuse());
    assert(first && second && first!=second && player.PositionalVoicesForTest()==2);
    assert(player.LiveStreamsForTest()==2);
    assert(player.ActiveVoicesForTest()==0 && player.AmbientVoicesForTest()==0);
    Near(player.PositionalGainForTest(first),0.f); // Silent until positioned.
    player.UpdatePositionalLoop(first,0,0,0,0);Near(player.PositionalGainForTest(first),1.f);
    player.UpdatePositionalLoop(first,0,0,300,300);Near(player.PositionalGainForTest(first),1.f);
    player.UpdatePositionalLoop(first,0,0,900,0);Near(player.PositionalGainForTest(first),.5f);
    player.UpdatePositionalLoop(first,0,0,0,-900);Near(player.PositionalGainForTest(first),.5f);
    player.UpdatePositionalLoop(first,0,0,600,1200);Near(player.PositionalGainForTest(first),.25f);
    player.UpdatePositionalLoop(first,0,0,1500,0);Near(player.PositionalGainForTest(first),0.f);
    player.UpdatePositionalLoop(first,0,0,0,1501);Near(player.PositionalGainForTest(first),0.f);
    assert(!player.AdvanceForTest(16384)); // Both zero-gain voices continue silently.
    assert(player.PositionalReadForTest(first)>0);
    // Follow a moving owner and camera independently; no voice is recreated.
    const auto read=player.PositionalReadForTest(first);
    player.UpdatePositionalLoop(first,5000,2000,5000,2000);
    Near(player.PositionalGainForTest(first),1.f);
    assert(player.PositionalReadForTest(first)==read && player.PositionalVoicesForTest()==2);
    Near(player.PositionalGainForTest(second),0.f);
    player.StopPositionalLoop(second);player.StopPositionalLoop(second);player.StopPositionalLoop(0);
    assert(player.PositionalVoicesForTest()==1);
    // 400 x 0.085 seconds exceeds the authored 4.412-second fuse many times.
    bool signal=false;
    for(int i=0;i<400;++i) {
        player.UpdateObjectAmbience(5000,2000,0,0,1.f/60.f);
        signal=player.AdvanceForTest(16384)||signal;
    }
    assert(signal && player.PositionalRepeatsForTest(first)>=2);
    assert(player.PositionalVoicesForTest()==1);
    player.SetSfxEnabled(false);
    const auto mutedRead=player.PositionalReadForTest(first);
    assert(!player.AdvanceForTest(16384));
    assert(player.PositionalReadForTest(first)==mutedRead); // Existing stream mute freezes.
    assert(player.StartPositionalLoop(Fuse())==0); // Same muted-start contract as PlayFile.
    player.SetSfxEnabled(true);
    assert(player.PositionalVoicesForTest()==1 && player.AdvanceForTest(16384));
    player.StopPositionalLoop(first);
    player.UpdatePositionalLoop(first,0,0,0,0); // Stale owner cannot resurrect it.
    assert(player.PositionalVoicesForTest()==0 && !player.AdvanceForTest(16384));
    // Starting/updating/stopping/clearing owners must not reset authored ambience.
    auto ambient=Fuse();ambient.repeat=true;ambient.adjustToCamera=false;
    player.SetAmbience({ambient});
    const void* identity=player.AmbientIdentityForTest();
    assert(identity && player.AdvanceForTest(16384));
    const auto ambientRead=player.AmbientReadForTest();
    const auto third=player.StartPositionalLoop(Fuse());
    player.UpdatePositionalLoop(third,0,0,0,0);
    assert(player.AmbientIdentityForTest()==identity && player.AmbientReadForTest()==ambientRead);
    // Setting authored sources also leaves owner lifetime under explicit control.
    player.SetAmbience({ambient});assert(player.PositionalVoicesForTest()==1);
    identity=player.AmbientIdentityForTest();
    for(int i=0;i<10;++i)player.StartPositionalLoop(Fuse());
    player.StopPositionalLoop(third);
    player.ClearPositionalLoops();player.ClearPositionalLoops();
    assert(player.PositionalVoicesForTest()==0 && player.AmbientVoicesForTest()==1);
    assert(player.AmbientIdentityForTest()==identity);
    assert(player.StartPositionalLoop(Fuse()));
    player.SetSfxEnabled(false);
    const auto muted=player.StartPositionalLoop(Fuse());assert(!muted);
    player.ClearPositionalLoops(); // Scene reset also works while muted.
    assert(player.PositionalVoicesForTest()==0 && player.LiveStreamsForTest()==1);
    player.SetSfxEnabled(true);
    player.SetAmbience({});
    const auto afterReset=player.StartPositionalLoop(Fuse());
    assert(afterReset && afterReset!=third);
    player.UpdatePositionalLoop(third,0,0,0,0); // Old handle cannot affect replacement.
    Near(player.PositionalGainForTest(afterReset),0.f);
    player.ClearPositionalLoops();assert(!player.AdvanceForTest(16384));
    assert(player.LiveStreamsForTest()==0);
    std::cout<<"PASS: positional owner lifetime/follow/600-3000 attenuation/repeat/mute/independent ambience/cleanup\n";
}
