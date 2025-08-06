# Main component for Yamaha Lighting ESP32 project

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB_RECURSE MAIN_SRCS "*.cpp" "*.c")
file(GLOB_RECURSE BLOCKB_SRCS "BlockB++/src/*.cpp")
file(GLOB_RECURSE BLOCKC_SRCS "BlockC++/src/*.cpp" "BlockC++/src/*/*.cpp")

# Register main component with all sources
idf_component_register(
    SRCS 
        ${MAIN_SRCS}
        ${BLOCKB_SRCS}
        ${BLOCKC_SRCS}
    INCLUDE_DIRS 
        "."
        "BlockB++/include"
        "BlockC++/include"
        "BlockC++/include/controllers"
        "BlockC++/include/models"
        "BlockC++/include/utils"
        "Eigen"
        "nlohmann"
    REQUIRES 
        "freertos"
        "esp_common" 
        "esp_system"
)

# Add C++ specific compile options
target_compile_options(${COMPONENT_LIB} PRIVATE
    -fexceptions
    -std=c++17
    -DFORCE_TEMPLATE_INSTANTIATION=1
    -DEIGEN_NO_AUTOMATIC_RESIZING
    -DEIGEN_DONT_VECTORIZE
    -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT
    -DJSON_NOEXCEPTION
    -DJSON_USE_EXCEPTION=0
    -DUSE_FREERTOS=1
) 