# Sets the following variables:
#
#  HARFBUZZ_FOUND
#  HARFBUZZ_INCLUDE_DIR
#  HARFBUZZ_LIBRARY

set(HARFBUZZ_SEARCH_PATHS
	/usr/local
	/usr
	)

find_path(HARFBUZZ_INCLUDE_DIR
	NAMES hb.h
	PATH_SUFFIXES include include/harfbuzz
	PATHS ${HARFBUZZ_SEARCH_PATHS})

find_library(HARFBUZZ_LIBRARY
	NAMES harfbuzz
	PATH_SUFFIXES lib
	PATHS ${HARFBUZZ_SEARCH_PATHS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Harfbuzz DEFAULT_MSG HARFBUZZ_LIBRARY HARFBUZZ_INCLUDE_DIR)

mark_as_advanced(HARFBUZZ_INCLUDE_DIR HARFBUZZ_LIBRARY)
