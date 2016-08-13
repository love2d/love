# Sets the following variables:
#
# MODPLUG_FOUND
# MODPLUG_INCLUDE_DIR
# MODPLUG_LIBRARY

set(MODPLUG_SEARCH_PATHS
	/usr/local
	/usr
	)

find_path(MODPLUG_INCLUDE_DIR
	NAMES libmodplug/modplug.h
	PATH_SUFFIXES include
	PATHS ${MODPLUG_SEARCH_PATHS})

find_library(MODPLUG_LIBRARY
	NAMES modplug
	PATH_SUFFIXES lib
	PATHS ${MODPLUG_SEARCH_PATHS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ModPlug DEFAULT_MSG MODPLUG_LIBRARY MODPLUG_INCLUDE_DIR)

mark_as_advanced(MODPLUG_INCLUDE_DIR MODPLUG_LIBRARY)
