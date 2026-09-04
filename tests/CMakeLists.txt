add_executable(howlinux_tests
    test_main.cpp
    test_query_concepts.cpp
    test_knowledge_search.cpp
    test_cli_app.cpp
)

target_link_libraries(howlinux_tests PRIVATE howlinux_core)
target_compile_features(howlinux_tests PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(howlinux_tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(howlinux_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

add_test(NAME howlinux.v1 COMMAND howlinux_tests)
set_tests_properties(howlinux.v1 PROPERTIES TIMEOUT 30)

add_test(
    NAME howlinux.smoke
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/smoke.sh
            $<TARGET_FILE:howlinux> ${PROJECT_SOURCE_DIR}/knowledge
)
set_tests_properties(howlinux.smoke PROPERTIES TIMEOUT 30)

add_test(
    NAME howlinux.ranking-evaluation
    COMMAND bash ${PROJECT_SOURCE_DIR}/scripts/evaluate-ranking.sh
            $<TARGET_FILE:howlinux> ${PROJECT_SOURCE_DIR}/knowledge
            ${CMAKE_CURRENT_SOURCE_DIR}/data/ranking-v2.tsv
)
set_tests_properties(howlinux.ranking-evaluation PROPERTIES TIMEOUT 30)
