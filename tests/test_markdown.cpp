#include "test_harness.hpp"
#include "markdown.hpp"
#include "render.hpp"

#include <sstream>
#include <string>

using namespace howlinux;

namespace {
std::string render(std::string_view source, bool color = false) {
    std::ostringstream output;
    renderMarkdown(output, source, color);
    return output.str();
}
}  // namespace

HL_TEST(markdown_renders_headings_lists_emphasis_quotes_and_links) {
    const auto output = render("# Heading\n\n## Steps\n- **Important**: use `FILE_NAME`.\n"
        "1. Read *carefully*.\n> Keep a backup.\n[Manual](https://example.com/a_(b))\n");
    HL_REQUIRE_CONTAINS(output, "Heading\n=======\n");
    HL_REQUIRE_CONTAINS(output, "Steps\n-----\n");
    HL_REQUIRE_CONTAINS(output, "  - Important: use FILE_NAME.");
    HL_REQUIRE_CONTAINS(output, "1. Read carefully.");
    HL_REQUIRE_CONTAINS(output, "  | Keep a backup.");
    HL_REQUIRE_CONTAINS(output, "Manual (https://example.com/a_(b))");
    HL_REQUIRE(output.find('\x1b') == std::string::npos);
}

HL_TEST(markdown_keeps_shell_syntax_literal_inside_code_blocks) {
    const std::string code = "printf '%s\\n' \"$FILE_NAME\"\n"
        "# comment **not bold**\ncat <<'EOF'\n`uname` > output\nEOF\n";
    const auto output = render("```bash\n" + code + "```\nAfter\n");
    HL_REQUIRE_CONTAINS(output, "    printf '%s\\n' \"$FILE_NAME\"\n");
    HL_REQUIRE_CONTAINS(output, "    # comment **not bold**\n");
    HL_REQUIRE_CONTAINS(output, "    `uname` > output\n");
    HL_REQUIRE_CONTAINS(output, "    EOF\nAfter\n");
    HL_REQUIRE(output.find("```") == std::string::npos);
}

HL_TEST(markdown_handles_fence_types_lengths_crlf_and_missing_final_newline) {
    HL_REQUIRE_EQ(render("~~~sh\r\necho hello\r\n~~~\r\nDone"),
                  "    echo hello\nDone\n");
    HL_REQUIRE_EQ(render("````markdown\n```bash\n~~~\n````\n"),
                  "    ```bash\n    ~~~\n");
    HL_REQUIRE_EQ(render("  ```bash\n  echo one\n    echo two\n  ```\n"),
                  "    echo one\n      echo two\n");
    HL_REQUIRE_EQ(render("```sh\necho hello"), "    echo hello\n");
    HL_REQUIRE_EQ(render("```sh\necho hello\n``` trailing\n```\n"),
                  "    echo hello\n    ``` trailing\n");
}

HL_TEST(markdown_preserves_unmatched_markup_underscores_and_code_spans) {
    HL_REQUIRE_EQ(render("FILE_NAME some_long_name `unclosed **unfinished"),
                  "FILE_NAME some_long_name `unclosed **unfinished\n");
    HL_REQUIRE_EQ(render("Use ``echo `uname` **literal**`` and \\*literal\\*."),
                  "Use echo `uname` **literal** and *literal*.\n");
    HL_REQUIRE_EQ(render("    # code\n    echo **literal**\n"),
                  "    # code\n    echo **literal**\n");
    HL_REQUIRE_EQ(render("Größe und Grüße"), "Größe und Grüße\n");
    HL_REQUIRE_EQ(render("Use `printf\nhello` and **read\ncarefully**.\n"),
                  "Use printf hello and read\ncarefully.\n");
    HL_REQUIRE_EQ(render("\t# code\n\techo **literal**\n"),
                  "\t# code\n\techo **literal**\n");
}

HL_TEST(markdown_escapes_terminal_controls_and_emits_only_requested_styles) {
    HL_REQUIRE_EQ(render("hello\x1b[2J\a"), "hello\\x1B[2J\\x07\n");
    const auto colored = render("# Title\n```bash\necho hello\n```\n", true);
    HL_REQUIRE_CONTAINS(colored, "\x1b[1;36mTitle\x1b[0m");
    HL_REQUIRE_CONTAINS(colored, "    \x1b[36mecho hello\x1b[0m");
    HL_REQUIRE(colored.find("```bash") == std::string::npos);
    std::ostringstream captured;
    HL_REQUIRE(!terminalColorEnabled(captured));
}

HL_TEST(entry_rendering_removes_duplicate_title_and_raw_preserves_source) {
    KnowledgeEntry entry;
    entry.id = "example";
    entry.title = "Example";
    entry.content = "# Example\r\n\r\n```bash\r\necho hello\r\n```";
    KnowledgeBase knowledge;
    std::ostringstream formatted;
    Renderer::entry(formatted, entry, knowledge);
    HL_REQUIRE_EQ(formatted.str(), "Example\nID: example\n\n    echo hello\n");
    std::ostringstream raw;
    Renderer::entry(raw, entry, knowledge, true);
    HL_REQUIRE_CONTAINS(raw.str(), entry.content);
    std::ostringstream json;
    Renderer::entryJson(json, entry, knowledge);
    HL_REQUIRE_CONTAINS(json.str(), escapeJson(entry.content));
}
