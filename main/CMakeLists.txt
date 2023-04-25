file(GLOB_RECURSE cfile "./src/*.c") 
message(STATUS "FILE INSIDE SRC: ${cfile}")

set(INC_DIRS "inc")

idf_component_register(SRCS main.c ${cfile}
                    INCLUDE_DIRS ${INC_DIRS})