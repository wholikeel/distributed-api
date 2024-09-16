
add_executable(
    geocli 
    main.cc
)

target_link_libraries(geocli
    PRIVATE
        peer
)

# target_include_directories(interpreter PUBLIC include)

add_compile_options(geocli "-std=c++23")

set_property(TARGET geocli PROPERTY CXX_STANDARD 23)
