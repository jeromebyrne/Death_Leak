#pragma once
#include "../../Game/tinyxml/tinyxml.h"
#include <filesystem>
#include <string>

namespace deathleak::game {
// Separate from the DL save, as in the Windows Settings/SaveManager split.
struct Settings {
    bool music = true;
    bool sfx = true;

    bool Load(const std::string& path) {
        if (!std::filesystem::exists(path)) return true;
        TiXmlDocument document(path.c_str());
        if (!document.LoadFile()) return false;
        const auto* root = document.RootElement();
        const auto* audio = root && std::string(root->Value()) == "settings" ? root->FirstChildElement("audio") : nullptr;
        if (!audio) return false;
        int musicValue = 0, sfxValue = 0;
        if (audio->QueryIntAttribute("music_enabled", &musicValue) != TIXML_SUCCESS ||
            audio->QueryIntAttribute("sfx_enabled", &sfxValue) != TIXML_SUCCESS ||
            (musicValue != 0 && musicValue != 1) || (sfxValue != 0 && sfxValue != 1)) return false;
        music = musicValue != 0;
        sfx = sfxValue != 0;
        return true;
    }

    bool Save(const std::string& path) const {
        TiXmlDocument document;
        auto* root = new TiXmlElement("settings");
        auto* audio = new TiXmlElement("audio");
        audio->SetAttribute("music_enabled", music ? 1 : 0);
        audio->SetAttribute("sfx_enabled", sfx ? 1 : 0);
        root->LinkEndChild(audio);
        document.LinkEndChild(root);
        const std::string temporary = path + ".tmp";
        if (!document.SaveFile(temporary.c_str())) return false;
        std::error_code error;
        std::filesystem::rename(temporary, path, error);
        return !error;
    }
};
}
