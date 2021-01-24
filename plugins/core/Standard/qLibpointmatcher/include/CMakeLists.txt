
target_sources( ${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/LibpointmatcherFilter.h
		${CMAKE_CURRENT_LIST_DIR}/LibpointmatcherConvert.h
		${CMAKE_CURRENT_LIST_DIR}/Libpointmatcher.h
		${CMAKE_CURRENT_LIST_DIR}/LibpointmatcherCommands.h
		${CMAKE_CURRENT_LIST_DIR}/LibpointmatcherDialog.h
		${CMAKE_CURRENT_LIST_DIR}/LibpointmatcherDisclaimerDialog.h
		${CMAKE_CURRENT_LIST_DIR}/LibpointmatcherProcess.h
		${CMAKE_CURRENT_LIST_DIR}/LibpointmatcherTools.h
		
		

)

target_include_directories( ${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}
)
