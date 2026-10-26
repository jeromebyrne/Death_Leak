#include "../linux/FreshPresentationPacket.h"
#include <cassert>
#include <iostream>
using namespace deathleak::linux_port;
static FullFrameSnapshot frame(float x) {
    FullFrameSnapshot f; f.sceneEpoch = 7;
    f.packet.Add(PresentationCommandPacket::Command{PresentationCommandPacket::Kind::Begin});
    PresentationCommandPacket::Command player{PresentationCommandPacket::Kind::Sprite};
    player.key={10,0}; player.sprite.texture=1; player.sprite.destination={x,20,20,30}; f.packet.Add(player);
    f.packet.Add(PresentationCommandPacket::Command{PresentationCommandPacket::Kind::Postprocess});
    PresentationCommandPacket::Command film{PresentationCommandPacket::Kind::Sprite};
    film.key={20,0}; film.sprite.texture=2; film.sprite.destination={0,0,1280,720}; f.packet.Add(film);
    PresentationCommandPacket::Command hud{PresentationCommandPacket::Kind::Sprite};
    hud.key={30,0}; hud.sprite.texture=3; hud.sprite.destination={50,50,100,20}; f.packet.Add(hud);
    f.packet.Seal(); return f;
}
int main() {
    FreshPresentationPacket p;
    p.Seal(frame(0),{0,20,0,0,7});
    p.Seal(frame(10),{10,20,0,0,7});
    const auto exact=p.Sample(1), half=p.Sample(.5), again=p.Sample(.5), start=p.Sample(0);
    assert(exact.packet.Size()==5 && exact.packet.Commands()[1].sprite.destination.x==10);
    assert(half.packet.Size()==5 && half.sampledCommands>0);
    assert(half.packet.Commands()[1].sprite.destination.x==5 && half.pose.playerX==5);
    assert(start.packet.Commands()[1].sprite.destination.x==0 && start.pose.playerX==0);
    assert(again.packet.Commands()[1].sprite.destination.x==5 && again.pose.playerX==5);
    assert(half.packet.Commands()[2].kind==PresentationCommandPacket::Kind::Postprocess);
    assert(half.packet.Commands()[3].sprite.texture==2 && half.packet.Commands()[4].sprite.texture==3);
    auto cut=frame(20); cut.sceneEpoch=8; p.Seal(std::move(cut),{20,20,0,0,8});
    assert(p.Sample(.5).packet.Commands()[1].sprite.destination.x==20);
    assert(p.Sample(0).packet.Commands()[1].sprite.destination.x==20);
    std::cout << "fresh full post-HUD packet sampler ok\n";
}
