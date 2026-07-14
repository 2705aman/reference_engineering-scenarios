cmake_minimum_required(VERSION 3.20)
project(reference_engineering_scenarios)

find_package(benchmark REQUIRED)

add_subdirectory(benchmark)
add_subdirectory(scenarios)
