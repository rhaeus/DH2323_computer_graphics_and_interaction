cmake_minimum_required (VERSION 3.1)
project (vectorLab)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )
# include_directories( ${OPENGL_INCLUDE_DIRS}  )


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



set(MY_HEADER_FILES
	vector.h

)

set(MY_SOURCE_FILES
	vector.cpp
	vectorLab.cpp
	
)


include_directories(
	# libs/
	# include/gl
	.
)

add_executable(vectorLab
	${MY_HEADER_FILES}
	${MY_SOURCE_FILES}
)

target_link_libraries(vectorLab ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} )

# set(ALL_LIBS
#     ${OPENGL_LIBRARY}
# 	${GLUT_LIBRARY}
# )

# target_link_libraries(vectorLab
#     ${ALL_LIBS}
# )
