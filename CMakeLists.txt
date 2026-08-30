cmake_minimum_required(VERSION 3.16)

project(
    howlinux
    VERSION 1.0.0
    DESCRIPTION "Deterministic offline Linux knowledge CLI"
    LANGUAGES CXX
)

include(CTest)
include(GNUInstallDirs)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(yaml-cpp REQUIRED)

add_library(howlinux_core
    src/app.cpp
    src/cli.cpp
    src/concepts.cpp
    src/index.cpp
    src/knowledge.cpp
    src/query.cpp
    src/render.cpp
    src/search.cpp
)

target_include_directories(howlinux_core PUBLIC include)
target_link_libraries(howlinux_core PUBLIC yaml-cpp)
target_compile_definitions(
    howlinux_core
    PUBLIC
        HOWLINUX_VERSION="${PROJECT_VERSION}"
        HOWLINUX_INSTALL_DATADIR="${CMAKE_INSTALL_DATADIR}"
)

function(howlinux_enable_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(
            ${target_name}
            PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow
        )
    endif()
endfunction()

howlinux_enable_warnings(howlinux_core)

add_executable(howlinux src/main.cpp)
target_link_libraries(howlinux PRIVATE howlinux_core)
howlinux_enable_warnings(howlinux)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

install(TARGETS howlinux RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY knowledge/ DESTINATION ${CMAKE_INSTALL_DATADIR}/howlinux/knowledge)
install(FILES README.md requirements.md CONTRIBUTING.md SECURITY.md LICENSE
        DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY docs/ DESTINATION ${CMAKE_INSTALL_DOCDIR}/docs)

# Shell integration and local documentation are installed with the binary so
# the source tree is not needed after installation.
install(FILES completions/howlinux.bash
        DESTINATION ${CMAKE_INSTALL_DATADIR}/bash-completion/completions
        RENAME howlinux)
install(FILES completions/_howlinux.zsh
        DESTINATION ${CMAKE_INSTALL_DATADIR}/zsh/site-functions)
install(FILES completions/howlinux.fish
        DESTINATION ${CMAKE_INSTALL_DATADIR}/fish/vendor_completions.d)
install(FILES docs/howlinux.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

set(CPACK_PACKAGE_NAME "howlinux")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Deterministic offline Linux knowledge CLI")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_FILE_NAME "howlinux-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
