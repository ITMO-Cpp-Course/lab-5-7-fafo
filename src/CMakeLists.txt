add_library(index_core STATIC
    index_core/Document.cpp
    index_core/DocumentBuilder.cpp
    index_core/InvertedIndex.cpp
)

target_link_libraries(index_core PRIVATE project_warnings)
if (TARGET project_sanitizers)
    target_link_libraries(index_core PRIVATE project_sanitizers)
endif()

target_include_directories(index_core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)