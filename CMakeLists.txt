cmake_minimum_required(VERSION 3.5)

# Assicurati che il toolchain file sia caricato correttamente
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif()
endif()

project(OmniBook)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "OmniBook")
set(VITA_TITLEID "OMBK00001") # TitleID a 9 caratteri
set(VITA_VERSION "01.01")

add_definitions(-DFMT_HEADER_ONLY)
add_definitions(-DAPP_VERSION="${VITA_VERSION}")
add_definitions(-DIMGUI_ENABLE_FREETYPE) 
add_definitions(-DIMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS)

# DISATTIVAZIONE LTO E FLAG DI COMPILAZIONE (Importante -fpermissive per GCC 15)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-lto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-lto")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-lto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -mtune=cortex-a9 -mfpu=neon -Wall -Wno-psabi -Wno-unused-function -fno-rtti -std=gnu++17 -fpermissive")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")

include_directories(
    include
    libs
    libs/imgui
    libs/imgui/backends
    ${VITASDK}/arm-vita-eabi/include/freetype2/
    ${VITASDK}/arm-vita-eabi/include/SDL2/ 
    ${VITASDK}/arm-vita-eabi/include/
)

link_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/libs/lib
)

add_executable(${PROJECT_NAME}
    # ImGui Core
    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/imgui_widgets.cpp
    libs/imgui/misc/freetype/imgui_freetype.cpp
    
    # ImGui Backends (Ponte SDL2 + vitaGL)
    libs/imgui/backends/imgui_impl_sdl2.cpp
    libs/imgui/backends/imgui_impl_sdlrenderer2.cpp   # <--- Usa questo al posto di opengl2

    # Sorgenti Progetto
    src/main.cpp
    src/ClassHome.cpp
    src/ClassDocument.cpp
    src/ClassDocumentView.cpp
    src/ClassFileManager.cpp
    src/OmniBook.cpp
    src/FileManagerUtils.cpp
    src/ClassLoadingScreen.cpp
)

target_link_libraries(${PROJECT_NAME}
    SDL2_image
    webpdemux    # <--- AGGIUNGI QUESTO
    webp
    SDL2
    mupdf
    mupdf-third
    harfbuzz
    freetype
    jpeg
    png
    z
    m
    
    # MODULI DI SISTEMA VITA (STUBS) - Questi risolvono gli ultimi errori
    SceAppUtil_stub      # Risolve sceAppUtilInit e Browser
    SceAudioIn_stub      # Risolve sceAudioInInput
    SceMotion_stub       # Risolve sceMotionGetSensorState
    ScePower_stub        # Spesso richiesto da SDL2
    SceTouch_stub
    SceGxm_stub
    SceDisplay_stub
    SceCtrl_stub
    SceAudio_stub
    SceHid_stub
    SceNet_stub
    SceNetCtl_stub
    SceSsl_stub
    SceHttp_stub
    SceAppMgr_stub
    SceCommonDialog_stub
    SceSysmodule_stub
    pthread
)
# Creazione VPK
vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE sce_sys/icon0.png sce_sys/icon0.png
    FILE sce_sys/pic0.png sce_sys/pic0.png
    FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
    FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
    FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png

    FILE res/texture/logo_home.png res/texture/logo_home.png

    FILE res/font/DroidSans.ttf res/font/DroidSans.ttf

    FILE res/icons/wifi_24.png res/icons/wifi_24.png
    FILE res/icons/battery_24.png res/icons/battery_24.png
)