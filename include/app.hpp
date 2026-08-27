#pragma once

#include <filesystem>
#include <iosfwd>
#include <string>
#include <vector>

namespace howlinux {

int runApplication(const std::vector<std::string>& arguments,
                   const std::filesystem::path& executable_path,
                   const std::filesystem::path& current_directory,
                   std::ostream& output,
                   std::ostream& error);

}  // namespace howlinux
