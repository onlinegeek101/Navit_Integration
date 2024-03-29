include_directories( "${PROJECT_BINARY_DIR}")
include_directories( "${PROJECT_SOURCE_DIR}")
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories( "${CMAKE_CURRENT_BINARY_DIR}")
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/support")

# navit cre
set(NAVIT_SRC announcement.c atom.c attr.c cache.c callback.c command.c compass.c config_.c coord.c country.c data_window.c debug.c 
   event.c file.c graphics.c gui.c item.c layout.c log.c main.c map.c obj_filter.c
   linguistics.c mapset.c maptype.c menu.c messages.c bookmarks.c navit.c navigation.c osd.c param.c phrase.c plugin.c popup.c
   profile.c projection.c roadprofile.c route.c routech.c search.c speech.c start_real.c sunriset.c transform.c track.c 
   util.c vehicle.c vehicleprofile.c xmlconfig.c )

if(NOT USE_PLUGINS)
  list(APPEND NAVIT_SRC  ${CMAKE_CURRENT_BINARY_DIR}/builtin.c)
endif(NOT USE_PLUGINS)

if (${HAVE_GLIB})
   list(APPEND NAVIT_SRC event_glib.c)
endif()

if(ANDROID)
   list(APPEND NAVIT_SRC android.c)
   set(NAVIT_LIBNAME navit)
else()
   set(NAVIT_LIBNAME navit_core)
endif()


foreach ( PLUGIN_PATH ${ALL_PLUGINS})
   if (${PLUGIN_PATH})
      include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_PATH}")
      
      string(REPLACE "/" "_" MODULE_INC ${PLUGIN_PATH})
      list(APPEND NAVIT_SUPPORT_LIBS ${MODULE_INC})
   endif()
endforeach()

foreach ( MODULE_PATH ${ALL_MODULES})
   if ( ${MODULE_PATH} )
      include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_PATH}")
      
      string(REPLACE "/" "_" MODULE_INC ${MODULE_PATH})
      list(APPEND MODULES_PROTOTYPE "\nvoid\tmodule_${MODULE_INC}_init(void)")
      list(APPEND MODULES_FUNC "\nmodule_${MODULE_INC}_init()")
      
      if (NOT USE_PLUGINS)
         list(APPEND MODULES_NAME ${MODULE_INC})
      else()
         string(REGEX REPLACE "(^[^/]*)/.*" "\\1" ${MODULE_INC}_TYPE ${MODULE_PATH})
      endif()
   endif()
endforeach()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/builtin.c.in ${CMAKE_CURRENT_BINARY_DIR}/builtin.c)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/fib-1.1")

# Add module paths
foreach (CURRENT_MODULE ${ALL_PLUGINS} ${ALL_MODULE_PLUGINS} ${ALL_MODULES})
   if (${CURRENT_MODULE})
      add_subdirectory( "${CMAKE_CURRENT_SOURCE_DIR}/${CURRENT_MODULE}")
   endif()
endforeach()

add_subdirectory (fib-1.1)

if(NOT ANDROID)
   set(NAVIT_START_SRC start.c)
   if(WIN32 OR WINCE AND NOT WIN_OMIT_RESOURCES)
      list(APPEND NAVIT_START_SRC ${CMAKE_CURRENT_SOURCE_DIR}/gui/win32/resources/resource.rc)
   else()
      if (APPLE)
          set(NAVIT_START_SRC start_apple.m)
      endif()
   endif()
   if (BUILD_BUNDLE)
      list(APPEND NAVIT_START_SRC resources/share)
   endif()
   add_executable(navit ${NAVIT_START_SRC})
   target_link_libraries (navit ${NAVIT_LIBNAME})
   set_target_properties(navit PROPERTIES COMPILE_DEFINITIONS "MODULE=${MODULE_NAME}")
   if (BUILD_BUNDLE)
      add_custom_command(OUTPUT resources/share COMMAND mkdir -p resources/share)
      set_source_files_properties(resources/share PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
      set_target_properties(navit PROPERTIES MACOSX_BUNDLE TRUE)
      add_dependencies(navit navit_config_xml_resource)
      add_dependencies(navit images_resource)
   endif()
endif()

if (SHARED_LIBNAVIT)
  add_library (${NAVIT_LIBNAME} SHARED ${NAVIT_SRC} )
else(SHARED_LIBNAVIT)
  add_library (${NAVIT_LIBNAME} STATIC ${NAVIT_SRC} )
endif(SHARED_LIBNAVIT)
if(NOT MSVC)
    SET(NAVIT_LIBS ${NAVIT_LIBS} m)
endif(NOT MSVC)
target_link_libraries(${NAVIT_LIBNAME}  ${MODULES_NAME} ${NAVIT_SUPPORT_LIBS} fib ${NAVIT_LIBS} ) 
set_target_properties(${NAVIT_LIBNAME} PROPERTIES COMPILE_DEFINITIONS "MODULE=navit;LIBDIR=\"${CMAKE_INSTALL_PREFIX}/${LIB_DIR}\";PREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
if (DEFINED NAVIT_COMPILE_FLAGS)
   set_target_properties(${NAVIT_LIBNAME} PROPERTIES COMPILE_FLAGS "${NAVIT_COMPILE_FLAGS}")
endif()

# Subversion revision
ADD_CUSTOM_TARGET(
   version
   ${CMAKE_COMMAND} -D SRC=${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
                    -D DST=${CMAKE_CURRENT_BINARY_DIR}/version.h
                    -D NAME=SVN_VERSION
                    -P ${PROJECT_SOURCE_DIR}/cmake/version.cmake
)

# additional parameter are passed to the stylesheet processor as parameter
macro(process_xslt SRC_XML DEST_XML)
   set(XSLT_COMMANDS COMMAND ${CMAKE_COMMAND} -E copy ${SRC_XML} ${DEST_XML}
                     COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/navit/navit.dtd ${CMAKE_CURRENT_BINARY_DIR}/navit.dtd)
   if(XSL_PROCESSING AND XSLTS)
       string(REPLACE "," ";" XSLTS "${XSLTS}")
       foreach(tmp ${XSLTS})
          set (XSLT_FILE "${PROJECT_SOURCE_DIR}/navit/xslt/${tmp}.xslt")
          list(APPEND XSLT_FILES "${XSLT_FILE}")
          list(APPEND XSLT_COMMANDS COMMAND ${CMAKE_COMMAND} -E echo  Applying ${tmp}.xslt)
          if(${XSLT_PROCESSOR} MATCHES "xsltproc")
             list(APPEND XSLT_COMMANDS COMMAND ${XSLT_PROCESSOR} ${XSLT_PROCESSOR_OPTIONS} ${XSLT_FILE} ${DEST_XML} > ${DEST_XML}.tmp)
          else()
             list(APPEND XSLT_COMMANDS COMMAND ${XSLT_PROCESSOR} ${XSLT_PROCESSOR_OPTIONS} ${DEST_XML} ${XSLT_FILE} ${ARGN} > ${DEST_XML}.tmp)
          endif()
          list(APPEND XSLT_COMMANDS COMMAND ${CMAKE_COMMAND} -E rename ${DEST_XML}.tmp ${DEST_XML})
       endforeach()
   endif()
   ADD_CUSTOM_COMMAND(
      OUTPUT ${DEST_XML}
      DEPENDS ${SRC_XML} ${XSLT_FILES}
      ${XSLT_COMMANDS}
   )
endmacro()

if(ANDROID)
   process_xslt(${CMAKE_CURRENT_SOURCE_DIR}/navit_shipped.xml ${CMAKE_CURRENT_BINARY_DIR}/navithdpi.xml OSD_SIZE=2 ICON_MEDIUM=48)
   process_xslt(${CMAKE_CURRENT_SOURCE_DIR}/navit_shipped.xml ${CMAKE_CURRENT_BINARY_DIR}/navitmdpi.xml OSD_SIZE=1.33)
   process_xslt(${CMAKE_CURRENT_SOURCE_DIR}/navit_shipped.xml ${CMAKE_CURRENT_BINARY_DIR}/navitldpi.xml OSD_SIZE=1)
   add_custom_target( navit_config_xml ALL DEPENDS navithdpi.xml navitmdpi.xml navitldpi.xml)
else()
   process_xslt(${CMAKE_CURRENT_SOURCE_DIR}/navit_shipped.xml ${CMAKE_CURRENT_BINARY_DIR}/navit.xml "")
   add_custom_target( navit_config_xml_resource DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/navit.xml COMMAND mkdir -p resources/share/navit COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/navit.xml resources/share/navit)
   add_custom_target( navit_config_xml ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/navit.xml)
endif()

ADD_DEPENDENCIES(${NAVIT_LIBNAME} version)
if (USE_LIBGNUINTL AND NOT HAVE_GLIB)
   ADD_DEPENDENCIES(support_glib intl_cmake)
endif()


add_subdirectory (maptool)
add_subdirectory (xpm)
add_subdirectory (maps)
if(ANDROID)
   add_subdirectory (android)
endif()

install(TARGETS navit
        DESTINATION ${BIN_DIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/navit.xml
        DESTINATION ${SHARE_DIR}
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

get_directory_property(INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
WRITE_FILE("cmake_plugin_settings.txt"
	"set(APPLE ${APPLE} CACHE BOOL init)\n" 
	"set(ANDROID ${ANDROID} CACHE BOOL init)\n"
	"set(USE_PLUGINS ${USE_PLUGINS} CACHE BOOL init)\n"
	"set(MODULE_BUILD_TYPE \"${MODULE_BUILD_TYPE}\" CACHE STRING init)\n"
	"set(NAVIT_COMPILE_FLAGS \"${NAVIT_COMPILE_FLAGS}\" CACHE STRING init)\n"
	"set(navit_SOURCE_DIR \"${navit_SOURCE_DIR}\" CACHE STRING init)\n"
	"set(NAVIT_LIBNAME \"${NAVIT_LIBNAME}\" CACHE STRING init)\n"
	"set(ANDROID_API_VERSION \"${ANDROID_API_VERSION}\" CACHE STRING init)\n"
	"set(ANDROID_NDK_API_VERSION \"${ANDROID_NDK_API_VERSION}\" CACHE STRING init)\n"
	"set(CMAKE_TOOLCHAIN_FILE \"${CMAKE_TOOLCHAIN_FILE}\" CACHE STRING init)\n"
	"set(INCLUDE_DIRECTORIES \"${INCLUDE_DIRECTORIES}\" CACHE STRING init)\n"
	)
