// reads a cstring into a javascript string, disposes of the string (with free),
// then returns the javascript string.
function decode_cstr(str_ptr) {
    let out = Module.UTF8ToString(str_ptr);
    _free(str_ptr);
    return out;
}

function calculate_text(text) {
    let str_ptr = Module.allocateUTF8(text); // create a c-string out of text
    let result = decode_cstr(_calculate_text(str_ptr));
    _free(str_ptr);
    return result;
}

/* ~ ~ ~ ~ ~ Runtime ~ ~ ~ ~ ~ */

Module.onRuntimeInitialized = function() { main(); }; // wait for WASM before running main


function main() {
    // TODO
}
