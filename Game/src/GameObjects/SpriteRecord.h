#ifndef SPRITERECORD_H
#define SPRITERECORD_H

#include "Engine/Render/IRenderer.h"
#include "GameObjects/LevelObjectRecord.h"

#include <string>

struct SpriteRecord
{
    std::string SourceLevel;
    std::string TypeName;
    int Id = 0;
    std::string TexturePath;
    bool TextureExists = false;
    TextureHandle Texture = 0;
    Vector2 WorldPosition;
    Vector2 Dimensions;
    float RotationRadians = 0.0f;
    float Alpha = 1.0f;
    int Depth = 0;
    SpriteFlip Flip = SpriteFlip::None;

    static SpriteRecord FromLevelObjectRecord(
        const LevelObjectRecord& object,
        const std::string& sourceLevel,
        const std::string& resolvedTexturePath);
};

#endif
