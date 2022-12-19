# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
# file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")
set(HEADER_LIST
        include/fluent/fluent.hpp 
        include/fluent/bundle.hpp 
        include/fluent/resource.hpp 
        include/fluent/types.hpp 
        include/fluent/ast.hpp)

# Make an automatic library - will be static or dynamic based on user setting
add_library(fluent
        ${HEADER_LIST}
        src/fluent.cpp 
        src/resource.cpp)

# We need this directory, and users of our library will need it too
target_include_directories(fluent PUBLIC include)

# target_link_libraries
target_link_libraries(fluent
        PRIVATE fmt::fmt
        PUBLIC spdlog::spdlog
        PRIVATE foonathan::lexy)

# All users of this library will need at least C++11
target_compile_features(fluent PUBLIC cxx_std_11)
