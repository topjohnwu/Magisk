# - Returns a version string from Git tags
#
# This function inspects the annotated git tags for the project and returns a string
# into a CMake variable
#
#  get_git_version(<var>)
#
# - Example
#
# include(GetGitVersion)
# get_git_version(GIT_VERSION)
#
# Requires CMake 2.8.11+
find_package(Git)

if(__get_git_version)
  return()
endif()
set(__get_git_version INCLUDED)

function(get_git_version var)
  if(GIT_EXECUTABLE)
      execute_process(COMMAND ${GIT_EXECUTABLE} describe --match "v[0-9]*.[0-9]*.[0-9]*" --abbrev=8
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
          RESULT_VARIABLE status
          OUTPUT_VARIABLE GIT_VERSION
          ERROR_QUIET)
      if(${status})
          set(GIT_VERSION "v0.0.0")
      else()
          string(STRIP ${GIT_VERSION} GIT_VERSION)
          string(REGEX REPLACE "-[0-9]+-g" "-" GIT_VERSION ${GIT_VERSION})
      endif()

      # Work out if the repository is dirty
      execute_process(COMMAND ${GIT_EXECUTABLE} update-index -q --refresh
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
          OUTPUT_QUIET
          ERROR_QUIET)
      execute_process(COMMAND ${GIT_EXECUTABLE} diff-index --name-only HEAD --
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
          OUTPUT_VARIABLE GIT_DIFF_INDEX
          ERROR_QUIET)
      string(COMPARE NOTEQUAL "${GIT_DIFF_INDEX}" "" GIT_DIRTY)
      if (${GIT_DIRTY})
          set(GIT_VERSION "${GIT_VERSION}-dirty")
      endif()
  else()
      set(GIT_VERSION "v0.0.0")
  endif()

  message(STATUS "git Version: ${GIT_VERSION}")
  set(${var} ${GIT_VERSION} PARENT_SCOPE)
endfunction()
