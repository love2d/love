# Sets the following variables:
#
# THEORA_FOUND
# THEORA_INCLUDE_DIR
# THEORA_LIBRARY
# THEORADEC_LIBRARY

set(THEORA_SEARCH_PATHS
	/usr/local
	/usr
	)

find_path(THEORA_INCLUDE_DIR
	NAMES theora/codec.h
	PATH_SUFFIXES include
	PATHS ${THEORA_SEARCH_PATHS})

find_library(THEORA_LIBRARY
	NAMES theora
	PATH_SUFFIXES lib
	PATHS ${THEORA_SEARCH_PATHS})

find_library(THEORADEC_LIBRARY
	NAMES theoradec
	PATH_SUFFIXES lib
	PATHS ${THEORA_SEARCH_PATHS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Theora DEFAULT_MSG THEORA_LIBRARY THEORA_INCLUDE_DIR)

mark_as_advanced(THEORA_INCLUDE_DIR THEORA_LIBRARY)
