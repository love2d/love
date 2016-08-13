# Sets the following variables:
#
# MPG123_FOUND
# MPG123_INCLUDE_DIR
# MPG123_LIBRARY

set(MPG123_SEARCH_PATHS
	/usr/local
	/usr
	)

find_path(MPG123_INCLUDE_DIR
	NAMES mpg123.h
	PATH_SUFFIXES include
	PATHS ${MPG123_SEARCH_PATHS})

find_library(MPG123_LIBRARY
	NAMES mpg123
	PATH_SUFFIXES lib
	PATHS ${MPG123_SEARCH_PATHS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MPG123 DEFAULT_MSG MPG123_LIBRARY MPG123_INCLUDE_DIR)

mark_as_advanced(MPG123_INCLUDE_DIR MPG123_LIBRARY)
