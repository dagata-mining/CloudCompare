option( PLUGIN_IO_LIBPOINTMATCHER "Install LIBPOINTMATCHER" ON )

if( PLUGIN_IO_LIBPOINTMATCHER )
	project( LIBPOINTMATCHER )

	
	add_library( ${PROJECT_NAME} STATIC )

	add_subdirectory( include )
	add_subdirectory( src )
	#add_subdirectory( ui )
	#add_subdirectory( PointMatcherSubdirectory ) # PCL bridge library
	find_package( libpointmatcher REQUIRED )
	find_package (Boost COMPONENTS system filesystem REQUIRED)
	target_include_directories( ${PROJECT_NAME}
	PUBLIC
		${Boost_INCLUDE_DIRS}
	    ${libpointmatcher_INCLUDE_DIRS}
		
	)

	target_link_directories( ${PROJECT_NAME}
		PUBLIC
			${Boost_LIBRARY_DIRS}
		    ${libpointmatcher_LIBRARY_DIRS}
	)
	target_link_libraries( ${PROJECT_NAME}
	CCPluginAPI
	QCC_GL_LIB
	${libpointmatcher_LIBRARIES}
	${Boost_LIBRARIES}
	)

	#set( LIBPOINTMATCHER_ROOT_DIR "" CACHE PATH "LIBPOINTMATCHER ()" )
	#set( LIBPOINTMATCHER_LIBRARIES "" CACHE FILEPATH "LIBPOINTMATCHER LIB ()" )

	
endif()
