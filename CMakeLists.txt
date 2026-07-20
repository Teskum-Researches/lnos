cmake_minimum_required(VERSION 3.16)

project(lnos)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LANGUAGES "ru")
set(TEXTDOMAIN "lnos")

find_package(PkgConfig REQUIRED)
find_package(Intl REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

find_program(MSGFMT_EXECUTABLE msgfmt REQUIRED)

foreach(LANG IN LISTS LANGUAGES)
        set(PO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/po/${LANG}.po")
        set(MO_DIR "${CMAKE_BINARY_DIR}/locale/${LANG}/LC_MESSAGES")
        set(MO_FILE "${MO_DIR}/${TEXTDOMAIN}.mo")

        if(EXISTS ${PO_FILE})
                add_custom_command(
                        OUTPUT ${MO_FILE}
                        COMMAND ${CMAKE_COMMAND} -E make_directory ${MO_DIR}
                        COMMAND ${MSGFMT_EXECUTABLE} -o ${MO_FILE} ${PO_FILE}
                        DEPENDS ${PO_FILE}
                        COMMENT "Compiling translation ${LANG}.po to ${TEXTDOMAIN}.mo and saving to ${MO_DIR}"
                )
                list(APPEND MO_FILES ${MO_FILE})

                install(FILES ${MO_FILE}
                        DESTINATION "share/locale/${LANG}/LC_MESSAGES"
                )
        endif()
endforeach()

add_custom_target(translations ALL DEPENDS ${MO_FILES})

#
# liblnos
#
add_library(lnos
        liblnos/src/config.cpp
        liblnos/src/crypto.cpp
)

add_library(lnos::lnos ALIAS lnos)

target_include_directories(lnos
        PUBLIC
        liblnos/include
        ${SODIUM_INCLUDE_DIRS}
)

target_link_libraries(lnos
        PUBLIC
        ${SODIUM_LIBRARIES}
)


#
# lnosd
#
add_executable(lnosd
        lnosd/src/main.cpp
        lnosd/src/registry.cpp
)

target_include_directories(lnosd
        PRIVATE
        lnosd/include
)

target_link_libraries(lnosd
        PRIVATE
        lnos::lnos
)


#
# lnosctl
#
add_executable(lnosctl
        lnosctl/src/main.cpp
)

target_link_libraries(lnosctl
        PRIVATE
        lnos::lnos
)
