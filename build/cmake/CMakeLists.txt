cmake_minimum_required(VERSION 3.30)

set(name carcockpit)
project(${name})

# !!! find_package must go after project() declaration !!!
# Otherwise VCPKG does not set the CMAKE_PREFIX_PATH to find packages.
find_package(myci CONFIG REQUIRED)

set(srcs)
myci_add_source_files(srcs
    DIRECTORY
        ../../src/${name}
    RECURSIVE
)

# TODO: remove when gltf stuff becomes part of ruis library
myci_add_source_files(srcs
    DIRECTORY
        ../../src/ruis
    RECURSIVE
)

myci_declare_application(${name}-opengles
    SOURCES
        ${srcs}
    RESOURCE_DIRECTORY
        ../../res
    INCLUDE_DIRECTORIES
        ../../src
    DEPENDENCIES
        clargs
        jsondom
        ruisapp::ruisapp-opengles
)

# myci_declare_application(${name}-opengl
#     SOURCES
#         ${srcs}
#     RESOURCE_DIRECTORY
#         ../../res
#     INCLUDE_DIRECTORIES
#         ../../src
#     DEPENDENCIES
#         clargs
#         jsondom
#         ruisapp::ruisapp-opengl
# )
