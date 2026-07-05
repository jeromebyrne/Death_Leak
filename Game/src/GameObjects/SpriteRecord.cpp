#include "GameObjects/SpriteRecord.h"

SpriteRecord SpriteRecord::FromLevelObjectRecord(
    const LevelObjectRecord& object,
    const std::string& sourceLevel,
    const std::string& resolvedTexturePath)
{
    SpriteRecord record;
    record.SourceLevel = sourceLevel;
    record.TypeName = object.TypeName;
    record.Id = object.Id;
    record.TexturePath = resolvedTexturePath;
    record.TextureExists = !resolvedTexturePath.empty();
    record.WorldPosition = object.Position;
    record.Dimensions = object.Dimensions;
    record.RotationRadians = object.Rotation;
    record.Alpha = object.Alpha;
    record.Depth = object.DepthValue;
    record.RepeatTextureX = object.RepeatTextureX;
    record.RepeatTextureY = object.RepeatTextureY;
    record.DrawAtNativeDimensions = object.DrawAtNativeDimensions;

    if (object.HorizontalFlip && object.VerticalFlip)
    {
        record.Flip = SpriteFlip::Both;
    }
    else if (object.HorizontalFlip)
    {
        record.Flip = SpriteFlip::Horizontal;
    }
    else if (object.VerticalFlip)
    {
        record.Flip = SpriteFlip::Vertical;
    }

    return record;
}
