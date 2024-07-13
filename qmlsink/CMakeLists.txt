cmake_minimum_required(VERSION 3.22)

# Check for pkg-config
include(FindPkgConfig)
if(NOT PKG_CONFIG_FOUND)
  message(FATAL_ERROR "pkg-config is required. Please install it.")
endif()

# Check for GStreamer packages
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0>=1.20 gstreamer-video-1.0>=1.20)

# Find Qt5 packages
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

# Set source files
set(SRC_LIST main.cpp)
qt5_add_resources(RESOURCES qmlsink.qrc)

# Include and link directories
include_directories(${GSTREAMER_INCLUDE_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})

# Add executable
add_executable(qml-example ${SRC_LIST} ${RESOURCES})

# Link libraries
target_link_libraries(qml-example ${GSTREAMER_LIBRARIES} Qt5::Core Qt5::Widgets Qt5::Qml Qt5::Quick)
