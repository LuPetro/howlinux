#pragma once
#include <string>
#include <vector>
#include <filesystem>

// Ein einzelner Knowledge-Eintrag, z.B. "rename-folder" oder der Command "mv".
// Kommt 1:1 aus meta.yaml + content.md.
struct KnowledgeEntry {
    std::string id;        // z.B. "rename-folder"
    std::string title;     // z.B. "Rename a folder"
    std::string type;      // "howto" oder "command"
    std::string command;   // z.B. "mv" (kann leer sein)

    std::vector<std::string> aliases;   // andere Formulierungen der gleichen Frage
    std::vector<std::string> keywords;  // Wörter die zu diesem Eintrag passen
    std::vector<std::string> related;   // IDs verwandter Einträge

    std::string content;   // kompletter Inhalt aus content.md
};

// Lädt alle Knowledge-Einträge aus einem Ordner (z.B. "knowledge/").
// Erwartet Struktur: <directory>/<kategorie>/<entry-id>/meta.yaml + content.md
class KnowledgeBase {
public:
    void load(const std::filesystem::path& directory);

    const std::vector<KnowledgeEntry>& entries() const { return entries_; }

private:
    std::vector<KnowledgeEntry> entries_;

    // Versucht genau einen Eintrag aus einem Ordner zu laden.
    // Gibt false zurück wenn meta.yaml oder content.md fehlen/kaputt sind.
    bool loadEntry(const std::filesystem::path& entryDir, KnowledgeEntry& out);
};
