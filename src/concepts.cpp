#include "concepts.hpp"

#include "query.hpp"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <functional>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace howlinux {
namespace {

struct ParsedConceptGroup {
    std::string canonical;
    std::string source_name;
    std::vector<std::string> aliases;
};

void addError(ConceptLoadReport& report,
              const std::filesystem::path& path,
              std::string concept_name,
              std::string message) {
    report.diagnostics.push_back(
        {DiagnosticSeverity::error, path, std::move(concept_name),
         std::move(message)});
}

[[nodiscard]] std::string normalizePhrase(const std::string& phrase) {
    return joinTokens(tokenize(phrase, true, false));
}

[[nodiscard]] bool hasErrors(const ConceptLoadReport& report) {
    return std::any_of(
        report.diagnostics.begin(), report.diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

void detectCycles(const std::vector<ParsedConceptGroup>& groups,
                  const std::filesystem::path& path,
                  ConceptLoadReport& report) {
    std::unordered_set<std::string> canonicals;
    for (const auto& group : groups) {
        canonicals.insert(group.canonical);
    }

    std::unordered_map<std::string, std::vector<std::string>> graph;
    for (const auto& group : groups) {
        auto& destinations = graph[group.canonical];
        for (const auto& alias : group.aliases) {
            if (alias != group.canonical && canonicals.contains(alias)) {
                destinations.push_back(alias);
            }
        }
        std::sort(destinations.begin(), destinations.end());
        destinations.erase(
            std::unique(destinations.begin(), destinations.end()),
            destinations.end());
    }

    enum class VisitState { unseen, visiting, complete };
    std::unordered_map<std::string, VisitState> states;
    std::vector<std::string> stack;
    bool cycle_reported = false;

    std::function<void(const std::string&)> visit =
        [&](const std::string& canonical) {
            if (cycle_reported) {
                return;
            }

            states[canonical] = VisitState::visiting;
            stack.push_back(canonical);

            const auto adjacency = graph.find(canonical);
            if (adjacency != graph.end()) {
                for (const auto& destination : adjacency->second) {
                    const auto state = states.find(destination);
                    const VisitState destination_state =
                        state == states.end() ? VisitState::unseen
                                              : state->second;
                    if (destination_state == VisitState::unseen) {
                        visit(destination);
                        if (cycle_reported) {
                            return;
                        }
                    } else if (destination_state == VisitState::visiting) {
                        const auto beginning =
                            std::find(stack.begin(), stack.end(), destination);
                        std::ostringstream message;
                        message << "circular concept definition: ";
                        bool first = true;
                        for (auto item = beginning; item != stack.end(); ++item) {
                            if (!first) {
                                message << " -> ";
                            }
                            message << *item;
                            first = false;
                        }
                        message << " -> " << destination;
                        addError(report, path, canonical, message.str());
                        cycle_reported = true;
                        return;
                    }
                }
            }

            stack.pop_back();
            states[canonical] = VisitState::complete;
        };

    for (const auto& group : groups) {
        if (states.find(group.canonical) == states.end()) {
            visit(group.canonical);
            if (cycle_reported) {
                break;
            }
        }
    }
}

}  // namespace

bool ConceptLoadReport::hasIssues() const {
    return !usable || !diagnostics.empty();
}

ConceptLoadReport ConceptDictionary::load(const std::filesystem::path& file) {
    phrase_to_canonical_.clear();
    definitions_.clear();
    report_ = {};

    std::error_code filesystem_error;
    const bool exists = std::filesystem::exists(file, filesystem_error);
    if (filesystem_error) {
        report_.usable = false;
        addError(report_, file, {},
                 "cannot inspect concepts file: " +
                     filesystem_error.message());
        return report_;
    }
    if (!exists) {
        // A missing file explicitly means that synonym expansion is disabled.
        return report_;
    }

    report_.file_present = true;
    const bool regular_file =
        std::filesystem::is_regular_file(file, filesystem_error);
    if (filesystem_error || !regular_file) {
        report_.usable = false;
        addError(report_, file, {},
                 filesystem_error
                     ? "cannot inspect concepts file: " +
                           filesystem_error.message()
                     : "concepts path is not a regular file");
        return report_;
    }

    YAML::Node root;
    try {
        root = YAML::LoadFile(file.string());
    } catch (const YAML::Exception& error) {
        report_.usable = false;
        addError(report_, file, {},
                 "cannot parse concepts YAML: " + std::string(error.what()));
        return report_;
    } catch (const std::exception& error) {
        report_.usable = false;
        addError(report_, file, {},
                 "cannot read concepts file: " + std::string(error.what()));
        return report_;
    }

    if (!root || !root.IsMap()) {
        report_.usable = false;
        addError(report_, file, {},
                 "concepts YAML root must be a mapping");
        return report_;
    }

    YAML::Node concepts_node;
    std::size_t concepts_keys = 0;
    for (const auto& item : root) {
        if (!item.first.IsScalar()) {
            continue;
        }
        try {
            if (item.first.as<std::string>() == "concepts") {
                ++concepts_keys;
                concepts_node = item.second;
            }
        } catch (const YAML::Exception&) {
            // A malformed unrelated root key does not affect the concepts map.
        }
    }

    if (concepts_keys == 0) {
        report_.usable = false;
        addError(report_, file, {},
                 "concepts YAML must contain a 'concepts' mapping");
        return report_;
    }
    if (concepts_keys > 1) {
        addError(report_, file, {},
                 "duplicate 'concepts' definitions at the YAML root");
    }
    if (!concepts_node || !concepts_node.IsMap()) {
        report_.usable = false;
        addError(report_, file, {}, "'concepts' must be a mapping");
        return report_;
    }

    std::vector<ParsedConceptGroup> groups;
    std::unordered_map<std::string, std::string> canonical_sources;

    for (const auto& item : concepts_node) {
        if (!item.first.IsScalar()) {
            addError(report_, file, {},
                     "each concept canonical name must be a scalar string");
            continue;
        }

        std::string source_name;
        try {
            source_name = item.first.as<std::string>();
        } catch (const YAML::Exception& error) {
            addError(report_, file, {},
                     "cannot read concept canonical name: " +
                         std::string(error.what()));
            continue;
        }

        const auto canonical = normalizePhrase(source_name);
        if (canonical.empty()) {
            addError(report_, file, source_name,
                     "concept canonical name is empty after normalization");
            continue;
        }

        const auto [source, inserted] =
            canonical_sources.emplace(canonical, source_name);
        if (!inserted) {
            addError(report_, file, canonical,
                     "duplicate canonical concept after normalization; also "
                     "defined as '" +
                         source->second + "'");
            continue;
        }

        if (!item.second || !item.second.IsSequence()) {
            addError(report_, file, canonical,
                     "concept aliases must be a sequence");
            continue;
        }

        ParsedConceptGroup group;
        group.canonical = canonical;
        group.source_name = source_name;
        std::unordered_set<std::string> local_aliases;

        for (const auto& alias_node : item.second) {
            if (!alias_node.IsScalar()) {
                addError(report_, file, canonical,
                         "each concept alias must be a scalar string");
                continue;
            }

            std::string source_alias;
            try {
                source_alias = alias_node.as<std::string>();
            } catch (const YAML::Exception& error) {
                addError(report_, file, canonical,
                         "cannot read concept alias: " +
                             std::string(error.what()));
                continue;
            }

            auto alias = normalizePhrase(source_alias);
            if (alias.empty()) {
                addError(report_, file, canonical,
                         "concept alias '" + source_alias +
                             "' is empty after normalization");
                continue;
            }

            if (!local_aliases.insert(alias).second) {
                addError(report_, file, canonical,
                         "duplicate alias after normalization: '" + alias +
                             "'");
                continue;
            }
            group.aliases.push_back(std::move(alias));
        }

        groups.push_back(std::move(group));
    }

    std::sort(groups.begin(), groups.end(),
              [](const ParsedConceptGroup& left,
                 const ParsedConceptGroup& right) {
                  return left.canonical < right.canonical;
              });

    std::unordered_map<std::string, std::string> phrases;
    for (const auto& group : groups) {
        phrases.emplace(group.canonical, group.canonical);
    }

    for (const auto& group : groups) {
        for (const auto& alias : group.aliases) {
            // The documented format commonly lists the canonical spelling in
            // its aliases.  One such redundant occurrence is harmless.
            if (alias == group.canonical) {
                continue;
            }

            const auto existing = phrases.find(alias);
            if (existing != phrases.end()) {
                addError(report_, file, group.canonical,
                         "concept phrase collision for '" + alias +
                             "': maps to both '" + existing->second +
                             "' and '" + group.canonical + "'");
                continue;
            }
            phrases.emplace(alias, group.canonical);
        }
    }

    detectCycles(groups, file, report_);

    if (hasErrors(report_)) {
        report_.usable = false;
        report_.concepts_loaded = 0;
        return report_;
    }

    std::vector<PhraseDefinition> definitions;
    definitions.reserve(phrases.size());
    for (const auto& [phrase, canonical] : phrases) {
        PhraseDefinition definition;
        definition.phrase = phrase;
        definition.canonical = canonical;
        definition.tokens = tokenize(phrase, true, false);
        definitions.push_back(std::move(definition));
    }

    std::sort(definitions.begin(), definitions.end(),
              [](const PhraseDefinition& left,
                 const PhraseDefinition& right) {
                  if (left.tokens.size() != right.tokens.size()) {
                      return left.tokens.size() > right.tokens.size();
                  }
                  if (left.phrase != right.phrase) {
                      return left.phrase < right.phrase;
                  }
                  return left.canonical < right.canonical;
              });

    phrase_to_canonical_ = std::move(phrases);
    definitions_ = std::move(definitions);
    report_.usable = true;
    report_.concepts_loaded = groups.size();
    return report_;
}

std::vector<ConceptMatch> ConceptDictionary::detect(
    const std::vector<std::string>& ordered_tokens) const {
    std::vector<ConceptMatch> matches;
    std::size_t offset = 0;

    while (offset < ordered_tokens.size()) {
        const PhraseDefinition* selected = nullptr;
        for (const auto& definition : definitions_) {
            if (definition.tokens.size() > ordered_tokens.size() - offset) {
                continue;
            }
            if (std::equal(definition.tokens.begin(), definition.tokens.end(),
                           ordered_tokens.begin() +
                               static_cast<std::ptrdiff_t>(offset))) {
                selected = &definition;
                break;  // Definitions are sorted longest-first.
            }
        }

        if (selected == nullptr) {
            ++offset;
            continue;
        }

        matches.push_back({selected->canonical, selected->phrase, offset,
                           selected->tokens.size()});
        offset += selected->tokens.size();
    }

    return matches;
}

std::optional<std::string> ConceptDictionary::canonicalForPhrase(
    const std::string& normalized_phrase) const {
    const auto phrase = normalizePhrase(normalized_phrase);
    const auto match = phrase_to_canonical_.find(phrase);
    if (match == phrase_to_canonical_.end()) {
        return std::nullopt;
    }
    return match->second;
}

}  // namespace howlinux
