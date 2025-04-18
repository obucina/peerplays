# Defines Peerplays library target.
project( Peerplays )
cmake_minimum_required( VERSION 2.8.12 )

set( BLOCKCHAIN_NAME "Peerplays" )

set( CLI_CLIENT_EXECUTABLE_NAME graphene_client )
set( GUI_CLIENT_EXECUTABLE_NAME Peerplays )
set( CUSTOM_URL_SCHEME "gcs" )
set( INSTALLER_APP_ID "68ad7005-8eee-49c9-95ce-9eed97e5b347" )

# http://stackoverflow.com/a/18369825
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
        message(FATAL_ERROR "GCC version must be at least 4.8!")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.3)
        message(FATAL_ERROR "Clang version must be at least 3.3!")
    endif()
endif()

list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules" )

function(get_linux_lsb_release_information)
    find_program(LSB_RELEASE_EXEC lsb_release)
    if(NOT LSB_RELEASE_EXEC)
        message(FATAL_ERROR "Could not detect lsb_release executable, can not gather required information")
    endif()

    execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --id OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --release OUTPUT_VARIABLE LSB_RELEASE_VERSION_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --codename OUTPUT_VARIABLE LSB_RELEASE_CODENAME_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(LSB_RELEASE_ID_SHORT "${LSB_RELEASE_ID_SHORT}" PARENT_SCOPE)
    set(LSB_RELEASE_VERSION_SHORT "${LSB_RELEASE_VERSION_SHORT}" PARENT_SCOPE)
    set(LSB_RELEASE_CODENAME_SHORT "${LSB_RELEASE_CODENAME_SHORT}" PARENT_SCOPE)
endfunction()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        find_package(cppzmq)
        target_link_libraries(cppzmq)

	get_linux_lsb_release_information()
	message(STATUS "Linux ${LSB_RELEASE_ID_SHORT} ${LSB_RELEASE_VERSION_SHORT} ${LSB_RELEASE_CODENAME_SHORT}")
	string(REGEX MATCHALL "([0-9]+)" arg_list ${LSB_RELEASE_VERSION_SHORT})
	list( LENGTH arg_list listlen )
	if (NOT listlen)
		message(FATAL_ERROR "Could not detect Ubuntu version")
	endif()
	list(GET arg_list 0 output)
	message("Ubuntu version is: ${output}")
	add_definitions(-DPEERPLAYS_UBUNTU_VERSION=${output})
endif()

# function to help with cUrl
macro(FIND_CURL)
   if (NOT WIN32 AND NOT APPLE AND CURL_STATICLIB)
      find_package(OpenSSL REQUIRED)
      set (OLD_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
      set (CMAKE_FIND_LIBRARY_SUFFIXES .a)
      find_package(CURL REQUIRED)
      list(APPEND CURL_LIBRARIES ${OPENSSL_LIBRARIES} ${BOOST_THREAD_LIBRARY} ${CMAKE_DL_LIBS})
      set (CMAKE_FIND_LIBRARY_SUFFIXES ${OLD_SUFFIXES})
   else (NOT WIN32 AND NOT APPLE AND CURL_STATICLIB)
      find_package(CURL REQUIRED)
   endif (NOT WIN32 AND NOT APPLE AND CURL_STATICLIB)

   if( WIN32 )
     if ( MSVC )
       list( APPEND CURL_LIBRARIES Wldap32 )
     endif( MSVC )

     if( MINGW )
       # MinGW requires a specific order of included libraries ( CURL before ZLib )
       find_package( ZLIB REQUIRED )
       list( APPEND CURL_LIBRARIES ${ZLIB_LIBRARY} pthread )
     endif( MINGW )

     list( APPEND CURL_LIBRARIES ${PLATFORM_SPECIFIC_LIBS} )
   endif( WIN32 )
endmacro()

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

if (BUILD_PEERPLAYS_TESTNET)
   set(GRAPHENE_EGENESIS_JSON "${CMAKE_CURRENT_SOURCE_DIR}/genesis-testnet.json" CACHE PATH "location of the genesis.json to embed in the executable" )
   #add_compile_definitions(BUILD_PEERPLAYS_TESTNET=1)
   add_definitions(-DBUILD_PEERPLAYS_TESTNET=1)
   message ("\n====================\nBuilding for Testnet\n====================\n")
else (BUILD_PEERPLAYS_TESTNET)
   set(GRAPHENE_EGENESIS_JSON "${CMAKE_CURRENT_SOURCE_DIR}/genesis-mainnet.json" CACHE PATH "location of the genesis.json to embed in the executable" )
   message ("\n====================\nBuilding for Mainnet\n====================\n")
endif (BUILD_PEERPLAYS_TESTNET)

#set (ENABLE_INSTALLER 1)
#set (USE_PCH 1)

if (USE_PCH)
  include (cotire)
endif(USE_PCH)

IF( NOT WIN32 )
  list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libraries/fc/CMakeModules" )
ENDIF( NOT WIN32 )
list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libraries/fc/GitVersionGen" )
include( GetGitRevisionDescription )
get_git_head_revision( GIT_REFSPEC GIT_SHA2 )

SET(BOOST_COMPONENTS)
LIST(APPEND BOOST_COMPONENTS thread
                             iostreams
                             date_time
                             system
                             filesystem
                             program_options
                             serialization
                             chrono
                             unit_test_framework
                             context
                             locale)
SET( Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF" )

IF( WIN32 )
  SET(BOOST_ROOT $ENV{BOOST_ROOT})
  set(Boost_USE_MULTITHREADED ON)
  set(BOOST_ALL_DYN_LINK OFF) # force dynamic linking for all libraries
ENDIF(WIN32)

FIND_PACKAGE(Boost 1.67 REQUIRED COMPONENTS ${BOOST_COMPONENTS})
# For Boost 1.53 on windows, coroutine was not in BOOST_LIBRARYDIR and do not need it to build,  but if boost versin >= 1.54, find coroutine otherwise will cause link errors
IF(NOT "${Boost_VERSION}" MATCHES "1.53(.*)")
   SET(BOOST_LIBRARIES_TEMP ${Boost_LIBRARIES})
   FIND_PACKAGE(Boost 1.54 REQUIRED COMPONENTS coroutine)
   LIST(APPEND BOOST_COMPONENTS coroutine)
   SET(Boost_LIBRARIES ${BOOST_LIBRARIES_TEMP} ${Boost_LIBRARIES})
ENDIF()

if( WIN32 )

    message( STATUS "Configuring Peerplays on WIN32")
    set( DB_VERSION 60 )
    set( BDB_STATIC_LIBS 1 )

    set( ZLIB_LIBRARIES "" )
    SET( DEFAULT_EXECUTABLE_INSTALL_DIR bin/ )

    set(CRYPTO_LIB)

    #looks like this flag can have different default on some machines.
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")

    # Probably cmake has a bug and vcxproj generated for executable in Debug conf. has disabled debug info
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")

    # On windows tcl should be installed to the directory pointed by setenv.bat script
    SET(TCL_INCLUDE_PATH $ENV{TCL_ROOT}/include)
    MESSAGE(STATUS "tcl INCLUDE PATH: ${TCL_INCLUDE_PATH}")

    FIND_PACKAGE(TCL)
    MESSAGE(STATUS "tcl_library: ${TCL_LIBRARY}")

    SET(TCL_LIBS "optimized;${TCL_LIBRARY};debug;")
    get_filename_component(TCL_LIB_PATH "${TCL_LIBRARY}" PATH)
    get_filename_component(TCL_LIB_NAME "${TCL_LIBRARY}" NAME_WE)
    get_filename_component(TCL_LIB_EXT "${TCL_LIBRARY}" EXT)

    SET(TCL_LIBS "${TCL_LIBS}${TCL_LIB_PATH}/${TCL_LIB_NAME}g${TCL_LIB_EXT}")
    SET(TCL_LIBRARY ${TCL_LIBS})

else( WIN32 ) # Apple AND Linux
    if( APPLE )
        # Apple Specific Options Here
        message( STATUS "Configuring Peerplays on OS X" )
        set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -stdlib=libc++ -Wall" )
    else( APPLE )
        # Linux Specific Options Here
        message( STATUS "Configuring Peerplays on Linux" )
        set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -Wall" )
        set( rt_library rt )
        #set( pthread_library pthread)
        set(CMAKE_LINKER_FLAGS "-pthread" CACHE STRING "Linker Flags" FORCE)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_LINKER_FLAGS}" CACHE STRING "" FORCE)
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS}" CACHE STRING "" FORCE)
        set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_LINKER_FLAGS}" CACHE STRING "" FORCE)
        if ( NOT DEFINED crypto_library )
          # I'm not sure why this is here, I guess someone has openssl and can't detect it with find_package()?
          # if you have a normal install, you can define crypto_library to the empty string to avoid a build error
          set( crypto_library crypto)
        endif ()
        if ( FULL_STATIC_BUILD )
          set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
        endif ( FULL_STATIC_BUILD )
    endif( APPLE )

    if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
    elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
        if( CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.0.0 OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.0.0 )
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-partial-specialization" )
        endif()
    endif()

    if( "${CMAKE_GENERATOR}" STREQUAL "Ninja" )
        if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics" )
        endif()
    endif()

    # based on http://www.delorie.com/gnu/docs/gdb/gdb_70.html
    # uncomment this line to tell GDB about macros (slows compile times)
    # set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-2 -g3" )

endif( WIN32 )

set(ENABLE_COVERAGE_TESTING FALSE CACHE BOOL "Build Peerplays for code coverage analysis")

if(ENABLE_COVERAGE_TESTING)
    SET(CMAKE_CXX_FLAGS "--coverage ${CMAKE_CXX_FLAGS}")
endif()

add_subdirectory( libraries )


set(BUILD_PEERPLAYS_PROGRAMS TRUE CACHE BOOL "Build peerplays executables (witness node, cli wallet, etc)")
add_subdirectory( programs )

set(BUILD_PEERPLAYS_TESTS TRUE CACHE BOOL "Build peerplays unit tests")
if( BUILD_PEERPLAYS_TESTS )
  add_subdirectory( tests )
endif( BUILD_PEERPLAYS_TESTS )


if (ENABLE_INSTALLER)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)


include(InstallRequiredSystemLibraries)

set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_BINARY_DIR}/packages)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

SET(CPACK_PACKAGE_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
set(CPACK_PACKAGE_NAME "graphene")
set(CPACK_PACKAGE_VENDOR "Cryptonomex, Inc.")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION "A client for the Peerplays network")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A client for the Peerplays network")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Peerplays ${CPACK_PACKAGE_VERSION}")

if(WIN32)
 SET(CPACK_GENERATOR "ZIP;NSIS")
 set(CPACK_PACKAGE_NAME "Peerplays") # override above
 set(CPACK_NSIS_EXECUTABLES_DIRECTORY .)
 set(CPACK_NSIS_PACKAGE_NAME "Peerplays v${CPACK_PACKAGE_VERSION}")
 set(CPACK_NSIS_DISPLAY_NAME "${CPACK_NSIS_PACKAGE_NAME}")
 set(CPACK_NSIS_DEFINES "  !define MUI_STARTMENUPAGE_DEFAULTFOLDER \\\"Peerplays\\\"")
 # it seems like windows zip files usually don't have a single directory inside them, unix tgz frequently do
 SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

endif(WIN32)

if(APPLE)
  set(CPACK_GENERATOR "DragNDrop")
endif()

if(LINUX)
  # Linux gets a .tgz
  SET(CPACK_GENERATOR "TGZ")
  SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 1)
endif(LINUX)

 include(CPack)
endif(ENABLE_INSTALLER)

unset(GRAPHENE_EGENESIS_JSON)
unset(GRAPHENE_EGENESIS_JSON CACHE)
unset(BUILD_PEERPLAYS_TESTNET)
unset(BUILD_PEERPLAYS_TESTNET CACHE)
