
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(rstartree)


find_package(VTK COMPONENTS 
CommonColor
CommonCore
FiltersSources
InteractionStyle
RenderingContextOpenGL2
RenderingCore
RenderingFreeType
RenderingGL2PSOpenGL2
RenderingOpenGL2
)

include_directories("${CMAKE_CURRENT_LIST_DIR}/src")

if (NOT VTK_FOUND)
  message(FATAL_ERROR "rstartree: Unable to find the VTK build folder.")
endif()

set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")
add_executable(rstartree MACOSX_BUNDLE main.cxx )
  target_link_libraries(rstartree PRIVATE ${VTK_LIBRARIES}
)
vtk_module_autoinit(
  TARGETS rstartree
  MODULES ${VTK_LIBRARIES}
)
