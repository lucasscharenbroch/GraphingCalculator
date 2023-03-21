const TEXT_INPUT_ELEMENT = document.querySelector("#text-input");
const TEXT_OUTPUT_ELEMENT = document.querySelector("#text-output");

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

function clear_screen() {
    TEXT_OUTPUT_ELEMENT.textContent = "";
    screen_is_cleared = true;
}

function handle_text_input_keypress(e) {
    if(screen_is_cleared) screen_is_cleared = false;
    if(e.code == "Enter") {
        let input_text = TEXT_INPUT_ELEMENT.value;
        TEXT_INPUT_ELEMENT.value = "";
        TEXT_OUTPUT_ELEMENT.textContent += input_text + "\n";
        TEXT_OUTPUT_ELEMENT.scrollTop = TEXT_OUTPUT_ELEMENT.scrollHeight; // scroll to input
        let result = calculate_text(input_text);
        if(!screen_is_cleared) TEXT_OUTPUT_ELEMENT.textContent += result + "\n";
        TEXT_OUTPUT_ELEMENT.scrollTop = TEXT_OUTPUT_ELEMENT.scrollHeight; // scroll to result
    }
    // TODO add arrow-key-to-previous-command support
}
TEXT_INPUT_ELEMENT.addEventListener("keydown", handle_text_input_keypress);

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Runtime ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

var screen_is_cleared = false; // this flag is set after clearing the screen so that the clear()
                               // function doesn't print a redundant "nan" (since it's indirectly
                               // called from WASM).

Module.onRuntimeInitialized = function() { main(); }; // wait for WASM before running main

function main() {
    _init();
    setInterval(draw_graph, 100);
}
