# Sets the following variables:
#
# OGG_FOUND
# OGG_INCLUDE_DIR
# OGG_LIBRARY

set(OGG_SEARCH_PATHS
	/usr/local
	/usr
	)

find_path(OGG_INCLUDE_DIR
	NAMES ogg/ogg.h
	PATH_SUFFIXES include
	PATHS ${OGG_SEARCH_PATHS})

find_library(OGG_LIBRARY
	NAMES ogg
	PATH_SUFFIXES lib
	PATHS ${OGG_SEARCH_PATHS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ogg DEFAULT_MSG OGG_LIBRARY OGG_INCLUDE_DIR)

mark_as_advanced(OGG_INCLUDE_DIR OGG_LIBRARY)
