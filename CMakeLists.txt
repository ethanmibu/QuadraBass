cmake_minimum_required(VERSION 3.21)

project(QuadraBass VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show JUCE module sources in IDE projects" ON)

if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE INTERNAL "")
endif()

option(QUADRABASS_COPY_PLUGIN_AFTER_BUILD "Copy plugin artifacts to system plugin directories after build" ON)
if (DEFINED ZL_JUCE_COPY_PLUGIN)
    set(QUADRABASS_COPY_PLUGIN_AFTER_BUILD ${ZL_JUCE_COPY_PLUGIN})
endif()

if (APPLE)
    set(QUADRABASS_PLUGIN_FORMATS "AU;VST3" CACHE STRING "Phase 2 enforced formats" FORCE)
else()
    set(QUADRABASS_PLUGIN_FORMATS "VST3" CACHE STRING "Phase 2 enforced formats" FORCE)
endif()
message(STATUS "Formats are ${QUADRABASS_PLUGIN_FORMATS}")

set(JUCE_DIR "${CMAKE_SOURCE_DIR}/third_party/JUCE")
if (NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "JUCE not found at third_party/JUCE.\n"
        "Add JUCE as a submodule:\n"
        "  git submodule add https://github.com/juce-framework/JUCE.git third_party/JUCE\n"
        "  git submodule update --init --recursive\n")
endif()

add_subdirectory(third_party/JUCE)

juce_add_plugin(QuadraBass
    COMPANY_NAME "ethanmibu"
    BUNDLE_ID "com.ethanmibu.QuadraBass"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${QUADRABASS_COPY_PLUGIN_AFTER_BUILD}

    PLUGIN_MANUFACTURER_CODE EtMb
    PLUGIN_CODE QdBs

    FORMATS ${QUADRABASS_PLUGIN_FORMATS}
    PRODUCT_NAME "QuadraBass"
)

target_sources(QuadraBass PRIVATE
    src/PluginProcessor.cpp
    src/PluginProcessor.h
    src/PluginEditor.cpp
    src/PluginEditor.h
    src/util/Params.cpp
    src/util/Params.h
    src/dsp/HilbertQuadratureProcessor.cpp
    src/dsp/HilbertQuadratureProcessor.h
    src/dsp/StereoMatrixProcessor.cpp
    src/dsp/StereoMatrixProcessor.h
    src/ui/GoniometerComponent.cpp
    src/ui/GoniometerComponent.h
    src/ui/CorrelationMeter.cpp
    src/ui/CorrelationMeter.h
)

target_include_directories(QuadraBass PRIVATE
    src
)

target_compile_definitions(QuadraBass PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(QuadraBass PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp

    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

juce_generate_juce_header(QuadraBass)

if (APPLE)
    add_custom_command(TARGET QuadraBass_AU POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/QuadraBass_artefacts/JuceLibraryCode/QuadraBass_AU/Info.plist
            ${CMAKE_CURRENT_BINARY_DIR}/QuadraBass_artefacts/$<CONFIG>/AU/QuadraBass.component/Contents/Info.plist
    )

    add_custom_command(TARGET QuadraBass_VST3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/QuadraBass_artefacts/JuceLibraryCode/QuadraBass_VST3/Info.plist
            ${CMAKE_CURRENT_BINARY_DIR}/QuadraBass_artefacts/$<CONFIG>/VST3/QuadraBass.vst3/Contents/Info.plist
    )
endif()

include(CTest)
set(CTEST_OUTPUT_ON_FAILURE ON)

if (BUILD_TESTING)
    add_custom_target(quadrabass_tests)

    macro(add_qb_test test_name src_file)
        add_executable(${test_name}
            ${src_file}
            ${ARGN}
        )

        target_include_directories(${test_name} PRIVATE 
            src
            ${CMAKE_CURRENT_BINARY_DIR}/QuadraBass_artefacts/JuceLibraryCode
        )

        target_compile_definitions(${test_name} PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
        )

        target_link_libraries(${test_name} PRIVATE
            juce::juce_audio_processors
            juce::juce_dsp
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
        )

        add_test(NAME ${test_name} COMMAND ${test_name})
        add_dependencies(quadrabass_tests ${test_name})
    endmacro()

    add_qb_test(InitTests tests/InitTests.cpp src/util/Params.cpp src/util/Params.h)
    add_qb_test(ParamLayout tests/ParamLayoutTests.cpp src/util/Params.cpp src/util/Params.h)
    add_qb_test(HilbertQuadrature tests/HilbertQuadratureTests.cpp src/dsp/HilbertQuadratureProcessor.cpp src/dsp/HilbertQuadratureProcessor.h)
    add_qb_test(StereoMatrix tests/StereoMatrixTests.cpp src/dsp/StereoMatrixProcessor.cpp src/dsp/StereoMatrixProcessor.h)
    add_qb_test(VisualizerMath tests/VisualizerMathTests.cpp src/ui/GoniometerComponent.cpp src/ui/GoniometerComponent.h src/ui/CorrelationMeter.cpp src/ui/CorrelationMeter.h)
    add_qb_test(DspCompliance tests/DspComplianceTests.cpp 
        src/PluginProcessor.cpp src/PluginProcessor.h 
        src/PluginEditor.cpp src/PluginEditor.h 
        src/util/Params.cpp src/util/Params.h 
        src/dsp/HilbertQuadratureProcessor.cpp src/dsp/HilbertQuadratureProcessor.h 
        src/dsp/StereoMatrixProcessor.cpp src/dsp/StereoMatrixProcessor.h 
        src/ui/GoniometerComponent.cpp src/ui/GoniometerComponent.h 
        src/ui/CorrelationMeter.cpp src/ui/CorrelationMeter.h)
endif()
