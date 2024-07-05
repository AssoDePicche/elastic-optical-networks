file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/source/*.cpp)

foreach(SOURCE ${SOURCES})
  if (NOT ${SOURCE} MATCHES app.cpp)
    get_filename_component(LIBRARY_NAME ${SOURCE} NAME_WE)

    add_library(${LIBRARY_NAME} SHARED ${SOURCE})
  endif()
endforeach(SOURCE)

add_executable(${PROJECT_NAME} app.cpp ${SOURCES})
