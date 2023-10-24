cmake_minimum_required(VERSION 3.14)

project(Serious-Runner VERSION 1.1.2)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW) #3.12.0 `find_package()`` uses ``<PackageName>_ROOT`` variables.
endif()

OPTION(ENABLE_LZMA "Enable the use of the system LZMA library if found" ON)
OPTION(ENABLE_ZLIB "Enable the use of the system ZLIB library if found" ON)
OPTION(ENABLE_BZip2 "Enable the use of the system BZip2 library if found" ON)
OPTION(ENABLE_STATIC_LINK "Enable static link" OFF)

IF(MINGW)
  ADD_DEFINITIONS(-D__USE_MINGW_ANSI_STDIO)
  ADD_DEFINITIONS(-D__MINGW_USE_VC2005_COMPAT)
ENDIF()

IF(WIN32)
  SET(LOCAL_INSTALL TRUE)
ENDIF()

#
# Check compress/decompress libraries
#
IF(WIN32 AND NOT CYGWIN)
  # GnuWin64
  IF(ENABLE_STATIC_LINK)
    SET(__GNUWIN64PATH "C:/Program Files/GnuWin64-static")
  ELSE()
    SET(__GNUWIN64PATH "C:/Program Files/GnuWin64")
ENDIF()      
ENDIF(WIN32 AND NOT CYGWIN)
IF(DEFINED __GNUWIN64PATH AND EXISTS "${__GNUWIN64PATH}")
  # You have to add a path available DLL file into PATH environment variable.
  # Maybe DLL path is "C:/Program Files/GnuWin64-dynamic/bin".
  # The zlib and the bzip2 Setup program have installed programs and DLLs into
  # "C:/Program Files/GnuWin32" by default.
  # This is convenience setting for Windows.
  SET(CMAKE_PREFIX_PATH ${__GNUWIN64PATH} $(CMAKE_PREFIX_PATH))
  #
  # If you didn't use Setup program or installed into nonstandard path,
  # cmake cannot find out your zlib or bzip2 libraries and include files,
  # you should execute cmake with  -DCMAKE_PREFIX_PATH option.
  #   e.g.
  #     cmake -DCMAKE_PREFIX_PATH=<your-GnuWin64-path> <path-to-source>
  #
  # If compiling error occurred in zconf.h, You may need patch to zconf.h.
  #--- zconf.h.orig	2005-07-21 00:40:26.000000000
  #+++ zconf.h	2009-01-19 11:39:10.093750000
  #@@ -286,7 +286,7 @@
  #
  # #if 1           /* HAVE_UNISTD_H -- this line is updated by ./configure */
  # #  include <sys/types.h> /* for off_t */
  #-#  include <unistd.h>    /* for SEEK_* and off_t */
  #+#  include <stdio.h>    /* for SEEK_* and off_t */
  # #  ifdef VMS
  # #    include <unixio.h>   /* for off_t */
  # #  endif
ENDIF(DEFINED __GNUWIN64PATH AND EXISTS "${__GNUWIN64PATH}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(CheckCXXCompilerFlag)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Sql)
find_package(QT NAMES Qt6 Qt5  REQUIRED COMPONENTS Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)

include(FindLibLZMA)
include(FindBZip2)
include(FindZLIB)

if(CMAKE_VERSION VERSION_GREATER 3.17.0)
    include(FindLibArchive)
else()
    # cmake < 3.17.0 Find LIBARCHIVE
    option(CMAKE_USE_SYSTEM_LIBARCHIVE "Use system-installed libarchive" "On")
    if(CMAKE_USE_SYSTEM_LIBARCHIVE)
      find_package(LibArchive 3.0.1)
      if(NOT LibArchive_FOUND)
        message(FATAL_ERROR "CMAKE_USE_SYSTEM_LIBARCHIVE is ON but LibArchive is not found!")
      else()
        message(STATUS "Find LibArchive Done.")
        set(LibArchive_INCLUDE_DIRS ${LibArchive_INCLUDE_DIR})
        set(LibArchive_LIBRARIES    ${LibArchive_LIBRARY})
        if (NOT TARGET LibArchive::LibArchive)
          add_library(LibArchive::LibArchive UNKNOWN IMPORTED)
          set_target_properties(LibArchive::LibArchive PROPERTIES
            IMPORTED_LOCATION "${LibArchive_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${LibArchive_INCLUDE_DIR}")
        endif() 
      endif()
    endif()
endif()

set(INTERNAL_NAME "seriousrunner")

# RAKE! Where to install the binaries.
if(CMAKE_INSTALL_PREFIX STREQUAL "/usr/local" OR CMAKE_INSTALL_PREFIX STREQUAL "") # Only works for linux since I don't
                                                                                   # know what default is for windows/macos/freebsd.

    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/")                               # set install path to project root directory since
                                                                                   # since one wasn't set during config
    set(LOCAL_INSTALL TRUE)
    message(STATUS "Using LOCAL Install")
endif()

if(CMAKE_INSTALL_PREFIX STREQUAL "/usr" ) # Only works for linux since I don't know what default is for windows/macos/freebsd.
    set(LOCAL_INSTALL FALSE)
    message(STATUS "Using SYSTEM Install")
endif()

# ImageMagic needed for SYSTEM INSTALL
if(NOT LOCAL_INSTALL)
    find_package(ImageMagick COMPONENTS convert)
    if(ImageMagick_convert_FOUND)
        #message(STATUS "ImageMagick convert found: ${ImageMagick_convert_EXECUTABLE}")
    else()
        message(FATAL_ERROR "Error! LOCAL_INSTAL is set but neccessary ImageMagick convert are missing")
    endif()
endif()

# Set up some sanity stuff...
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wno-reorder -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Wno-class-memaccess -Wno-unused-function -Wno-format")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-reorder -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Wno-class-memaccess -Wno-unused-function -Wno-format")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(FREEBSD TRUE)
endif()
if(APPLE)
    set(MACOSX TRUE)
endif()
if(MSVC)
    set(WINDOWS TRUE)
endif()

# remove clang spam.
if(MACOSX OR FREEBSD)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-nullability-completeness")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nullability-completeness")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "None Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

set(DEBUG FALSE)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG TRUE)
endif()

if (LINUX)
    set (CMAKE_OS_NAME "GNU/Linux" CACHE STRING "Operating system name" FORCE)
    ## Check for Debian GNU/Linux
    find_file (DEBIAN_FOUND debian_version debconf.conf
      PATHS /etc
      )
    if (DEBIAN_FOUND)
      set (CMAKE_OS_NAME "Debian" CACHE STRING "Operating system name" FORCE)
    endif (DEBIAN_FOUND)
    ##  Check for Fedora
    find_file (FEDORA_FOUND fedora-release
      PATHS /etc
      )
    if (FEDORA_FOUND)
      set (CMAKE_OS_NAME "Fedora" CACHE STRING "Operating system name" FORCE)
    endif (FEDORA_FOUND)
    ##  Check for RedHat
    find_file (REDHAT_FOUND redhat-release inittab.RH
      PATHS /etc
      )
    if (REDHAT_FOUND)
      set (CMAKE_OS_NAME "RedHat" CACHE STRING "Operating system name" FORCE)
    endif (REDHAT_FOUND)   
    ## Extra check for Ubuntu    
    if (DEBIAN_FOUND)
      ## At its core Ubuntu is a Debian system, with
      ## a slightly altered configuration; hence from
      ## a first superficial inspection a system will
      ## be considered as Debian, which signifies an
      ## extra check is required.
      find_file (UBUNTU_EXTRA legal issue
	PATHS /etc
	)
    if (UBUNTU_EXTRA)
	## Scan contents of file
	file (STRINGS ${UBUNTU_EXTRA} UBUNTU_FOUND
	  REGEX Ubuntu
	  )
	## Check result of string search
	if (UBUNTU_FOUND)
	  set (CMAKE_OS_NAME "Ubuntu" CACHE STRING "Operating system name" FORCE)
	  set (DEBIAN_FOUND FALSE)
	endif (UBUNTU_FOUND)
    endif (UBUNTU_EXTRA)      
    endif (DEBIAN_FOUND)
endif (LINUX)

if(WIN32)
    set(CMAKE_OS_NAME "Windows")
    add_definitions(-DPLATFORM_WIN32=1)
    add_definitions(-DPRAGMA_ONCE=1)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE=1)
endif()

message(STATUS "Operating system name: ${CMAKE_OS_NAME}")

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    # This section and the like are for flags/defines that can be shared between 
    # c and c++ compile options
    add_compile_options(-Wall)
    add_compile_options(-pipe)
    add_compile_options(-fPIC)
    add_compile_options(-fms-extensions)
    if(LOCAL_INSTALL)
       add_compile_options(-march=native)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i586|i686|x86|AMD64|x86_64")
       add_compile_options(-march=native) # set mtune=generic if nedded
    endif()
    add_compile_options(-fno-strict-aliasing)
    add_definitions(-D_REENTRANT=1)
    add_definitions(-D_MT=1)

    ## Add your custom C and CXX flags on the command line aka -DCMAKE_C_FLAGS=-std=c98 or -DCMAKE_CXX_FLAGS=-std=c++11
    ## For C flags
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g -D_DEBUG=1 -DDEBUG=1 -O0")
        if(MACOSX AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
            set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -DNDEBUG=1 -D_NDEBUG=1 -g -O0 -fno-unsafe-math-optimizations")
            set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS} -DNDEBUG=1 -D_NDEBUG=1 -g -O0 -fno-unsafe-math-optimizations")
            set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS} -DNDEBUG=1 -D_NDEBUG=1 -O0 -fno-unsafe-math-optimizations")
        else()
            set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -DNDEBUG=1 -D_NDEBUG=1 -g -O2 -fno-delete-null-pointer-checks -fno-unsafe-math-optimizations")
            set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS} -DNDEBUG=1 -D_NDEBUG=1 -g -O2 -fno-delete-null-pointer-checks -fno-unsafe-math-optimizations")
            set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS} -DNDEBUG=1 -D_NDEBUG=1 -Os -fno-delete-null-pointer-checks -fno-unsafe-math-optimizations")
        endif()

    ## For C++ flags
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -D_DEBUG=1 -DDEBUG=1 -O0")
        if(MACOSX AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG=1 -D_NDEBUG=1 -O0 -fno-unsafe-math-optimizations")
            set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} -DNDEBUG=1 -D_NDEBUG=1 -g -O0 -fno-unsafe-math-optimizations") ## RAKE! Does -DNDEBUG=1 and -D_NDEBUG=1 mess with RelWithDebInfo?
            set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS} -DNDEBUG=1 -D_NDEBUG=1 -O0 -fno-unsafe-math-optimizations")
        else()
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG=1 -D_NDEBUG=1 -O2 -fno-delete-null-pointer-checks -fno-unsafe-math-optimizations")
            set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} -DNDEBUG=1 -D_NDEBUG=1 -g -O2 -fno-delete-null-pointer-checks -fno-unsafe-math-optimizations") ## RAKE! Does -DNDEBUG=1 and -D_NDEBUG=1 mess with RelWithDebInfo?
            set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS} -DNDEBUG=1 -D_NDEBUG=1 -Os -fno-delete-null-pointer-checks -fno-unsafe-math-optimizations")
        endif()

    # TODO fix these warnings
    add_compile_options(-Wno-format-security)
    add_compile_options(-Wno-switch)
    add_compile_options(-Wno-char-subscripts)
    add_compile_options(-Wno-unknown-pragmas)
    add_compile_options(-Wno-unused-variable) # TODO: maybe only enable this for Entities
    add_compile_options(-Wno-unused-value)    # same here (the Scripts generate tons of unused variables and values)
    add_compile_options(-Wno-missing-braces)
    add_compile_options(-Wno-overloaded-virtual)
    add_compile_options(-Wno-invalid-offsetof)
    #MESSAGE(WARNING, "re-enable some of the warnings some day!")

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
        # !!! FIXME: turn a bunch of these off, this is just for now. I hope.  --ryan.
        add_compile_options(-Wno-c++11-compat-deprecated-writable-strings)
    endif()

    if(MACOSX)
        add_definitions(-DPLATFORM_UNIX=1)
        add_definitions(-DPLATFORM_MACOSX=1)
        add_definitions(-DPRAGMA_ONCE=1)
        include_directories("/usr/local/include")
        include_directories("/usr/X11/include/")
    elseif(WINDOWS)
        add_definitions(-DPLATFORM_WIN32=1)
        add_definitions(-DPRAGMA_ONCE=1)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
        add_definitions(-D_CRT_SECURE_NO_DEPRECATE=1)
    elseif(LINUX)
        set(CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE)
        add_definitions(-DPLATFORM_UNIX=1)
        add_definitions(-D_FILE_OFFSET_BITS=64)
        add_definitions(-D_LARGEFILE_SOURCE=1)
        add_definitions(-DPRAGMA_ONCE=1)
    elseif(FREEBSD)
        set(CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE)
        add_definitions(-DPLATFORM_UNIX=1)
        add_definitions(-DPLATFORM_FREEBSD=1)
        add_definitions(-D_FILE_OFFSET_BITS=64)
        add_definitions(-D_LARGEFILE_SOURCE=1)
        add_definitions(-DPRAGMA_ONCE=1)
        include_directories("/usr/local/include")
    endif()

    if(MACOSX OR LINUX OR FREEBSD)
        add_compile_options(-pthread)
        add_compile_options(-fsigned-char)
    endif()

    if(CMAKE_COMPILER_IS_GNUCC)
        # !!! FIXME: turn a bunch of these off, this is just for now. I hope.  --ryan.
        add_compile_options(-Wno-invalid-offsetof)
    endif()

elseif(MSVC) # RAKE! I don't know if this will build with MSVC
    add_compile_options(/W4)
    add_compile_options(/wd4100) # unreferenced formal parameter
    add_compile_options(/wd4127) # conditional expression is constant
    add_compile_options(/wd4244) # possible loss of data
    add_compile_options(/wd4245) # signed/unsigned mismatch
    add_compile_options(/wd4267) # possible loss of data
    add_compile_options(/wd4714) # 'function' marked as __forceinline not inlined
    add_compile_options(/wd4996) # 'function': was declared deprecated
    add_compile_options(/wd4068) # unknown pragma
    set(CMAKE_C_FLAGS_DEBUG "-D_DEBUG /Od /Zi /MDd")
    set(CMAKE_C_FLAGS_RELEASE "/Ox /Oy /MD")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "/Ox /Oy /Zi /MD")
    set(CMAKE_C_FLAGS_MINSIZEREL "/Ox /Oy /Os /MD")
else()
    message(FATAL_ERROR "Unsupported compiler")
endif()

if(WIN32)
    set(PROJECT_SOURCES
        Resources.qrc
        Main.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
        Classes/Downloader.cpp
        Classes/Downloader.h
        Classes/Unpacker.cpp
        Classes/Unpacker.h
        Classes/CopyDir.cpp
        Classes/CopyDir.h
        Classes/Types.h
)
else()
    set(PROJECT_SOURCES
        Resources.qrc
        Main.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
        Classes/Downloader.cpp
        Classes/Downloader.h
        Classes/Unpacker.cpp
        Classes/Unpacker.h
        Classes/FindInHomeDir.cpp
        Classes/FindInHomeDir.h
        Classes/CopyDir.cpp
        Classes/CopyDir.h
        Classes/Types.h
)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Serious-Runner
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Serious-Runner APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Serious-Runner SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    elseif(WIN32)
	    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/seriousrunner.rc")
        add_executable(Serious-Runner
            ${PROJECT_SOURCES}  ${APP_ICON_RESOURCE_WINDOWS}
        )
    else()
        add_executable(Serious-Runner
            ${PROJECT_SOURCES}
        )		
    endif()
endif()

# Link libraries.
if(WIN32)
    if(ENABLE_STATIC_LINK)
        target_link_libraries(Serious-Runner PUBLIC "-static" Qt${QT_VERSION_MAJOR}::Widgets  Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network LibArchive::LibArchive LibLZMA::LibLZMA BZip2::BZip2 ZLIB::ZLIB)
    else()
            target_link_libraries(Serious-Runner PRIVATE Qt${QT_VERSION_MAJOR}::Widgets  Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network LibArchive::LibArchive LibLZMA::LibLZMA BZip2::BZip2 ZLIB::ZLIB)
	endif()
else()
    target_link_libraries(Serious-Runner PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network LibArchive::LibArchive LibLZMA::LibLZMA BZip2::BZip2 ZLIB::ZLIB)
endif()

set_target_properties(Serious-Runner PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

#install(TARGETS Serious-Runner
#    BUNDLE DESTINATION .
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Serious-Runner)
endif()

# Set output name for not local installation
if(LOCAL_INSTALL)
 set_target_properties(Serious-Runner PROPERTIES OUTPUT_NAME "Serious-Runner")
else()
 set_target_properties(Serious-Runner PROPERTIES OUTPUT_NAME "${INTERNAL_NAME}")
endif()

# RAKE! Install Section.
if(LOCAL_INSTALL)
    install(TARGETS Serious-Runner
            RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}"
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
else()
    install(TARGETS Serious-Runner
            RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()


# RAKE! If CMAKE_INSTALL_PREFIX was set during config then its not a local install
if(NOT LOCAL_INSTALL)
    install(FILES ${CMAKE_ADD_TARGET_DIR}DB/seriousrunner.db
            DESTINATION "${CMAKE_INSTALL_PREFIX}/share/${INTERNAL_NAME}/DB"
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
    install(FILES "${CMAKE_ADD_TARGET_DIR}${INTERNAL_NAME}.desktop"
            DESTINATION "${CMAKE_INSTALL_PREFIX}//share/applications"
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
    set(A 16)
    set(B 32)
    set(C 48)
    set(D 64)
    set(E 128)
    foreach(X IN LISTS A B C D E)
        #message(STATUS "Create icon ${X}x${X}")
        execute_process (
            COMMAND bash -c "${ImageMagick_convert_EXECUTABLE} ${CMAKE_ADD_TARGET_DIR}../Icons/${INTERNAL_NAME}.png -scale ${X}x${X} ${CMAKE_ADD_TARGET_DIR}../${X}.png;"
            OUTPUT_VARIABLE outVar
       )
       install(FILES ${CMAKE_ADD_TARGET_DIR}${X}.png
               DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/${X}x${X}/apps" RENAME "${INTERNAL_NAME}.png"
               PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
    endforeach()
endif()

# end of CMakeLists.txt ...
