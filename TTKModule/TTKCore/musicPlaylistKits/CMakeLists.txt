cmake_minimum_required(VERSION 2.8.11)

set_property(GLOBAL PROPERTY MUSIC_CORE_PLAYLIST_KITS_HEADERS
    ${MUSIC_CORE_PLAYLIST_DIR}/musicasxconfigmanager.h
    ${MUSIC_CORE_PLAYLIST_DIR}/musicdbplconfigmanager.h
    ${MUSIC_CORE_PLAYLIST_DIR}/musicfplconfigmanager.h
    ${MUSIC_CORE_PLAYLIST_DIR}/musicm3uconfigmanager.h
    ${MUSIC_CORE_PLAYLIST_DIR}/musicplaylistinterface.h
    ${MUSIC_CORE_PLAYLIST_DIR}/musicplsconfigmanager.h
    ${MUSIC_CORE_PLAYLIST_DIR}/musictkplconfigmanager.h
    ${MUSIC_CORE_PLAYLIST_DIR}/musicwplconfigmanager.h
    ${MUSIC_CORE_PLAYLIST_DIR}/musicxspfconfigmanager.h
    ${MUSIC_CORE_PLAYLIST_DIR}/musiccsvconfigmanager.h
    ${MUSIC_CORE_PLAYLIST_DIR}/musictxtconfigmanager.h
  )

set_property(GLOBAL PROPERTY MUSIC_CORE_PLAYLIST_KITS_SOURCES
    ${MUSIC_CORE_PLAYLIST_DIR}/musicasxconfigmanager.cpp
    ${MUSIC_CORE_PLAYLIST_DIR}/musicdbplconfigmanager.cpp
    ${MUSIC_CORE_PLAYLIST_DIR}/musicfplconfigmanager.cpp
    ${MUSIC_CORE_PLAYLIST_DIR}/musicm3uconfigmanager.cpp
    ${MUSIC_CORE_PLAYLIST_DIR}/musicplsconfigmanager.cpp
    ${MUSIC_CORE_PLAYLIST_DIR}/musictkplconfigmanager.cpp
    ${MUSIC_CORE_PLAYLIST_DIR}/musicwplconfigmanager.cpp
    ${MUSIC_CORE_PLAYLIST_DIR}/musicxspfconfigmanager.cpp
    ${MUSIC_CORE_PLAYLIST_DIR}/musiccsvconfigmanager.cpp
    ${MUSIC_CORE_PLAYLIST_DIR}/musictxtconfigmanager.cpp
  )
  
