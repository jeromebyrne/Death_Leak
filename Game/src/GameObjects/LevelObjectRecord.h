#ifndef LEVELOBJECTRECORD_H
#define LEVELOBJECTRECORD_H

#include "Vector2.h"
#include "tinyxml.h"

#include <string>
#include <vector>

struct LevelObjectRecord
{
    std::string TypeName;
    int Id = 0;
    bool LevelEditLocked = false;
    bool Updateable = true;
    bool AlwaysUpdate = false;
    std::string DepthLayer;
    int DepthValue = 0;
    Vector2 Position;
    float Rotation = 0.0f;
    float AutoRotationValue = 0.0f;
    Vector2 Dimensions;
    std::string MaterialName;
    bool PositionalAudioEnabled = false;
    std::string PositionalAudioFile;
    bool IsDrawable = false;
    float Alpha = 1.0f;
    std::string EffectName;
    std::string TextureFilename;
    bool HorizontalFlip = false;
    bool VerticalFlip = false;
    bool IsAnimated = false;
    std::string AnimationFile;
    bool RepeatTextureX = false;
    bool RepeatTextureY = false;
    bool DrawAtNativeDimensions = true;

    static LevelObjectRecord FromXml(TiXmlElement* element);
    static std::vector<LevelObjectRecord> ReadLevelObjects(TiXmlElement* levelRoot);
};

#endif
