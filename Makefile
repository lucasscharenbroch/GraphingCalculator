exported_functions := _init,_calculate_text,_get_graph_buffer,_remove_from_graph,_resize_graph,_malloc,_free
exported_runtime_functions := UTF8ToString,allocateUTF8
export_flags := -sEXPORTED_FUNCTIONS=$(exported_functions) -sEXPORTED_RUNTIME_METHODS=$(exported_runtime_functions)
calc_files := src/calc/grammar_parser.cpp src/calc/math.cpp src/calc/calc_backend.cpp src/calc/frontend.cpp src/calc/lexer.cpp
graph_files := src/graph/graphing.cpp
source_files := $(calc_files) $(graph_files)
header_files := src/calculator.h src/calc/backend.h src/calc/parser.h
flags := -sWASM=1 -sTOTAL_STACK=32mb -sTOTAL_MEMORY=64mb -sNO_DISABLE_EXCEPTION_CATCHING
optimization := -O3 # TODO change to O3 for release

bin/wasm.js bin/wasm.wasm: $(header_files) $(source_files) Makefile
	em++ $(optimization) -o bin/wasm.js $(source_files) $(export_flags) $(flags)
