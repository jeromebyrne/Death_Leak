#pragma once
#include "../../Game/tinyxml/tinyxml.h"
#include "BitmapText.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace deathleak::game {
// strings.xml ships only en-gb entries. Other selected locales use the authored
// English text until translations are supplied, rather than showing a blank note.
class PaperText {
public:
    bool Load(const std::string& path) { return document_.LoadFile(path.c_str()); }
    std::string Lookup(const std::string& key, const std::string& locale) const {
        const auto* root = document_.RootElement();
        const auto* entry = root ? root->FirstChildElement(key.c_str()) : nullptr;
        if (!entry) return {};
        const auto* translated = entry->FirstChildElement(locale.empty() ? "en-gb" : locale.c_str());
        if (!translated) translated = entry->FirstChildElement("en-gb");
        return translated && translated->GetText() ? translated->GetText() : "";
    }
private:
    TiXmlDocument document_;
};

inline std::vector<std::string> WrapPaperText(const std::string& input, size_t columns) {
    std::vector<std::string> lines;
    std::string line, word;
    auto flushWord = [&] {
        if (word.empty()) return;
        while (word.size() > columns) {
            if (!line.empty()) { lines.push_back(line); line.clear(); }
            lines.push_back(word.substr(0, columns)); word.erase(0, columns);
        }
        if (!line.empty() && line.size() + 1 + word.size() > columns) {
            lines.push_back(line); line.clear();
        }
        if (!line.empty()) line += ' ';
        line += word; word.clear();
    };
    for (unsigned char c : input) {
        if (c == '\n') { flushWord(); lines.push_back(line); line.clear(); }
        else if (std::isspace(c)) flushWord();
        else word += static_cast<char>(c);
    }
    flushWord();
    if (!line.empty()) lines.push_back(line);
    return lines;
}
inline void DrawPaperModal(render::Renderer& renderer, const std::string& title,
                           const std::string& description, render::TextureId background,
                           render::TextureId backButton, bool ready) {
    if (background) renderer.DrawSprite({background, {}, {0,0,1280,720}, {255,255,255,204}, 1.f, 0.f, false, true});
    auto upper = [](std::string value) {
        for (char& c : value) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return value;
    };
    const auto titleLines = WrapPaperText(upper(title), 46);
    float y = 70.f;
    for (const auto& line : titleLines) {
        DrawBitmapText(renderer, line, std::max(50.f, (1280.f - line.size()*24.f)*0.5f), y, 4, {153,153,153,255});
        y += 42.f;
    }
    const auto bodyLines = WrapPaperText(upper(description), 56);
    y = std::max(220.f, 340.f - bodyLines.size()*15.f);
    for (const auto& line : bodyLines) {
        DrawBitmapText(renderer, line, std::max(40.f, (1280.f - line.size()*18.f)*0.5f), y, 3);
        y += 32.f;
    }
    if (ready) {
        if (backButton) renderer.DrawSprite({backButton, {}, {1130,610,43,43}});
        DrawBitmapText(renderer, "ESC BACK", 960, 660, 3);
    }
}
}
