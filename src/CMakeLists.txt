set(NAME "watcher")
project(${NAME})

set(SourceFiles
	atlas/atlas.cpp
	atlas/atlas.h
	atlas/tile.cpp
	atlas/tile.h
	atlas/tile_streamer.cpp
	atlas/tile_streamer.h
	database/database.cpp
	database/database.h
	database/prepared_statement.cpp
	database/prepared_statement.h
	database/query_result.cpp
	database/query_result.h
	network/network.cpp
	network/network.h
	network/network_linux.cpp
	network/network_windows.cpp
	camera.h
	camerarep.cpp
	camerarep.h
	configuration.cpp
	configuration.h
	filesystem.cpp
	filesystem.h
	geolocationdata.cpp
	geolocation.h
	log.cpp
	log.h
	main.cpp
	plugin.h
	plugin_manager.cpp
	plugin_manager.h
	render.h
	texture_loader.cpp
	texture_loader.h
	watcher.cpp
	watcher.h
)

source_group("" FILES 
	camera.h
	camerarep.cpp
	camerarep.h
	configuration.cpp
	configuration.h
	filesystem.cpp
	filesystem.h
	geolocationdata.cpp
	geolocation.h
	log.cpp
	log.h
	main.cpp
	plugin.h
	plugin_manager.cpp
	plugin_manager.h
	render.h
	texture_loader.cpp
	texture_loader.h
	watcher.cpp
	watcher.h
)

source_group("atlas" FILES 
	atlas/atlas.cpp
	atlas/atlas.h
	atlas/tile.cpp
	atlas/tile.h
	atlas/tile_streamer.cpp
	atlas/tile_streamer.h
)

source_group("database" FILES
	database/database.cpp
	database/database.h
	database/prepared_statement.cpp
	database/prepared_statement.h
	database/query_result.cpp
	database/query_result.h
)

source_group("network" FILES
	network/network.cpp
	network/network.h
	network/network_linux.cpp
	network/network_windows.cpp
)

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

#source_group("" FILES ${Resources})

#add_executable(${NAME} ${Win32Executable} ${SourceFiles} ${Resources})

add_executable(${NAME} ${Win32Executable} ${SourceFiles})

find_package(imgui REQUIRED)
target_link_libraries(${NAME} PRIVATE imgui)

set(OUTPUT_BIN_DIR ${CMAKE_BINARY_DIR}/bin)

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

add_custom_command(
    TARGET ${NAME}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ARGS ${OUTPUT_BIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ARGS ${CMAKE_SOURCE_DIR}/Data ${OUTPUT_BIN_DIR}/Data
)
