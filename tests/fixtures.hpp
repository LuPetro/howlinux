#pragma once

#include "test_harness.hpp"

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace hltest {

struct EntrySpec {
    std::string id;
    std::string title;
    std::string type;
    std::string command;
    std::vector<std::string> aliases;
    std::vector<std::string> keywords;
    std::vector<std::string> related;
    std::vector<std::string> intents;
    std::string content{"Fixture content\n"};
};

inline EntrySpec entrySpec(std::string id,
                           std::string title,
                           std::string type = "howto") {
    EntrySpec result;
    result.id = std::move(id);
    result.title = std::move(title);
    result.type = std::move(type);
    return result;
}

inline std::string yamlQuote(const std::string& value) {
    std::string result{"\""};
    for (const char character : value) {
        switch (character) {
            case '\\':
                result += "\\\\";
                break;
            case '"':
                result += "\\\"";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result.push_back(character);
                break;
        }
    }
    result.push_back('"');
    return result;
}

inline void appendYamlList(std::ostringstream& yaml,
                           const char* field,
                           const std::vector<std::string>& values) {
    if (values.empty()) {
        return;
    }
    yaml << field << ":\n";
    for (const auto& value : values) {
        yaml << "  - " << yamlQuote(value) << '\n';
    }
}

inline void writeEntry(const std::filesystem::path& root,
                       const std::filesystem::path& relative_directory,
                       const EntrySpec& spec) {
    std::ostringstream yaml;
    yaml << "id: " << yamlQuote(spec.id) << '\n'
         << "title: " << yamlQuote(spec.title) << '\n'
         << "type: " << yamlQuote(spec.type) << '\n';
    if (!spec.command.empty()) {
        yaml << "command: " << yamlQuote(spec.command) << '\n';
    }
    appendYamlList(yaml, "aliases", spec.aliases);
    appendYamlList(yaml, "keywords", spec.keywords);
    appendYamlList(yaml, "related", spec.related);
    appendYamlList(yaml, "intent", spec.intents);

    const auto directory = root / relative_directory;
    writeText(directory / "meta.yaml", yaml.str());
    writeText(directory / "content.md", spec.content);
}

inline void writeRawEntry(const std::filesystem::path& root,
                          const std::filesystem::path& relative_directory,
                          const std::string& metadata,
                          const std::string& content) {
    const auto directory = root / relative_directory;
    writeText(directory / "meta.yaml", metadata);
    writeText(directory / "content.md", content);
}

inline void writeStandardConcepts(const std::filesystem::path& root) {
    writeText(root / "concepts.yaml", R"yaml(concepts:
  folder:
    - folder
    - directory
    - dir
  delete:
    - delete
    - remove
    - erase
  rename:
    - rename
    - change name
    - give another name
  permissions:
    - permission
    - permissions
    - rights
)yaml");
}

inline void writeSearchKnowledge(const std::filesystem::path& root) {
    writeStandardConcepts(root);

    auto rename = entrySpec("rename-folder", "Rename a folder");
    rename.command = "mv";
    rename.aliases = {"rename folder", "rename directory",
                      "change directory name"};
    rename.keywords = {"rename", "folder", "directory", "name"};
    rename.related = {"mv"};
    rename.intents = {"how_to"};
    rename.content = "## Rename safely\n\n```bash\nmv OLD_NAME NEW_NAME\n```\n\nGröße stays UTF-8.\n";
    writeEntry(root, "topics/rename-folder", rename);

    auto move = entrySpec("mv", "mv - move files", "command");
    move.command = "mv";
    move.aliases = {"mv"};
    move.keywords = {"mv", "move", "rename"};
    move.related = {"rename-folder"};
    move.content = "MV-CONTENT\n\n```bash\nmv SOURCE DESTINATION\n```\n";
    writeEntry(root, "commands/mv", move);

    auto chmod = entrySpec("chmod", "chmod - change permissions", "command");
    chmod.command = "chmod";
    chmod.aliases = {"chmod"};
    chmod.keywords = {"chmod", "permission", "permissions", "rights"};
    chmod.related = {"chmod-755"};
    chmod.content = "CHMOD-CONTENT\n";
    writeEntry(root, "commands/chmod", chmod);

    auto chmod755 = entrySpec("chmod-755", "Explain chmod 755", "explain");
    chmod755.command = "chmod";
    chmod755.aliases = {"chmod 755", "make file executable"};
    chmod755.keywords = {"chmod", "755", "executable", "permission"};
    chmod755.related = {"chmod"};
    chmod755.intents = {"explain"};
    chmod755.content = "CHMOD-755-CONTENT: owner rwx, group rx, other rx.\n";
    writeEntry(root, "topics/chmod-755", chmod755);

    auto remove = entrySpec("delete-folder", "Delete a folder");
    remove.command = "rm";
    remove.aliases = {"delete folder"};
    remove.keywords = {"delete", "folder"};
    remove.intents = {"how_to"};
    remove.content = "DELETE-FOLDER-CONTENT\n";
    writeEntry(root, "topics/delete-folder", remove);

    auto network = entrySpec("network-status", "Inspect network status");
    network.command = "ip";
    network.aliases = {"network status"};
    network.keywords = {"network", "status", "ip"};
    network.content = "NETWORK-CONTENT\n";
    writeEntry(root, "extras/admin/network-status", network);
}

inline void writeTieKnowledge(const std::filesystem::path& root) {
    auto beta = entrySpec("beta", "Shared entry");
    beta.aliases = {"sharedtoken"};
    beta.keywords = {"sharedtoken"};
    beta.content = "BETA-CONTENT\n";
    writeEntry(root, "topics/beta", beta);

    auto alpha = entrySpec("alpha", "Shared entry");
    alpha.aliases = {"sharedtoken"};
    alpha.keywords = {"sharedtoken"};
    alpha.content = "ALPHA-CONTENT\n";
    writeEntry(root, "topics/alpha", alpha);
}

}  // namespace hltest
