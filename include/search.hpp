#pragma once
#include "knowledge.hpp"
#include <string>
#include <vector>

// Ein Suchtreffer: welcher Eintrag, mit welchem Score.
struct SearchResult {
    const KnowledgeEntry* entry;
    double score;
};

// Zerlegt eine Eingabe in normalisierte Tokens:
// lowercase -> Satzzeichen weg -> Wörter -> Stopwörter raus
std::vector<std::string> normalize(const std::string& input);

class SearchEngine {
public:
    explicit SearchEngine(const KnowledgeBase& kb) : kb_(kb) {}

    // Durchsucht alle Einträge und gibt sie absteigend nach Score sortiert zurück.
    // Nur Einträge mit score > 0 werden zurückgegeben.
    std::vector<SearchResult> search(const std::string& rawQuery) const;

private:
    const KnowledgeBase& kb_;
};
