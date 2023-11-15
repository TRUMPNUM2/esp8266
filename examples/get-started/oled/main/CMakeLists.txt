
# set(include_dirs main/)
# "${include_dirs}"
set(srcs
    "oled.c" 
    )


idf_component_register(SRCS "${srcs}"  "hello_world_main.c"
                    INCLUDE_DIRS "")