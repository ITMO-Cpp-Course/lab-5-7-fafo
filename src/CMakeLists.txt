add_library(index_core STATIC
        index_core/Document.cpp
        index_core/DocumentBuilder.cpp
        index_core/InvertedIndex.cpp
        index_core/IndexStore.cpp
        index_core/UpdateTransaction.cpp
)

target_link_libraries(index_core PRIVATE project_warnings)

if (TARGET project_sanitizers)
    target_link_libraries(index_core PRIVATE project_sanitizers)
endif ()

target_include_directories(index_core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/index_core
)

set_target_properties(index_core PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
)

add_library(resource_core ALIAS index_core)