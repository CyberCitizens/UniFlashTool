# dependencies to build
# sudo apt update && sudo apt install qt6-base-dev qt6-tools-dev qt6-tools-dev-tools cmake build-essential libcurlpp-dev
cmake -S . -B build/ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/ --parallel $(("$(nproc)" - 2))