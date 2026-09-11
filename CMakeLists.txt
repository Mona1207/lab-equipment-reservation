cmake_minimum_required(VERSION 3.16)
project(LabEquipmentReservation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# ---- 界面组(D)：界面程序额外需要 Widgets 与 Charts ----
option(BUILD_UI "Build Qt Widgets GUI (D/界面组)" ON)
if(BUILD_UI)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts)
endif()

add_library(labres_core STATIC
    src/User.cpp
    src/Student.cpp
    src/Teacher.cpp
    src/Admin.cpp
    src/DateTime.cpp
    src/Equipment.cpp
    src/Reservation.cpp
    src/MaintenanceTask.cpp
    src/ReservationManager.cpp
)

target_include_directories(labres_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(labres_core PUBLIC Qt${QT_VERSION_MAJOR}::Core)

# 界面组(D)联调补充：核心库源码里 ReservationManager.cpp 使用了局部变量名 slots，
# 与 Qt 关键字宏 slots 冲突（在补齐缺失头文件后该文件首次参与编译才暴露）。
# 仅对核心库自身关闭 Qt 旧关键字宏（PRIVATE，不外溢；界面目标仍可正常使用 signals/slots）。
target_compile_definitions(labres_core PRIVATE QT_NO_KEYWORDS)

option(BUILD_DEMO "Build command-line demo" ON)
if(BUILD_DEMO)
    add_executable(labres_demo src/main.cpp)
    target_link_libraries(labres_demo PRIVATE labres_core)
endif()

# ---- 界面组(D)：Qt Widgets 图形界面可执行程序，链接核心静态库 ----
# 界面源码全部放在 ui/ 目录，不改动 src/ 下算法组任何 .cpp；
# 关闭命令行 demo 时本目标仍可独立构建（自带 GUI 入口 ui/main.cpp）。
if(BUILD_UI)
    add_executable(labres_ui
        ui/main.cpp
        ui/AppContext.cpp
        ui/Theme.cpp
        ui/LoginWindow.cpp
        ui/MainWindow.cpp
        ui/EquipmentPage.cpp
        ui/ReserveDialog.cpp
        ui/AdminPage.cpp
        ui/StatsPage.cpp
        ui/MaintenancePage.cpp
        ui/DashboardPage.cpp
    )
    target_link_libraries(labres_ui PRIVATE
        labres_core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Charts
    )
endif()
