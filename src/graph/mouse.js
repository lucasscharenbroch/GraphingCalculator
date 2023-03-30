var is_mouse_down = false;
var trace_mode_enabled = false;
var trace_x;
var pan_sensitivity = 50 / 1000;
var zoom_sensitivity = 10 / 1000;
var MIN_GRAPH_WIN_HEIGHT = .1, MIN_GRAPH_WIN_WIDTH = .1;

function mouse_move(e) {
    e.preventDefault();

    if(trace_mode_enabled) trace_x = e.offsetX; // Update trace-line

    if(!is_mouse_down) return;

    // Panning

    let x_scale = (x_max - x_min) / 20;
    let y_scale = (y_max - y_min) / 20;

    x_min = x_min - e.movementX * x_scale * pan_sensitivity;
    x_max = x_max - e.movementX * x_scale * pan_sensitivity;
    y_min = y_min + e.movementY * y_scale * pan_sensitivity;
    y_max = y_max + e.movementY * y_scale * pan_sensitivity;

    graph_dimensions_changed = true;
}

function double_click() {
    x_min = -10;
    x_max = 10;
    y_min = -10;
    y_max = 10;

    graph_dimensions_changed = true;
}

function right_click(e) {
    e.preventDefault();
    if(!trace_mode_enabled) {
        trace_mode_enabled = true;
        trace_x = e.offsetX;
        return;
    }
    trace_mode_enabled = false;
}

function scroll(e) {
    console.log(e);
    e.preventDefault();

    let width = x_max - x_min;
    let height = y_max - y_min;

    let x_change = e.deltaY * width * zoom_sensitivity;
    let y_change = e.deltaY * height * zoom_sensitivity;

    // bias the change towards the position of the cursor
    let x_bias = e.offsetX / GRAPH_ELEMENT.width;
    let y_bias = e.offsetY / GRAPH_ELEMENT.height;
    console.log(x_bias, y_bias, x_change, y_change);

    // ensure minimum dimensions
    if(height + 2 * y_change < 0 || width + 2 * x_change < 0) return;
    if(height + 2 * y_change < MIN_GRAPH_WIN_HEIGHT && y_change < 0) return;
    if(width + 2 * x_change < MIN_GRAPH_WIN_HEIGHT && x_change < 0) return;

    y_min = y_min - y_change * (1 - y_bias);
    y_max = y_max + y_change * y_bias;
    x_min = x_min - x_change * x_bias;
    x_max = x_max + x_change * (1 - x_bias);

    graph_dimensions_changed = true;
}

GRAPH_ELEMENT.onmousemove = function(e) { mouse_move(e) };
GRAPH_ELEMENT.ondblclick = function(e) { double_click() };
GRAPH_ELEMENT.onmousewheel = function(e) { scroll(e); };
GRAPH_ELEMENT.oncontextmenu = function(e) { right_click(e); };
document.onmousedown = function(e) { is_mouse_down = true };
document.onmouseup = function(e) { is_mouse_down = false };
