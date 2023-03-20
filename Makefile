exported_functions := _init,_calculate_text,_malloc,_free
exported_runtime_functions := UTF8ToString,allocateUTF8
export_flags := -sEXPORTED_FUNCTIONS=$(exported_functions) -sEXPORTED_RUNTIME_METHODS=$(exported_runtime_functions)
source_files := src/calc/grammar_parser.cpp src/calc/math.cpp src/calc/calc_backend.cpp src/calc/frontend.cpp src/calc/lexer.cpp
header_files := src/calc/calculator.h src/calc/backend.h src/calc/parser.h
flags := -sWASM=1 -sTOTAL_STACK=32mb -sTOTAL_MEMORY=64mb -sNO_DISABLE_EXCEPTION_CATCHING
optimization := -O0 # TODO change to O3 for release

bin/wasm.js bin/wasm.wasm: $(header_files) $(source_files)
	em++ $(optimization) -o bin/wasm.js $(source_files) $(export_flags) $(flags)
