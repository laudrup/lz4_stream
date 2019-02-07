ExternalProject_Add(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG v2.6.0
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Get_property(Catch2 SOURCE_DIR)

add_library(catch2 INTERFACE)
target_include_directories(catch2 INTERFACE ${SOURCE_DIR}/single_include/catch2)
add_dependencies(catch2 Catch2)
