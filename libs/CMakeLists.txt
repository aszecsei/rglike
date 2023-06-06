# C++ LIBRARIES:

# rglike: The game library
add_subdirectory(rglike)

# RUST LIBRARIES:

# fluency: A C++ FFI wrapper around ProjectFluent
corrosion_import_crate(MANIFEST_PATH fluency/Cargo.toml)
target_include_directories(fluency INTERFACE fluency/include)

message(STATUS "fluency include directories: ${INTERFACE_INCLUDE_DIRECTORIES}")