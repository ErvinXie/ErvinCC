cmake_minimum_required(VERSION 3.15)
project(ErvinCC)

set(CMAKE_CXX_STANDARD 17)

add_executable(compile compile.cpp)
add_executable(generate_forms utils/utils.cpp parser/form-generator/grammar.cpp)
add_executable(scanner scanner/scanner.cpp utils/utils.cpp)
add_executable(parser parser/parser/LR-parser.cpp utils/utils.cpp parser/parser/cst_node.cpp)
add_executable(semantic semantic/semantic.cpp semantic/table.cpp parser/parser/cst_node.cpp)
add_executable(codegen codegen/codegen.cpp utils/utils.cpp)
