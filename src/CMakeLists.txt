set(NAME "watcher")
project(${NAME})
 
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.hpp *.c *.h *.rc)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${SOURCE_FILES})

set(Win32Executable)
if (WIN32)
    set(Win32Executable WIN32)

#    set(ApplicationIcon ${CMAKE_SOURCE_DIR}/Rabbithole/resources/Icon.ico)
#    file(TO_NATIVE_PATH "${ApplicationIcon}" ApplicationIcon)
#    string(REPLACE "\\" "\\\\" ApplicationIcon "${ApplicationIcon}")
#    configure_file(
#        ${CMAKE_SOURCE_DIR}/Rabbithole/resources/Resource.rc.in
#        ${CMAKE_CURRENT_BINARY_DIR}/Resource.rc
#    )
#    set(Resources
#        ${CMAKE_CURRENT_BINARY_DIR}/Resource.rc
#    )
endif()

find_package(CURL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

include_directories(
  "."
  "ext"
  ${CURL_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
  ${SDL2_IMAGE_INCLUDE_DIRS}
)

link_directories(
  ${CURL_LIBRARY_DIRS}
  ${SDL2_LIBRARY_DIRS}
  ${SDL2_IMAGE_LIBRARY_DIRS}
)

link_libraries(
  curl
  SDL2main
  SDL2
  SDL2_image
)

add_executable(${NAME} ${Win32Executable} ${SOURCE_FILES} ${RESOURCES})

set(OUTPUT_BIN_DIR ${CMAKE_CURRENT_LIST_DIR}/../bin)

set_target_properties(${NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY                "${OUTPUT_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${OUTPUT_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${OUTPUT_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${OUTPUT_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${OUTPUT_BIN_DIR}"
    DEBUG_POSTFIX                           _d
    RELWITHDEBINGO_POSTFIX                  _rd
    MINSIZEREL_POSTFIX                      _r
    VS_DEBUGGER_WORKING_DIRECTORY           ${OUTPUT_BIN_DIR}
)

if(WIN32)
    target_compile_definitions(${NAME} PRIVATE UNICODE _UNICODE _HASEXCEPTIONS=0 CURL_STATICLIB)
    target_link_libraries(${NAME} PRIVATE Opengl32 glu32 ws2_32 libssl64MT libcrypto64MT crypt32)
else()
    target_link_libraries(${NAME} PRIVATE GL pthread ${CMAKE_DL_LIBS})
endif()