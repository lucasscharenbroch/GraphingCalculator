var x_min = -10, x_max = 10, y_min = -10, y_max = 10;

const X_MIN_LABEL = document.querySelector("#x-min-label");
const X_MAX_LABEL = document.querySelector("#x-max-label");
const Y_MIN_LABEL = document.querySelector("#y-min-label");
const Y_MAX_LABEL = document.querySelector("#y-max-label");

const GRAPH_ELEMENT = document.querySelector("#graph");

var old_graph_width = GRAPH_ELEMENT.offsetWidth;
var old_graph_height = GRAPH_ELEMENT.offsetWidth;

// truncates the given float's decimal digits except the first two
function round(num) {
    num = Math.floor(num * 100);
    return num / 100;
}

function update_window_bound_labels() {
    X_MIN_LABEL.innerHTML = "x = " + x_min;
    X_MAX_LABEL.innerHTML = "x = " + x_max;
    Y_MIN_LABEL.innerHTML = "y = " + y_min;
    Y_MAX_LABEL.innerHTML = "y = " + y_max;
}

function scale_graph_window_bounds() {
    var new_graph_width = GRAPH_ELEMENT.offsetWidth;
    var new_graph_height = GRAPH_ELEMENT.offsetWidth;

    // scale window bounds according to resize
    if(old_graph_width != new_graph_width || old_graph_height != new_graph_height) {
        let horz_mid = (x_min + x_max) / 2;
        let vert_mid = (y_min + y_max) / 2;

        let horz_scale = new_graph_width / old_graph_width;
        let vert_scale = new_graph_height / old_graph_height;

        x_min = round(horz_mid - horz_scale * (x_max - x_min) / 2);
        x_max = round(horz_mid + horz_scale * (x_max - x_min) / 2);
        y_min = round(vert_mid - vert_scale * (y_max - y_min) / 2);
        y_max = round(vert_mid + vert_scale * (y_max - y_min) / 2);

        old_graph_width = new_graph_width;
        old_graph_height = new_graph_height;
    }
    update_window_bound_labels();
}

function draw_graph() {
    scale_graph_window_bounds();
    console.log("drawaing graph");

    // TODO call function from WASM, copy buffer to canvas
}
