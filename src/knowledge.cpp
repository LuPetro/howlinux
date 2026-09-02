#include "knowledge.hpp"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cctype>
#include <exception>
#include <fstream>
#include <optional>
#include <sstream>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace howlinux {
namespace {

namespace fs = std::filesystem;

constexpr std::string_view kMetadataFilename = "meta.yaml";
constexpr std::string_view kContentFilename = "content.md";

void addDiagnostic(std::vector<Diagnostic>& diagnostics,
                   const DiagnosticSeverity severity,
                   const fs::path& path,
                   std::string entry_id,
                   std::string message) {
    diagnostics.push_back(
        Diagnostic{severity, path, std::move(entry_id), std::move(message)});
}

bool isMissingPathError(const std::error_code& error) {
    return error == std::errc::no_such_file_or_directory;
}

bool pathSortsBefore(const fs::path& lhs, const fs::path& rhs) {
    return lhs.generic_string() < rhs.generic_string();
}

std::string candidateId(const fs::path& entry_directory) {
    return entry_directory.filename().string();
}

bool hasNonWhitespace(const std::string& value) {
    return std::any_of(value.begin(), value.end(), [](const unsigned char character) {
        return std::isspace(character) == 0;
    });
}

bool isPortableEntryId(const std::string& value) {
    if (value.empty() || value.front() == '-' || value.back() == '-') {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](const unsigned char character) {
        return (character >= static_cast<unsigned char>('a') &&
                character <= static_cast<unsigned char>('z')) ||
               (character >= static_cast<unsigned char>('0') &&
                character <= static_cast<unsigned char>('9')) ||
               character == static_cast<unsigned char>('-');
    });
}

bool pathIsPresent(const fs::path& path) {
    std::error_code error;
    const fs::file_status status = fs::symlink_status(path, error);
    if (error) {
        // An inaccessible path is still a candidate. loadEntry() will emit the
        // concrete diagnostic when it validates the required file.
        return !isMissingPathError(error);
    }
    return status.type() != fs::file_type::not_found &&
           status.type() != fs::file_type::none;
}

bool validateRegularFile(const fs::path& path,
                         const std::string& entry_id,
                         const std::string_view description,
                         std::vector<Diagnostic>& diagnostics) {
    std::error_code error;
    const fs::file_status status = fs::symlink_status(path, error);
    if (error) {
        if (isMissingPathError(error)) {
            addDiagnostic(diagnostics,
                          DiagnosticSeverity::warning,
                          path,
                          entry_id,
                          "missing required " + std::string(description));
        } else {
            addDiagnostic(diagnostics,
                          DiagnosticSeverity::warning,
                          path,
                          entry_id,
                          "cannot inspect required " + std::string(description) +
                              ": " + error.message());
        }
        return false;
    }

    if (status.type() == fs::file_type::not_found) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      path,
                      entry_id,
                      "missing required " + std::string(description));
        return false;
    }

    if (!fs::is_regular_file(status)) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      path,
                      entry_id,
                      std::string(description) +
                          " must be a regular file (symbolic links are not accepted)");
        return false;
    }

    return true;
}

std::optional<std::string> readFileByteExact(
    const fs::path& path,
    const std::string& entry_id,
    const std::string_view description,
    std::vector<Diagnostic>& diagnostics) {
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      path,
                      entry_id,
                      "cannot open " + std::string(description) + " for reading");
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    if (input.bad()) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      path,
                      entry_id,
                      "failed while reading " + std::string(description));
        return std::nullopt;
    }
    return buffer.str();
}

bool readRequiredString(const YAML::Node& metadata,
                        const std::string& field,
                        std::string& output,
                        const fs::path& metadata_path,
                        const std::string& entry_id,
                        std::vector<Diagnostic>& diagnostics) {
    const YAML::Node value = metadata[field];
    if (!value.IsDefined() || value.IsNull()) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      metadata_path,
                      entry_id,
                      "missing required metadata field '" + field + "'");
        return false;
    }
    if (!value.IsScalar()) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      metadata_path,
                      entry_id,
                      "metadata field '" + field + "' must be a string");
        return false;
    }

    output = value.Scalar();
    if (!hasNonWhitespace(output)) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      metadata_path,
                      entry_id,
                      "required metadata field '" + field + "' must not be empty");
        return false;
    }
    return true;
}

bool readOptionalString(const YAML::Node& metadata,
                        const std::string& field,
                        std::string& output,
                        const fs::path& metadata_path,
                        const std::string& entry_id,
                        std::vector<Diagnostic>& diagnostics) {
    const YAML::Node value = metadata[field];
    if (!value.IsDefined()) {
        return true;
    }
    if (value.IsNull() || !value.IsScalar()) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      metadata_path,
                      entry_id,
                      "metadata field '" + field + "' must be a string when present");
        return false;
    }

    output = value.Scalar();
    return true;
}

bool readOptionalStringList(const YAML::Node& metadata,
                            const std::string& field,
                            std::vector<std::string>& output,
                            const fs::path& metadata_path,
                            const std::string& entry_id,
                            std::vector<Diagnostic>& diagnostics) {
    const YAML::Node value = metadata[field];
    if (!value.IsDefined()) {
        return true;
    }
    if (value.IsNull() || !value.IsSequence()) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      metadata_path,
                      entry_id,
                      "metadata field '" + field + "' must be a list of strings");
        return false;
    }

    std::vector<std::string> parsed;
    parsed.reserve(value.size());
    bool valid = true;
    for (std::size_t index = 0; index < value.size(); ++index) {
        const YAML::Node item = value[index];
        if (item.IsNull() || !item.IsScalar()) {
            addDiagnostic(diagnostics,
                          DiagnosticSeverity::warning,
                          metadata_path,
                          entry_id,
                          "metadata field '" + field + "' item " +
                              std::to_string(index) + " must be a string");
            valid = false;
            continue;
        }
        parsed.push_back(item.Scalar());
    }

    if (valid) {
        output = std::move(parsed);
    }
    return valid;
}

bool isKnownMetadataField(const std::string& field) {
    static const std::unordered_set<std::string> known_fields = {
        "id",        "title",    "type",       "command", "aliases",
        "keywords",  "related",  "intent",     "difficulty",
        "platforms", "tags",     "examples",
    };
    return known_fields.contains(field);
}

std::vector<std::string> relativeComponents(const fs::path& path,
                                            const fs::path& root) {
    std::vector<std::string> components;
    const fs::path relative = path.lexically_relative(root);
    for (const auto& component : relative) {
        if (component != "." && !component.empty()) {
            components.push_back(component.string());
        }
    }
    return components;
}

}  // namespace

bool KnowledgeLoadReport::hasIssues() const {
    return !diagnostics.empty();
}

std::string formatDiagnostic(const Diagnostic& diagnostic) {
    std::ostringstream output;
    output << (diagnostic.severity == DiagnosticSeverity::error ? "error" : "warning");

    if (!diagnostic.path.empty()) {
        output << ": " << diagnostic.path.generic_string();
    }
    if (!diagnostic.entry_id.empty()) {
        output << " (entry '" << diagnostic.entry_id << "')";
    }
    if (!diagnostic.message.empty()) {
        output << ": " << diagnostic.message;
    }
    return output.str();
}

std::optional<KnowledgeEntry> KnowledgeBase::loadEntry(
    const fs::path& entry_directory,
    const std::string& category,
    std::vector<Diagnostic>& diagnostics) const {
    const fs::path metadata_path = entry_directory / kMetadataFilename;
    const fs::path content_path = entry_directory / kContentFilename;
    const std::string provisional_id = candidateId(entry_directory);

    const bool metadata_valid = validateRegularFile(metadata_path,
                                                    provisional_id,
                                                    "metadata file 'meta.yaml'",
                                                    diagnostics);
    const bool content_valid = validateRegularFile(content_path,
                                                   provisional_id,
                                                   "content file 'content.md'",
                                                   diagnostics);
    if (!metadata_valid || !content_valid) {
        return std::nullopt;
    }

    const auto metadata_text = readFileByteExact(metadata_path,
                                                 provisional_id,
                                                 "metadata file 'meta.yaml'",
                                                 diagnostics);
    if (!metadata_text) {
        return std::nullopt;
    }

    YAML::Node metadata;
    try {
        metadata = YAML::Load(*metadata_text);
    } catch (const YAML::Exception& error) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      metadata_path,
                      provisional_id,
                      "invalid YAML: " + std::string(error.what()));
        return std::nullopt;
    } catch (const std::exception& error) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      metadata_path,
                      provisional_id,
                      "cannot parse metadata: " + std::string(error.what()));
        return std::nullopt;
    }

    if (!metadata.IsMap()) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      metadata_path,
                      provisional_id,
                      "metadata root must be a mapping");
        return std::nullopt;
    }

    bool metadata_keys_valid = true;
    std::unordered_set<std::string> seen_fields;
    std::vector<std::string> unknown_fields;
    for (const auto& item : metadata) {
        if (!item.first.IsScalar()) {
            addDiagnostic(diagnostics,
                          DiagnosticSeverity::warning,
                          metadata_path,
                          provisional_id,
                          "metadata field names must be strings");
            metadata_keys_valid = false;
            continue;
        }

        const std::string field = item.first.Scalar();
        if (!seen_fields.insert(field).second) {
            addDiagnostic(diagnostics,
                          DiagnosticSeverity::warning,
                          metadata_path,
                          provisional_id,
                          "duplicate metadata field '" + field + "'");
            metadata_keys_valid = false;
            continue;
        }
        if (!isKnownMetadataField(field)) {
            unknown_fields.push_back(field);
        }
    }

    if (!metadata_keys_valid) {
        return std::nullopt;
    }

    KnowledgeEntry entry;
    entry.category = category;
    entry.source_directory = entry_directory;

    bool fields_valid = true;
    fields_valid = readRequiredString(metadata,
                                      "id",
                                      entry.id,
                                      metadata_path,
                                      provisional_id,
                                      diagnostics) &&
                   fields_valid;
    const std::string diagnostic_id =
        hasNonWhitespace(entry.id) ? entry.id : provisional_id;
    fields_valid = readRequiredString(metadata,
                                      "title",
                                      entry.title,
                                      metadata_path,
                                      diagnostic_id,
                                      diagnostics) &&
                   fields_valid;
    fields_valid = readRequiredString(metadata,
                                      "type",
                                      entry.type,
                                      metadata_path,
                                      diagnostic_id,
                                      diagnostics) &&
                   fields_valid;
    fields_valid = readOptionalString(metadata,
                                      "command",
                                      entry.command,
                                      metadata_path,
                                      diagnostic_id,
                                      diagnostics) &&
                   fields_valid;
    fields_valid = readOptionalString(metadata,
                                      "difficulty",
                                      entry.difficulty,
                                      metadata_path,
                                      diagnostic_id,
                                      diagnostics) &&
                   fields_valid;
    fields_valid = readOptionalStringList(metadata,
                                          "aliases",
                                          entry.aliases,
                                          metadata_path,
                                          diagnostic_id,
                                          diagnostics) &&
                   fields_valid;
    fields_valid = readOptionalStringList(metadata,
                                          "keywords",
                                          entry.keywords,
                                          metadata_path,
                                          diagnostic_id,
                                          diagnostics) &&
                   fields_valid;
    fields_valid = readOptionalStringList(metadata,
                                          "related",
                                          entry.related,
                                          metadata_path,
                                          diagnostic_id,
                                          diagnostics) &&
                   fields_valid;
    fields_valid = readOptionalStringList(metadata,
                                          "intent",
                                          entry.intents,
                                          metadata_path,
                                          diagnostic_id,
                                          diagnostics) &&
                   fields_valid;
    fields_valid = readOptionalStringList(metadata,
                                          "platforms",
                                          entry.platforms,
                                          metadata_path,
                                          diagnostic_id,
                                          diagnostics) &&
                   fields_valid;
    fields_valid = readOptionalStringList(metadata,
                                          "tags",
                                          entry.tags,
                                          metadata_path,
                                          diagnostic_id,
                                          diagnostics) &&
                   fields_valid;
    fields_valid = readOptionalStringList(metadata,
                                          "examples",
                                          entry.examples,
                                          metadata_path,
                                          diagnostic_id,
                                          diagnostics) &&
                   fields_valid;

    std::sort(unknown_fields.begin(), unknown_fields.end());
    for (const auto& field : unknown_fields) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      metadata_path,
                      diagnostic_id,
                      "unknown metadata field '" + field + "' was ignored");
    }

    if (!fields_valid) {
        return std::nullopt;
    }

    if (!isPortableEntryId(entry.id)) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      metadata_path,
                      entry.id,
                      "id should use lowercase ASCII letters, digits and hyphens only");
    }
    if (entry.id != provisional_id) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      metadata_path,
                      entry.id,
                      "id does not match the entry directory name '" + provisional_id + "'");
    }

    const auto content = readFileByteExact(content_path,
                                           entry.id,
                                           "content file 'content.md'",
                                           diagnostics);
    if (!content) {
        return std::nullopt;
    }
    if (!hasNonWhitespace(*content)) {
        addDiagnostic(diagnostics,
                      DiagnosticSeverity::warning,
                      content_path,
                      entry.id,
                      "content file must not be empty");
        return std::nullopt;
    }
    entry.content = *content;
    return entry;
}

KnowledgeLoadReport KnowledgeBase::load(const fs::path& directory) {
    root_ = directory.lexically_normal();
    entries_.clear();
    id_lookup_.clear();
    report_ = KnowledgeLoadReport{};

    std::error_code root_error;
    const fs::file_status root_status = fs::status(root_, root_error);
    if (root_error) {
        const std::string message = isMissingPathError(root_error)
                                        ? "knowledge root does not exist"
                                        : "cannot inspect knowledge root: " +
                                              root_error.message();
        addDiagnostic(report_.diagnostics,
                      DiagnosticSeverity::error,
                      root_,
                      {},
                      message);
        return report_;
    }
    if (root_status.type() == fs::file_type::not_found ||
        root_status.type() == fs::file_type::none) {
        addDiagnostic(report_.diagnostics,
                      DiagnosticSeverity::error,
                      root_,
                      {},
                      "knowledge root does not exist");
        return report_;
    }
    if (!fs::is_directory(root_status)) {
        addDiagnostic(report_.diagnostics,
                      DiagnosticSeverity::error,
                      root_,
                      {},
                      "knowledge root is not a directory");
        return report_;
    }

    std::vector<fs::path> directories{root_};
    for (std::size_t directory_index = 0;
         directory_index < directories.size();
         ++directory_index) {
        const fs::path current = directories[directory_index];
        std::error_code iterator_error;
        fs::directory_iterator iterator(current,
                                        fs::directory_options::none,
                                        iterator_error);
        if (iterator_error) {
            const bool root_failed = directory_index == 0;
            addDiagnostic(report_.diagnostics,
                          root_failed ? DiagnosticSeverity::error
                                      : DiagnosticSeverity::warning,
                          current,
                          {},
                          "cannot read directory: " + iterator_error.message());
            if (root_failed) {
                return report_;
            }
            continue;
        }

        std::vector<fs::path> children;
        const fs::directory_iterator end;
        while (iterator != end) {
            children.push_back(iterator->path());
            iterator.increment(iterator_error);
            if (iterator_error) {
                const bool root_failed = directory_index == 0;
                addDiagnostic(report_.diagnostics,
                              root_failed ? DiagnosticSeverity::error
                                          : DiagnosticSeverity::warning,
                              current,
                              {},
                              "failed while reading directory: " +
                                  iterator_error.message());
                if (root_failed) {
                    entries_.clear();
                    id_lookup_.clear();
                    return report_;
                }
                break;
            }
        }

        if (directory_index == 0) {
            report_.root_available = true;
        }

        std::sort(children.begin(), children.end(), pathSortsBefore);
        for (const auto& child : children) {
            std::error_code status_error;
            const fs::file_status child_status = fs::symlink_status(child, status_error);
            if (status_error) {
                addDiagnostic(report_.diagnostics,
                              DiagnosticSeverity::warning,
                              child,
                              {},
                              "cannot inspect path while scanning knowledge root: " +
                                  status_error.message());
                continue;
            }

            if (fs::is_directory(child_status)) {
                directories.push_back(child);
                continue;
            }

            if (fs::is_symlink(child_status)) {
                std::error_code target_error;
                const fs::file_status target_status = fs::status(child, target_error);
                if (!target_error && fs::is_directory(target_status)) {
                    addDiagnostic(report_.diagnostics,
                                  DiagnosticSeverity::warning,
                                  child,
                                  {},
                                  "symbolic-link directory was not traversed");
                }
            }
        }
    }

    std::vector<fs::path> candidates;
    candidates.reserve(directories.size());
    for (const auto& current : directories) {
        if (pathIsPresent(current / kMetadataFilename) ||
            pathIsPresent(current / kContentFilename)) {
            candidates.push_back(current);
        }
    }
    std::sort(candidates.begin(), candidates.end(), pathSortsBefore);
    candidates.erase(std::unique(candidates.begin(), candidates.end()),
                     candidates.end());
    report_.discovered_entries = candidates.size();

    std::unordered_map<std::string, fs::path> first_source_by_id;
    first_source_by_id.reserve(candidates.size());

    for (const auto& entry_directory : candidates) {
        const std::vector<std::string> components =
            relativeComponents(entry_directory, root_);
        if (components.size() < 2) {
            addDiagnostic(report_.diagnostics,
                          DiagnosticSeverity::warning,
                          entry_directory,
                          candidateId(entry_directory),
                          "entry must be stored below a category directory");
            ++report_.skipped_entries;
            continue;
        }

        auto entry = loadEntry(entry_directory,
                               components.front(),
                               report_.diagnostics);
        if (!entry) {
            ++report_.skipped_entries;
            continue;
        }

        const auto [first_source, inserted] =
            first_source_by_id.emplace(entry->id, entry->source_directory);
        if (!inserted) {
            addDiagnostic(report_.diagnostics,
                          DiagnosticSeverity::warning,
                          entry->source_directory / kMetadataFilename,
                          entry->id,
                          "duplicate id; the entry at '" +
                              first_source->second.generic_string() +
                              "' was loaded first, so this entry was skipped");
            ++report_.skipped_entries;
            continue;
        }

        entries_.push_back(std::move(*entry));
    }

    std::sort(entries_.begin(), entries_.end(), [](const KnowledgeEntry& lhs,
                                                   const KnowledgeEntry& rhs) {
        return lhs.id < rhs.id;
    });
    rebuildLookup();
    report_.loaded_entries = entries_.size();

    for (const auto& entry : entries_) {
        for (const auto& related_id : entry.related) {
            if (id_lookup_.find(related_id) == id_lookup_.end()) {
                addDiagnostic(report_.diagnostics,
                              DiagnosticSeverity::warning,
                              entry.source_directory / kMetadataFilename,
                              entry.id,
                              "related id '" + related_id +
                                  "' does not refer to a loaded entry");
            }
        }
    }

    return report_;
}

void KnowledgeBase::rebuildLookup() {
    id_lookup_.clear();
    id_lookup_.reserve(entries_.size());
    for (std::size_t index = 0; index < entries_.size(); ++index) {
        id_lookup_.emplace(entries_[index].id, index);
    }
}

const KnowledgeEntry* KnowledgeBase::findById(const std::string& id) const noexcept {
    const auto found = id_lookup_.find(id);
    if (found == id_lookup_.end()) {
        return nullptr;
    }
    return &entries_[found->second];
}

}  // namespace howlinux
