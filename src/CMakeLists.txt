# CMakeList.txt : CMake project for Viper, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (Viper "main.cpp" "Viper.h" "objects/object.h" "config.h" "core/vimem.h" "core/vimem.cpp" "port.h" "objects/object.cpp" "objects/stringobject.h" "objects/stringobject.cpp" "objects/intobject.h" "objects/intobject.cpp" "objects/floatobject.h" "objects/floatobject.cpp" "objects/bytesarrayobject.h" "objects/bytesarrayobject.cpp" "objects/codeobject.h" "objects/codeobject.cpp" "objects/tupleobject.h" "objects/tupleobject.cpp" "core/vistatus.h" "core/vistatus.cpp" "objects/listobject.h" "objects/listobject.cpp" "parser/token.h" "parser/token.cpp"    "core/viperrun.h" "core/viperrun.cpp" "core/errorcode.h" "core/thread.h" "core/thread.cpp" "core/runtime.h" "core/runtime.cpp" "core/interpreter.h" "core/error.h" "core/error.cpp" "core/interpreter.cpp" "parser/ast.h" "parser/ast.cpp" "parser/tokenizer.h" "parser/tokenizer.cpp" "parser/parser.h" "parser/parser.cpp" "parser/vigen.h" "parser/vigen.cpp" "core/visys.h" "core/visys.cpp" "objects/complexobject.h" "objects/complexobject.cpp" "core/victype.h" "core/victype.cpp" "core/viarena.h" "core/viarena.cpp"   "patchlevel.h"   "core/viconfig.h" "core/viconfig.cpp" "objects/boolobject.h" "objects/boolobject.cpp"   "parser/stringparser.h" "parser/stringparser.cpp")

# TODO: Add tests and install targets if needed.
