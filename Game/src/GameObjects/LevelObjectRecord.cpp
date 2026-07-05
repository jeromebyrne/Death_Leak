#include "GameObjects/LevelObjectRecord.h"

#include "XmlUtilities.h"

namespace
{
TiXmlElement* AttributeElement(TiXmlElement* element, const char* childName)
{
    if (element == nullptr)
    {
        return nullptr;
    }

    if (childName == nullptr || std::string(childName).empty())
    {
        return element;
    }

    return element->FirstChildElement(childName);
}

bool HasAttribute(TiXmlElement* element, const char* childName, const char* key)
{
    TiXmlElement* attributeElement = AttributeElement(element, childName);
    return attributeElement != nullptr && attributeElement->Attribute(key) != nullptr;
}

const char* SafeAttribute(TiXmlElement* element, const char* childName, const char* key, const char* fallback = "")
{
    TiXmlElement* attributeElement = AttributeElement(element, childName);
    if (attributeElement == nullptr)
    {
        return fallback;
    }

    const char* value = attributeElement->Attribute(key);
    return value != nullptr ? value : fallback;
}

float SafeFloat(TiXmlElement* element, const char* childName, const char* key, float fallback = 0.0f)
{
    if (!HasAttribute(element, childName, key))
    {
        return fallback;
    }

    return XmlUtilities::ReadAttributeAsFloat(element, childName, key);
}

bool SafeBool(TiXmlElement* element, const char* childName, const char* key, bool fallback = false)
{
    if (!HasAttribute(element, childName, key))
    {
        return fallback;
    }

    return XmlUtilities::ReadAttributeAsBool(element, childName, key);
}

int SafeInt(TiXmlElement* element, const char* childName, const char* key, int fallback = 0)
{
    if (!HasAttribute(element, childName, key))
    {
        return fallback;
    }

    return XmlUtilities::ReadAttributeAsInt(element, childName, key);
}

int DepthValueFromString(const std::string& depthLayer)
{
    if (depthLayer == "kMoon") return 5999;
    if (depthLayer == "kFarBackground") return 6000;
    if (depthLayer == "kMiddleBackground") return 5500;
    if (depthLayer == "kNearBackground") return 5000;
    if (depthLayer == "kGroundBack") return 4750;
    if (depthLayer == "kGround") return 4500;
    if (depthLayer == "kGroundBlood") return 4000;
    if (depthLayer == "kNpc") return 3500;
    if (depthLayer == "kOrb") return 3250;
    if (depthLayer == "kPlayer") return 3000;
    if (depthLayer == "kGhostVomitProjectile") return 2530;
    if (depthLayer == "kBombProjectile") return 2520;
    if (depthLayer == "kNinjaStarProjectile") return 2510;
    if (depthLayer == "kPlayerProjectile") return 2500;
    if (depthLayer == "kImpactCircles") return 2250;
    if (depthLayer == "kBloodSpray1") return 2000;
    if (depthLayer == "kGroundFront") return 1750;
    if (depthLayer == "kFarForeground") return 1500;
    if (depthLayer == "kMiddleForeground") return 1000;
    if (depthLayer == "kNearForeground") return 500;
    if (depthLayer == "kWeatherForeground") return 20;
    if (depthLayer == "kSolidLines") return 10;
    return 0;
}
}

LevelObjectRecord LevelObjectRecord::FromXml(TiXmlElement* element)
{
    LevelObjectRecord record;
    if (element == nullptr)
    {
        return record;
    }

    record.TypeName = element->Value();
    record.Id = SafeInt(element, "", "id");
    record.LevelEditLocked = SafeBool(element, "", "level_edit_locked");
    record.Updateable = SafeBool(element, "", "updateable", true);
    record.AlwaysUpdate = SafeBool(element, "", "always_update");
    record.DepthLayer = SafeAttribute(element, "position", "depth_layer");
    record.DepthValue = DepthValueFromString(record.DepthLayer);
    record.Position.X = SafeFloat(element, "position", "x");
    record.Position.Y = SafeFloat(element, "position", "y");
    record.Rotation = SafeFloat(element, "position", "rotation");
    record.AutoRotationValue = SafeFloat(element, "position", "auto_rotate_value");
    record.Dimensions.X = SafeFloat(element, "dimensions", "width");
    record.Dimensions.Y = SafeFloat(element, "dimensions", "height");
    record.MaterialName = SafeAttribute(element, "material", "value");
    record.PositionalAudioEnabled = SafeBool(element, "pos_audio_props", "enabled");
    record.PositionalAudioFile = SafeAttribute(element, "pos_audio_props", "file");

    record.IsDrawable = HasAttribute(element, "alpha", "value") ||
        HasAttribute(element, "effect", "name") ||
        HasAttribute(element, "texture", "filename");

    if (record.IsDrawable)
    {
        record.Alpha = SafeFloat(element, "alpha", "value", 1.0f);
        record.EffectName = SafeAttribute(element, "effect", "name");
        record.TextureFilename = SafeAttribute(element, "texture", "filename");
        record.HorizontalFlip = SafeBool(element, "horizontalflip", "value");
        record.VerticalFlip = SafeBool(element, "verticalflip", "value");
        record.IsAnimated = SafeBool(element, "isanimated", "value");
        record.AnimationFile = SafeAttribute(element, "animationfile", "value");
        record.DrawAtNativeDimensions = SafeBool(element, "dimensions", "native", true);
        if (HasAttribute(element, "dimensions", "repeatX"))
        {
            record.RepeatTextureX = SafeBool(element, "dimensions", "repeatX");
        }
        if (HasAttribute(element, "dimensions", "repeatY"))
        {
            record.RepeatTextureY = SafeBool(element, "dimensions", "repeatY");
        }
    }

    return record;
}

std::vector<LevelObjectRecord> LevelObjectRecord::ReadLevelObjects(TiXmlElement* levelRoot)
{
    std::vector<LevelObjectRecord> records;
    if (levelRoot == nullptr)
    {
        return records;
    }

    TiXmlElement* child = levelRoot->FirstChildElement();
    while (child != nullptr)
    {
        const std::string typeName = child->Value();
        if (typeName != "LevelProperties")
        {
            records.push_back(FromXml(child));
        }

        child = child->NextSiblingElement();
    }

    return records;
}
