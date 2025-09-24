cmake_minimum_required(VERSION 3.19)
project(smart_home LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6: Core, Widgets, Network, Sql
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network Sql)
find_package(Qt6 REQUIRED COMPONENTS Widgets SerialPort Multimedia)
qt_standard_project_setup()

# OpenCV
find_package(OpenCV REQUIRED)

qt_add_executable(smart_home
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    safety.h safety.cpp
    certified.h certified.cpp
    search.h search.cpp
    tcpclient.h tcpclient.cpp
    database.h database.cpp

)

# (선택) 실행파일 옆에 하르카스케이드 복사/설치
# 프로젝트 루트에 haarcascade_frontalface_default.xml 가 있다고 가정
set(CASCADE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/haarcascade_frontalface_default.xml)
if(EXISTS ${CASCADE_FILE})
  add_custom_command(TARGET smart_home POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CASCADE_FILE} $<TARGET_FILE_DIR:smart_home>)
  install(FILES ${CASCADE_FILE} DESTINATION .)
endif()

qt_add_resources(smart_home "resources"
    PREFIX "/"
    BASE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        res/blur.png
        res/cctv.png
        res/fire.png
        res/food.png
        res/foodwater.png
        res/home.png
        res/lock.png
        res/pet1.png
        res/poo.png
        res/sprout.png
        res/stove.png
        res/thermometer.png
        res/search.png
)

# include & link
target_include_directories(smart_home
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(smart_home
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        Qt6::SerialPort
        Qt6::Multimedia
        Qt::Sql            # ★ QMYSQL 드라이버 사용을 위해 필요
        ${OpenCV_LIBS}     # ★ OpenCV 링크
)

include(GNUInstallDirs)
install(TARGETS smart_home
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET smart_home
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
