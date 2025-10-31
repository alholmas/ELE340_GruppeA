cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME sensor_styrenode)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable target (no sources listed yet; sources are added later)
add_executable(${CMAKE_PROJECT_NAME})

# Preprocessor symbols used across the project (defines for HAL and build config)
set(PROJECT_Defines_Syms 
	USE_FULL_LL_DRIVER
    # USE_HAL_DRIVER
    USE_FULL_ASSERT
    # HSE_VALUE=8000000 
	# HSE_STARTUP_TIMEOUT=100 
	# LSE_STARTUP_TIMEOUT=5000 
	# LSE_VALUE=32768 
	# EXTERNAL_CLOCK_VALUE=8000000 
	# HSI_VALUE=8000000 
	# LSI_VALUE=40000
    # VDD_VALUE=3300 # Må kommenteres ved bruk av HAL
	# PREFETCH_ENABLE=1 # Må kommenteres ved bruk av HAL
	
    STM32F303xC
    $<$<CONFIG:Debug>:DEBUG>
)


# STM32CubeMX generated include paths
set(PROJECT_Include_Dirs
    # Application includes
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    # LL drivers includes
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_LL_Driver/Inc
    # HAL drivers includes
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Inc
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Inc/Legacy
    # CMSIS includes
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F3xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
)

# STM32CubeMX generated application sources
set(PROJECT_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/stm32f3xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/startup/sysmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/startup/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Startup/startup_stm32f303xc.s
    # HAL MSP file (kommenter ut hvis bare LL brukes)
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/stm32f3xx_hal_msp.c
    # User-defined source files
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/sensorNode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/styreNode.c

    # 
    
)

# List of HAL and low-level driver source files required by the platform
set(Drivers_Src
    # LL drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/system_stm32f3xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_LL_Driver/Src/stm32f3xx_ll_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_LL_Driver/Src/stm32f3xx_ll_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_LL_Driver/Src/stm32f3xx_ll_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_LL_Driver/Src/stm32f3xx_ll_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_LL_Driver/Src/stm32f3xx_ll_adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_LL_Driver/Src/stm32f3xx_ll_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_LL_Driver/Src/stm32f3xx_ll_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_LL_Driver/Src/stm32f3xx_ll_usart.c

    # HAL drivers (kommenter ut hvis bare LL brukes)
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rcc.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rcc_ex.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_gpio.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_dma.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_cortex.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pwr.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pwr_ex.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_flash.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_flash_ex.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2c.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2c_ex.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_exti.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_ll_rcc.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_spi.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_spi_ex.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pcd.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pcd_ex.c
)


# Configure optional library search paths (empty by default)
set(PROJECT_LINK_DIRS
)

# Libraries to link into the final executable (Drivers plus toolchain libs)
set(PROJECT_LINK_LIBS 
    Drivers
    ${TOOLCHAIN_LINK_LIBRARIES}
)

# Small interface library that provides include paths and compile definitions
add_library(LIB INTERFACE)
target_include_directories(LIB INTERFACE ${PROJECT_Include_Dirs})
target_compile_definitions(LIB INTERFACE ${PROJECT_Defines_Syms})

# Object library that compiles the HAL/driver sources; linked into executable
add_library(Drivers OBJECT)
target_sources(Drivers PRIVATE ${Drivers_Src})
target_link_libraries(Drivers PUBLIC LIB)


# Add application and startup sources to the executable target
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_Application_Src})

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_LINK_DIRS})

# Add libraries to the project
target_link_libraries(${CMAKE_PROJECT_NAME} ${PROJECT_LINK_LIBS})

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)

# Validate that code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()