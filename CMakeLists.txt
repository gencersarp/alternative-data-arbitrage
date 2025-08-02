cmake_minimum_required(VERSION 3.10)
project(AlternativeDataArbitrage)

set(CMAKE_CXX_STANDARD 17)

# Find and configure the cURL library
find_package(CURL REQUIRED)

# Add the include directory for our JSON library
include_directories(include)

add_executable(arbitrage_engine src/main.cpp)

# Link the cURL library to our executable
target_link_libraries(arbitrage_engine PRIVATE CURL::libcurl)
