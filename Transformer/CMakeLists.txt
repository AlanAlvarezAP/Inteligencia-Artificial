cmake_minimum_required(VERSION 3.18)
project(transformer_encoder CUDA CXX)

set(CMAKE_CUDA_STANDARD 17)

# Agrega esto — path del CUDA toolkit
find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})

set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --expt-relaxed-constexpr")


add_executable(encoder
    main.cu
)

target_compile_options(encoder PRIVATE 
    $<$<COMPILE_LANGUAGE:CUDA>:-arch=sm_75>
)