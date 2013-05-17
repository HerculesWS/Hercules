#
# setup
#
get_property( CAN_BUILD_SHARED_LIBS  GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS )
if( NOT CAN_BUILD_SHARED_LIBS )
	return()
endif()

#
# sample
#
option( BUILD_PLUGIN_sample "build sample plugin" OFF )
if( BUILD_PLUGIN_sample )
message( STATUS "Creating target sample" )
set( SAMPLE_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/sample.c"
	)
set( LIBRARIES ${GLOBAL_LIBRARIES} )
set( INCLUDE_DIRS ${GLOBAL_INCLUDE_DIRS} )
set( DEFINITIONS ${GLOBAL_DEFINITIONS} )
set( SOURCE_FILES ${SAMPLE_SOURCES} )
source_group( sample FILES ${SAMPLE_SOURCES} )
include_directories( ${INCLUDE_DIRS} )
add_library( sample SHARED ${SOURCE_FILES} )
target_link_libraries( sample ${LIBRARIES} )
set_target_properties( sample PROPERTIES COMPILE_FLAGS "${DEFINITIONS}" )
set_target_properties( sample PROPERTIES PREFIX "" )
if( INSTALL_COMPONENT_RUNTIME )
	cpack_add_component( Runtime_sample DESCRIPTION "sample plugin" DISPLAY_NAME "sample" GROUP Runtime )
	install( TARGETS sample
		DESTINATION "plugins"
		COMPONENT Runtime_sample )
endif( INSTALL_COMPONENT_RUNTIME )
set( TARGET_LIST ${TARGET_LIST} sample  CACHE INTERNAL "" )
message( STATUS "Creating target sample - done" )
endif( BUILD_PLUGIN_sample )