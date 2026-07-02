# Sets the following variables:
#
# OPUS_FOUND
# OPUS_INCLUDE_DIR
# OPUS_LIBRARY
# OPUSFILE_LIBRARY

set(OPUS_SEARCH_PATHS
	/usr/local
	/usr
	)

find_path(OPUS_INCLUDE_DIR opus
	PATH_SUFFIXES include
	PATHS ${OPUS_SEARCH_PATHS})

find_library(OPUS_LIBRARY
	NAMES opus
	PATH_SUFFIXES lib
	PATHS ${OPUS_SEARCH_PATHS})

find_library(OPUSFILE_LIBRARY
	NAMES opusfile
	PATH_SUFFIXES lib
	PATHS ${OPUS_SEARCH_PATHS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Opus DEFAULT_MSG OPUS_LIBRARY OPUSFILE_LIBRARY OPUS_INCLUDE_DIR)

mark_as_advanced(OPUS_INCLUDE_DIR OPUS_LIBRARY OPUSFILE_LIBRARY)
