cmake_minimum_required(VERSION 3.20)
project(CnCGeneralsZeroHourConsumer LANGUAGES CXX)

find_package(CnCGeneralsZeroHour CONFIG REQUIRED)

add_executable(cnc_package_consumer main.cpp)
target_compile_features(cnc_package_consumer PRIVATE cxx_std_17)
target_link_libraries(cnc_package_consumer PRIVATE cnc::game_session)
