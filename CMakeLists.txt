set( TargetName kors_wifi_classifier )
set(TargetSources
    src/interlayer.cpp
    src/GlobalSource.cpp
)
set(TargetHeaders
    include/interlayer.hpp
    include/GlobalSource.hpp
)

add_subdirectory( libs/frameslib )
#install( TARGETS kors_frameslib
#    RUNTIME DESTINATION ${PROJECT_INSTALL_DIR}
#    LIBRARY DESTINATION ${PROJECT_INSTALL_DIR}
#    COMPONENT ${TargetName} 
#)

add_subdirectory( libs/mllib )
#install( TARGETS kors_mllib
#    RUNTIME DESTINATION ${PROJECT_INSTALL_DIR}
#    LIBRARY DESTINATION ${PROJECT_INSTALL_DIR}
#    COMPONENT ${TargetName} 
#)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include )

add_library( ${TargetName} STATIC ${TargetHeaders} ${TargetSources} )
target_link_libraries( ${TargetName} PRIVATE kors_mllib kors_frameslib )