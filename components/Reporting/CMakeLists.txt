idf_component_register(SRCS "reporting.c"
                        INCLUDE_DIRS "include" "."
                        REQUIRES "spiffs")
link_directories(COMPONENTS)