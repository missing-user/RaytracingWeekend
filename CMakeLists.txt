cmake_minimum_required(VERSION 3.12)

project(RaytracingWeekend)

set(CMAKE_CXX_STANDARD 20)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/RaytracingWeekend/*.cpp"
                  "${CMAKE_SOURCE_DIR}/RaytracingWeekend/*.h")
                  
# Set the build configurations
set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)

# Find and include SFML
find_package(SFML COMPONENTS graphics REQUIRED)
include_directories(${SFML_INCLUDE_DIR})

# Find and include GLM
find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})
# target_link_libraries(<your executable> glm::glm)

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_options(${PROJECT_NAME} PRIVATE -Ofast -march=native)


# Link against SFML
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics)

# Set the build configurations for the executable
set_target_properties(${PROJECT_NAME} PROPERTIES
    RELEASE_OUTPUT_NAME "${PROJECT_NAME}"
)

# Specify the output directory for the executable
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release"
)

# Set the project configurations
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
