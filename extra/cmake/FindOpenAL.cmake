# FindOpenAL.cmake that comes with CMake expects "#include <al.h>" to be used
# However, we use "#include <AL/al.h>"

# Sets the following variables:
#
# OPENAL_FOUND
# OPENAL_INCLUDE_DIR
# OPENAL_LIBRARY

set(OPENAL_SEARCH_PATHS
	/usr/local
	/usr
	)

find_path(OPENAL_INCLUDE_DIR
	NAMES AL/al.h
	PATH_SUFFIXES include
	PATHS ${OPENAL_SEARCH_PATHS})

find_library(OPENAL_LIBRARY
	NAMES openal
	PATH_SUFFIXES lib
	PATHS ${OPENAL_SEARCH_PATHS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenAL DEFAULT_MSG OPENAL_LIBRARY OPENAL_INCLUDE_DIR)

mark_as_advanced(OPENAL_INCLUDE_DIR OPENAL_LIBRARY)
