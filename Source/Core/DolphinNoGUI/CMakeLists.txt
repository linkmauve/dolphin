set(NOGUI_SRCS Main.cpp)

if(USE_GLFW)
	set(NOGUI_SRCS ${NOGUI_SRCS} GLFW.cpp)
elseif(USE_X11)
	set(NOGUI_SRCS ${NOGUI_SRCS} X11.cpp ../DolphinWX/X11Utils.cpp)
else()
	message(FATAL_ERROR "Either GLFW or X11 should have been selected for the nogui build.")
endif()

list(APPEND LIBS core uicommon)

if(APPLE)
	set(DOLPHIN_EXE Dolphin-nogui)
else()
	set(DOLPHIN_EXE dolphin-emu-nogui)
endif()

add_executable(${DOLPHIN_EXE} ${SRCS} ${NOGUI_SRCS})
target_link_libraries(${DOLPHIN_EXE} ${LIBS})
set(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_EXECUTABLES} ${DOLPHIN_EXE})
install(TARGETS ${DOLPHIN_EXE} RUNTIME DESTINATION ${bindir})
