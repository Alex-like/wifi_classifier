set( TargetName kors_mllib )
file( GLOB TargetSources src/*.cpp )
file( GLOB TargetHeaders include/*.hpp )

add_library( ${TargetName} STATIC ${TargetHeaders} ${TargetSources} )