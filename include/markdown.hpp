#pragma once

#include <iosfwd>
#include <string_view>

namespace howlinux {

// Render the knowledge-authoring Markdown subset without running external tools.
void renderMarkdown(std::ostream& output, std::string_view markdown, bool color = false);
bool terminalColorEnabled(std::ostream& output);

}  // namespace howlinux
