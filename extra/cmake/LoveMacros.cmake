
# Group all related stuff in folders in Visual Studio

function(love_group_projects)
	set(options NESTED)
	set(oneValueArgs NAME)
	set(multiValueArgs TARGETS)
	cmake_parse_arguments(LOVE_GROUP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	foreach(TARGET_NAME ${LOVE_GROUP_TARGETS})
		if (TARGET ${TARGET_NAME})
			get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
			# If this is an interface target, walk the children
			if (${TARGET_TYPE} STREQUAL "INTERFACE_LIBRARY")
				get_target_property(TARGET_LIBS ${TARGET_NAME} INTERFACE_LINK_LIBRARIES)
				foreach(TARGET_LIB ${TARGET_LIBS})
					# Is this a target? (Could also be a .lib file)
					if (TARGET ${TARGET_LIB})
						# Do we want to nest per-project?
						if (LOVE_GROUP_NESTED)
							set_target_properties(${TARGET_LIB} PROPERTIES FOLDER "${LOVE_GROUP_NAME}/${TARGET_NAME}")
						else()
							set_target_properties(${TARGET_LIB} PROPERTIES FOLDER "${LOVE_GROUP_NAME}")
						endif()
					endif()
				endforeach()
			else()
				# This is no interface library, so group it under the root 'NAME' node
				set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "${LOVE_GROUP_NAME}")
			endif()
		endif()
	endforeach()
endfunction()
