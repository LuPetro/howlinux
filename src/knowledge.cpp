#include "knowledge.hpp"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace fs = std::filesystem;

bool KnowledgeBase::loadEntry(const fs::path& entryDir, KnowledgeEntry& out) {
    fs::path metaPath = entryDir / "meta.yaml";
    fs::path contentPath = entryDir / "content.md";

    // Kein vollständiger Eintrag -> einfach überspringen, kein Fehler
    if (!fs::exists(metaPath) || !fs::exists(contentPath)) {
        return false;
    }

    try {
        YAML::Node meta = YAML::LoadFile(metaPath.string());

        out.id = meta["id"] ? meta["id"].as<std::string>() : "";
        out.title = meta["title"] ? meta["title"].as<std::string>() : "";
        out.type = meta["type"] ? meta["type"].as<std::string>() : "";
        out.command = meta["command"] ? meta["command"].as<std::string>() : "";

        if (meta["aliases"]) {
            for (const auto& a : meta["aliases"]) {
                out.aliases.push_back(a.as<std::string>());
            }
        }
        if (meta["keywords"]) {
            for (const auto& k : meta["keywords"]) {
                out.keywords.push_back(k.as<std::string>());
            }
        }
        if (meta["related"]) {
            for (const auto& r : meta["related"]) {
                out.related.push_back(r.as<std::string>());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading " << metaPath << ": " << e.what() << "\n";
        return false;
    }

    // content.md einfach komplett als String einlesen
    std::ifstream contentFile(contentPath);
    std::stringstream buffer;
    buffer << contentFile.rdbuf();
    out.content = buffer.str();

    return true;
}

void KnowledgeBase::load(const fs::path& directory) {
    if (!fs::exists(directory)) {
        std::cerr << "Knowledge directory not found: " << directory << "\n";
        return;
    }

    // Erste Ebene: Kategorien (z.B. "commands", "topics")
    for (const auto& category : fs::directory_iterator(directory)) {
        if (!category.is_directory()) continue;

        // Zweite Ebene: einzelne Einträge (z.B. "rename-folder")
        for (const auto& entryDir : fs::directory_iterator(category.path())) {
            if (!entryDir.is_directory()) continue;

            KnowledgeEntry entry;
            if (loadEntry(entryDir.path(), entry)) {
                entries_.push_back(std::move(entry));
            }
        }
    }
}
