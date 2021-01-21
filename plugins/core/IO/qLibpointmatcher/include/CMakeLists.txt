
target_sources( ${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/LibpointmatcherFilter.h
		${CMAKE_CURRENT_LIST_DIR}/LibpointmatcherConvert.h

)

target_include_directories( ${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}
)
