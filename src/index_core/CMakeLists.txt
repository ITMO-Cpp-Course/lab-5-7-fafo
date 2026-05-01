add_library(index_core STATIC
    Document.cpp
    DocumentBuilder.cpp
    InvertedIndex.cpp
)

target_link_libraries(index_lib PRIVATE project_warnings)
if (TARGET project_sanitizers)
    target_link_libraries(index_lib PRIVATE project_sanitizers)
endif()

target_include_directories(index_core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)