project(storage_manager)

FILE(GLOB_RECURSE header_files CONFIGURE_DEPENDS include/*.hpp include/*.h)
FILE(GLOB_RECURSE source_files CONFIGURE_DEPENDS src/*.cpp)

message(STATUS "${PROJECT_NAME} Configuration")
message(STATUS "---- Header Files:")
    foreach(header ${header_files})
        message(STATUS "----\t ${header}")
    endforeach()
message(STATUS "---- Source Files:")
    foreach(source ${source_files})
        message(STATUS "----\t ${source}")
    endforeach()

add_library(${PROJECT_NAME} STATIC ${header_files} ${source_files})

target_include_directories(${PROJECT_NAME} PUBLIC include)