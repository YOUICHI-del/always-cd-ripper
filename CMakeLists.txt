cmake_minimum_required(VERSION 3.20)
project(AlwaysCDRipper VERSION 1.0.0 LANGUAGES CXX)

# --- ビルド番号自動生成 (YYYYMMDD-HHMM) ---
string(TIMESTAMP BUILD_NUMBER "%Y%m%d-%H%M")
add_compile_definitions(BUILD_NUMBER="${BUILD_NUMBER}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# --- Qt6 ---
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

# --- vcpkg 経由ライブラリ ---
find_package(FLAC REQUIRED)
find_package(taglib REQUIRED)

# --- ソースファイル ---
set(SOURCES
    src/main.cpp

    src/ripper/CdDrive.h
    src/ripper/CdDrive.cpp
    src/ripper/ParanoiaReader.h
    src/ripper/ParanoiaReader.cpp
    src/ripper/FlacEncoder.h
    src/ripper/FlacEncoder.cpp
    src/ripper/RipWorker.h
    src/ripper/RipWorker.cpp

    src/metadata/MusicBrainz.h
    src/metadata/MusicBrainz.cpp
    src/metadata/GnuDb.h
    src/metadata/GnuDb.cpp
    src/metadata/CoverArt.h
    src/metadata/CoverArt.cpp
    src/metadata/TagWriter.h
    src/metadata/TagWriter.cpp

    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/SectorMap.h
    src/ui/SectorMap.cpp
    src/ui/MetadataEditor.h
    src/ui/MetadataEditor.cpp
    src/ui/LogViewer.h
    src/ui/LogViewer.cpp
)

# --- Windowsリソース ---
if(WIN32)
    list(APPEND SOURCES resources/app.rc)
endif()

add_executable(AlwaysCDRipper ${SOURCES})

target_include_directories(AlwaysCDRipper PRIVATE
    src
    ${TAGLIB_INCLUDE_DIRS}
)

target_link_libraries(AlwaysCDRipper PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    FLAC::FLAC
    TagLib::TagLib
)

# --- Windows: コンソール非表示 ---
if(WIN32)
    set_target_properties(AlwaysCDRipper PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(AlwaysCDRipper PRIVATE winmm winhttp)
    target_include_directories(AlwaysCDRipper PRIVATE
        src
        ${TAGLIB_INCLUDE_DIRS}
    )
endif()

install(TARGETS AlwaysCDRipper RUNTIME DESTINATION bin)
install(FILES resources/icon.ico DESTINATION bin)

find_package(Qt6 COMPONENTS Concurrent REQUIRED)
target_link_libraries(AlwaysCDRipper PRIVATE Qt6::Concurrent)
