var is_mouse_down = false;
var sensitivity = 50 / 1000;
var MIN_GRAPH_WIN_HEIGHT = 1, MIN_GRAPH_WIN_WIDTH = 1;

function mouse_move(e) {
    e.preventDefault();
    if(!is_mouse_down) return;

    let x_scale = (x_max - x_min) / 20;
    let y_scale = (y_max - y_min) / 20;

    x_min = x_min - e.movementX * x_scale * sensitivity;
    x_max = x_max - e.movementX * x_scale * sensitivity;
    y_min = y_min + e.movementY * y_scale * sensitivity;
    y_max = y_max + e.movementY * y_scale * sensitivity;

    graph_dimensions_changed = true;
}

function double_click() {
    x_min = -10;
    x_max = 10;
    y_min = -10;
    y_max = 10;

    graph_dimensions_changed = true;
}

function scroll(e) {
    e.preventDefault();

    let width = x_max - x_min;
    let height = y_max - y_min;

    let x_change = e.deltaY * width / 500;
    let y_change = e.deltaY * height / 500;

    // ensure minimum dimensions
    if(height + 2 * y_change < 0 || width + 2 * x_change < 0) return;
    if(height + 2 * y_change < MIN_GRAPH_WIN_HEIGHT && y_change < 0) return;
    if(width + 2 * x_change < MIN_GRAPH_WIN_HEIGHT && x_change < 0) return;

    y_min = y_min - y_change;
    y_max = y_max + y_change;
    x_min = x_min - x_change;
    x_max = x_max + x_change;

    graph_dimensions_changed = true;
}

GRAPH_ELEMENT.onmousemove = function(e) { mouse_move(e) };
GRAPH_ELEMENT.ondblclick = function(e) { double_click() };
GRAPH_ELEMENT.onmousewheel = function(e) { scroll(e); };
document.onmousedown = function(e) { is_mouse_down = true };
document.onmouseup = function(e) { is_mouse_down = false };
