#
# CMakeLists.txt
#
# Copyright (C) 2009-11 by RStudio, Inc.
#
# This program is licensed to you under the terms of version 3 of the
# GNU Affero General Public License. This program is distributed WITHOUT
# ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
# MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
# AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
#
#

project (CORE)

# include files
file(GLOB_RECURSE CORE_HEADER_FILES "*.h*")

# source files
set (CORE_SOURCE_FILES
   BoostErrors.cpp
   ConfigUtils.cpp
   DateTime.cpp
   Error.cpp 
   Exec.cpp
   FileInfo.cpp 
   FileLogWriter.cpp
   FilePath.cpp
   FileSerializer.cpp
   Hash.cpp
   PerformanceTimer.cpp
   ProgramOptions.cpp
   SafeConvert.cpp
   Settings.cpp
   Log.cpp
   StringUtils.cpp
   Thread.cpp
   WaitUtils.cpp
   gwt/GwtFileHandler.cpp
   gwt/GwtLogHandler.cpp
   json/Json.cpp
   json/JsonRpc.cpp
   json/spirit/json_spirit_reader.cpp
   json/spirit/json_spirit_value.cpp
   json/spirit/json_spirit_writer.cpp
   http/Cookie.cpp
   http/Header.cpp
   http/Message.cpp
   http/MultipartRelated.cpp
   http/Request.cpp
   http/RequestParser.cpp
   http/Response.cpp
   http/URL.cpp
   http/UriHandler.cpp
   http/Util.cpp
   system/System.cpp
   text/TemplateFilter.cpp
)

# UNIX specific
if (UNIX)

   # platform introspection
   check_symbol_exists(SA_NOCLDWAIT "signal.h" HAVE_SA_NOCLDWAIT)
   check_function_exists(inotify_init1 HAVE_INOTIFY)
   check_symbol_exists(SO_PEERCRED "sys/socket.h" HAVE_SO_PEERCRED)
   check_function_exists(getpeereid HAVE_GETPEEREID)
   if(EXISTS "/proc/self")
      set(HAVE_PROCSELF TRUE)
   endif()
   configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
                   ${CMAKE_CURRENT_BINARY_DIR}/config.h)

   # find packages and libraries
   find_library(PTHREAD_LIBRARIES pthread)
   if(NOT APPLE)
      find_library(UUID_LIBRARIES uuid)
   endif()
   find_package(ZLIB REQUIRED QUIET)

   # include directories and libraries
   set (CORE_SYSTEM_LIBRARIES
      ${PTHREAD_LIBRARIES}
      ${UUID_LIBRARIES}
      ${ZLIB_LIBRARIES}
   )

   # make directory monitor conditional
   if(HAVE_INOTIFY)
      set(DIRECTORY_MONITOR_CPP system/INotifyDirectoryMonitor.cpp)
   else()
      set(DIRECTORY_MONITOR_CPP system/DirectoryMonitor.cpp)
   endif()

   # source files
   set(CORE_SOURCE_FILES ${CORE_SOURCE_FILES}
      ${DIRECTORY_MONITOR_CPP}
      r_util/REnvironmentPosix.cpp
      SyslogLogWriter.cpp
      system/PosixParentProcessMonitor.cpp
      system/PosixOutputCapture.cpp
      system/PosixSystem.cpp
      system/PosixUser.cpp
   )


# Win32 specific
else()

   # system libraries
   set (CORE_SYSTEM_LIBRARIES -lws2_32 -lmswsock -lrpcrt4)

   # source files
   set(CORE_SOURCE_FILES ${CORE_SOURCE_FILES}
      system/DirectoryMonitor.cpp
      system/RegistryKey.cpp
      system/Win32ParentProcessMonitor.cpp
      system/Win32OutputCapture.cpp
      system/Win32System.cpp
   )

endif()

# define include dirs
set(CORE_INCLUDE_DIRS include)

# search for core addins
if(RSTUDIO_ADDINS_PATH)
   set(CORE_ADDIN_PATH ${RSTUDIO_ADDINS_PATH}/core)
   if(EXISTS ${CORE_ADDIN_PATH})
      file(GLOB_RECURSE ADDIN_HEADER_FILES "${CORE_ADDIN_PATH}/*.h*")
      list(APPEND CORE_HEADER_FILES ${ADDIN_HEADER_FILES})
      file(GLOB_RECURSE ADDIN_SOURCE_FILES "${CORE_ADDIN_PATH}/*.c*")
      list(APPEND CORE_SOURCE_FILES ${ADDIN_SOURCE_FILES})
      list(APPEND CORE_INCLUDE_DIRS ${CORE_ADDIN_PATH}/include)
   endif()
endif()


# include directories
include_directories(
   ${CORE_INCLUDE_DIRS}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${Boost_INCLUDE_DIRS}
)

# define shared library
add_library(rstudio-core STATIC ${CORE_SOURCE_FILES} ${CORE_HEADER_FILES})

# link dependencies
target_link_libraries(
   rstudio-core
   ${Boost_LIBRARIES}
   ${CORE_SYSTEM_LIBRARIES}
)
