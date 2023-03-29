/* ~ ~ ~ ~ ~ Constant Declarations ~ ~ ~ ~ ~ */

const X_MIN_LABEL = document.querySelector("#x-min-label");
const X_MAX_LABEL = document.querySelector("#x-max-label");
const Y_MIN_LABEL = document.querySelector("#y-min-label");
const Y_MAX_LABEL = document.querySelector("#y-max-label");

const GRAPH_ELEMENT = document.querySelector("#graph");
const GRAPH_CONTEXT = GRAPH_ELEMENT.getContext("2d");

const BLUE = [36, 36, 255, 255];
const RED = [255, 0, 0, 255];
const BLACK = [0, 0, 0, 255];
const PURPLE = [255, 0, 255, 255];
const GREEN = [45, 200, 45, 255];
const ORANGE = [255, 160, 25, 255];
const BROWN = [165, 42, 42, 255];

const DEFAULT_COLORS = [BLUE, RED, BLACK, PURPLE, GREEN, ORANGE, BROWN];

const MAX_GRAPH_WIDTH = 1000;
const MAX_GRAPH_HEIGHT = 1000;

/* ~ ~ ~ ~ ~ Variable Declarations ~ ~ ~ ~ ~ */

let old_x_min = -10, old_x_max = 10, old_y_min = -10, old_y_max = 10;
var x_min = -10, x_max = 10, y_min = -10, y_max = 10;
var graph_width = GRAPH_ELEMENT.offsetWidth;
var graph_height = GRAPH_ELEMENT.offsetWidth;
var graph_dimensions_changed = false;
var axis_color = BLACK;

/* ~ ~ ~ ~ ~ Backend Graphing Functions ~ ~ ~ ~ ~ */

// rounds the given number to two decimal places
function round(num) {
    num = Math.floor(num * 100);
    return num / 100;
}

function update_graph_dimensions() {
    if(!graph_dimensions_changed) return;
    graph_dimensions_changed = false;

    _resize_graph(graph_height, graph_width, x_min, x_max, y_min, y_max); // resize in backend

    // labels
    X_MIN_LABEL.innerHTML = "x = " + round(x_min);
    X_MAX_LABEL.innerHTML = "x = " + round(x_max);
    Y_MIN_LABEL.innerHTML = "y = " + round(y_min);
    Y_MAX_LABEL.innerHTML = "y = " + round(y_max);
}

function scale_graph_window_bounds() {
    var new_graph_width = Math.min(MAX_GRAPH_WIDTH, GRAPH_ELEMENT.offsetWidth);
    var new_graph_height = Math.min(MAX_GRAPH_HEIGHT, GRAPH_ELEMENT.offsetHeight);
    GRAPH_ELEMENT.width = new_graph_width;
    GRAPH_ELEMENT.height = new_graph_height;

    if(graph_width == new_graph_width && graph_height == new_graph_height) return;

    // scale window bounds according to resize
    let horz_mid = (x_min + x_max) / 2;
    let vert_mid = (y_min + y_max) / 2;

    let horz_scale = new_graph_width / graph_width;
    let vert_scale = new_graph_height / graph_height;

    let graph_unit_width = x_max - x_min;
    let graph_unit_height = y_max - y_min;

    x_min = round(horz_mid - horz_scale * graph_unit_width / 2);
    x_max = round(horz_mid + horz_scale * graph_unit_width / 2);
    y_min = round(vert_mid - vert_scale * graph_unit_height / 2);
    y_max = round(vert_mid + vert_scale * graph_unit_height / 2);

    graph_width = new_graph_width;
    graph_height = new_graph_height;

    graph_dimensions_changed = true;
}

function draw_graph() {
    scale_graph_window_bounds();
    update_graph_dimensions();

    let graph_buffer = new Uint32Array(
        Module.HEAPU32.buffer,
        _get_graph_buffer(),
        graph_height * graph_width
    );

    let image_data = GRAPH_CONTEXT.createImageData(graph_width, graph_height);

    for(let i = 0; i < graph_buffer.length; i++) {
        if(graph_buffer[i] == 0) continue;

        for(let k = 0; k < graphed_fns.length; k++) {
            let id = graphed_fns[k].id;
            if(graph_buffer[i] & (1 << (id + 1))) {
                for(let j = 0; j < 4; j++)
                    image_data.data[i * 4 + j] = graphed_fns[k].color[j];
                break;
            }
        }

        if(graph_buffer[i] == 1) for(let j = 0; j < 4; j++) // axis
            image_data.data[i * 4 + j] = axis_color[j];
    }

    GRAPH_CONTEXT.putImageData(image_data, 0, 0);
}
