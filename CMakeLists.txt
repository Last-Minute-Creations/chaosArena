cmake_minimum_required(VERSION 3.14.0)
project(chaosArena LANGUAGES C)

if(NOT AMIGA)
	message(SEND_ERROR "This project only compiles for Amiga")
endif()

# ACE
add_subdirectory(deps/ace ace)

set(CMAKE_C_STANDARD 11)
file(GLOB_RECURSE SOURCES src/*.c src/*.h)

if(ELF2HUNK)
	set(GAME_EXECUTABLE chaosArena.elf)
	set(GAME_OUTPUT_EXECUTABLE chaosArena.exe)
	add_executable(${GAME_EXECUTABLE} ${SOURCES})
	target_link_libraries(${GAME_EXECUTABLE} -Wl,-Map=chaosArena.map)

	if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug") AND NOT (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		set(ELF2HUNK_ARGS ${ELF2HUNK_ARGS} -s)
	endif()

	add_custom_command(
		TARGET ${GAME_EXECUTABLE} POST_BUILD
		COMMAND ${ELF2HUNK} ${GAME_EXECUTABLE} ${GAME_OUTPUT_EXECUTABLE} ${ELF2HUNK_ARGS}
	)
	add_custom_command(
		TARGET ${GAME_EXECUTABLE} POST_BUILD
		COMMAND ${OBJDUMP} --disassemble -S ${GAME_EXECUTABLE} > chaosArena.s
	)
else()
	SET(GAME_EXECUTABLE chaosArena)
	SET(GAME_OUTPUT_EXECUTABLE chaosArena)
	add_executable(${GAME_EXECUTABLE} ${SOURCES})
endif()

# Version stuff
string(TIMESTAMP YEAR "%y")
string(TIMESTAMP DAY "%d")
string(TIMESTAMP MONTH "%m")
MATH(EXPR VER_MAJOR "0 + ${YEAR}")
MATH(EXPR VER_MINOR "0 + ${MONTH}")
MATH(EXPR VER_FIX "0 + ${DAY}")
set(VERSION "${VER_MAJOR}.${VER_MINOR}.${VER_FIX}")
target_compile_definitions(${GAME_EXECUTABLE} PRIVATE GAME_VERSION="${VERSION}")

target_include_directories(${GAME_EXECUTABLE} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_options(${GAME_EXECUTABLE} PUBLIC -Wall)
target_link_libraries(${GAME_EXECUTABLE} ace)
if(GAME_DEBUG)
	target_compile_definitions(${GAME_EXECUTABLE} PRIVATE GAME_DEBUG)
	target_compile_definitions(ace PUBLIC ACE_DEBUG_UAE)
endif()

set(RES_DIR ${CMAKE_CURRENT_LIST_DIR}/res)
set(DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/data)
set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(WARRIOR_DIR ${GEN_DIR}/warrior)
file(MAKE_DIRECTORY ${DATA_DIR} ${GEN_DIR} ${WARRIOR_DIR})

# Assets
SET(X_SPACING 16)
SET(Y_SPACING 13)
SET(FRAME_WIDTH 16)
SET(FRAME_HEIGHT 19)
foreach(direction RANGE 7)
	SET(warriorFrames "")
	file(MAKE_DIRECTORY ${WARRIOR_DIR})
	foreach(anim RANGE 11)
		MATH(EXPR posX "8 + ${anim} * (${FRAME_WIDTH} + ${X_SPACING})")
		MATH(EXPR posY "5 + ${direction} * (${FRAME_HEIGHT} + ${Y_SPACING})")
		MATH(EXPR frameIndex "12 * ${direction} + ${anim}")
		extractBitmaps(
			TARGET ${GAME_EXECUTABLE} SOURCE ${RES_DIR}/warrior.png
			DESTINATIONS
				${WARRIOR_DIR}/${frameIndex}.png ${posX} ${posY} ${FRAME_WIDTH} ${FRAME_HEIGHT}
			GENERATED_FILE_LIST warriorFrame
		)
		SET(warriorFrames "${warriorFrames};${warriorFrame}")
	endforeach()
endforeach()

convertTileset(
	TARGET ${GAME_EXECUTABLE} SIZE 16 HEIGHT 19
	SOURCE ${WARRIOR_DIR} DESTINATION ${GEN_DIR}/warrior.png
	TILE_PATHS ${warriorFrames}
)
convertBitmaps(
	TARGET ${GAME_EXECUTABLE}
	PALETTE ${RES_DIR}/chaos_arena.gpl
	MASK_COLOR "#FF00FF"
	INTERLEAVED SOURCES ${GEN_DIR}/warrior.png
	DESTINATIONS ${DATA_DIR}/warrior.bm
	MASKS ${DATA_DIR}/warrior_mask.bm
)

convertPalette(
	${GAME_EXECUTABLE} ${RES_DIR}/chaos_arena.gpl ${DATA_DIR}/palette.plt
)

# Generating ZIP
set(GAME_ZIP_NAME "${CMAKE_PROJECT_NAME} ${VER_MAJOR}_${VER_MINOR}_${VER_FIX}")
add_custom_target(generateZip COMMAND
	${CMAKE_COMMAND} -E tar "cf" "${GAME_ZIP_NAME}.zip" --format=zip
	"${CMAKE_CURRENT_BINARY_DIR}/${GAME_OUTPUT_EXECUTABLE}" "${DATA_DIR}"
	COMMENT "Generating ${GAME_ZIP_NAME}.zip"
)

# Generating ADF
set(ADF_DIR "${CMAKE_CURRENT_BINARY_DIR}/adf")
add_custom_target(generateAdf
	COMMAND ${CMAKE_COMMAND} -E make_directory "${ADF_DIR}/s"
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${GAME_OUTPUT_EXECUTABLE}" "${ADF_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${DATA_DIR}" "${ADF_DIR}/data"
	COMMAND ${CMAKE_COMMAND} -E echo "${GAME_OUTPUT_EXECUTABLE}" > "${ADF_DIR}/s/startup-sequence"
	COMMAND exe2adf -l ${CMAKE_PROJECT_NAME} -a "${CMAKE_PROJECT_NAME}.adf" -d ${ADF_DIR}
	COMMAND ${CMAKE_COMMAND} -E rm -rf "${ADF_DIR}"
	COMMENT "Generating ADF file"
)