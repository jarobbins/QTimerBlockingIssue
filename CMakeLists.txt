CMAKE_MINIMUM_REQUIRED(VERSION 3.15)
PROJECT("QTimerBlockingIssue")
SET(CMAKE_BUILD_TYPE Debug)
set( CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug> )

find_package(Qt5 COMPONENTS Core REQUIRED)

get_target_property(QtCore_location Qt5::Core LOCATION)
message(QtCore_location: "${QtCore_location}")

get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

qt5_wrap_cpp( reproduce-blocking-qtimer_moc
    TestObject.hpp
    )
add_executable( reproduce-blocking-qtimer
    main.cpp
	TestObject.cpp
	${reproduce-blocking-qtimer_moc}
	)
qt5_use_modules( reproduce-blocking-qtimer Core )

add_custom_command(TARGET reproduce-blocking-qtimer POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
            --verbose 0
            --no-compiler-runtime
            \"$<TARGET_FILE:reproduce-blocking-qtimer>\"
    COMMENT "Deploying Qt..."
)
