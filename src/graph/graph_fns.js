var graphed_fns = [];

function rgba_to_hex(rgba) {
    let hex_str = "#";

    for(let i = 0; i < 3; i++) {
        let s = rgba[i].toString(16);
        hex_str += s.length == 1 ? ("0" + s) : s;
    }

    return hex_str;
}

function hex_to_rgba(hex) {
    return [
        parseInt(hex[1], 16) * 16 + parseInt(hex[2], 16),
        parseInt(hex[3], 16) * 16 + parseInt(hex[4], 16),
        parseInt(hex[5], 16) * 16 + parseInt(hex[6], 16),
        255
    ];
}

class GraphFunction {
    constructor(name, id, color) {
        this.name = name;
        this.id = id;
        this.color = color;

        this.page_elem = document.querySelector("#fn-div-template").cloneNode(true);

        this.color_input = this.page_elem.querySelector(".color-input");
        this.fn_text = this.page_elem.querySelector(".fn-text");
        this.layer_up_button = this.page_elem.querySelector(".layer-up-btn");
        this.layer_down_button = this.page_elem.querySelector(".layer-down-btn");
        this.delete_button = this.page_elem.querySelector(".del-btn");

        this.page_elem.id = "fn-div-" + this.id;
        this.page_elem.style.display = ""; // remove the display = "none" property
        this.color_input.value = rgba_to_hex(this.color);
        this.color_input.parent_fn = this;
        this.color_input.onchange = function() { this.parent_fn.color = hex_to_rgba(this.value); };
        this.fn_text.innerHTML = this.name;
        this.layer_up_button.parent_fn = this.layer_down_button.parent_fn = this;
        this.layer_up_button.onclick = function() { this.parent_fn.layer_up(); };
        this.layer_down_button.onclick = function() { this.parent_fn.layer_down(); };
        this.delete_button.parent_fn = this;
        this.delete_button.onclick = function() { this.parent_fn.destroy(); };

        document.querySelector("#function-section").appendChild(this.page_elem);
    }

    layer_up() {
        let index = graphed_fns.indexOf(this);
        if(index == 0) return;

        // reoder graphed_fns
        graphed_fns.splice(index, 1);
        graphed_fns.splice(index - 1, 0, this);

        // reorder this.page_elem.parentElement.children
        let fn_section = this.page_elem.parentElement;
        fn_section.removeChild(this.page_elem);
        fn_section.insertBefore(this.page_elem, fn_section.children[index]);
    }

    layer_down() {
        let index = graphed_fns.indexOf(this);
        if(index == graphed_fns.length - 1) return;

        // reoder graphed_fns
        graphed_fns.splice(index, 1);
        graphed_fns.splice(index + 1, 0, this);

        // reorder this.page_elem.parentElement.children
        let fn_section = this.page_elem.parentElement;
        fn_section.removeChild(this.page_elem);
        fn_section.insertBefore(this.page_elem, fn_section.children[index + 2]);
    }

    destroy() {
        remove_graph_fn(graphed_fns.indexOf(this));
    }
}

function add_graph_fn(name, id) {
    let next_default_color = DEFAULT_COLORS[graphed_fns.length % DEFAULT_COLORS.length];
    graphed_fns.push(new GraphFunction(name, id, next_default_color));
}

function remove_graph_fn(index) {
    _remove_from_graph(graphed_fns[index].id);
    let page_elem = graphed_fns[index].page_elem;
    page_elem.parentElement.removeChild(page_elem);
    graphed_fns.splice(index, 1);
}

var terminal_input_prev_val = "";
var exists_terminal_input_prev_val = false;

function display_trace_coordinates() {
    let x_p = x_min + (trace_x / graph_width * (x_max - x_min));

    if(!exists_terminal_input_prev_val) {
        exists_terminal_input_prev_val = true;
        terminal_input_prev_val = TEXT_INPUT_ELEMENT.value;
    }

    TEXT_INPUT_ELEMENT.value = "x = " + x_p;

    for(let i = 0; i < graphed_fns.length; i++) {
        let fn_txt_elem = graphed_fns[i].fn_text;
        var old_x = _calculate_text("x");
        calculate_text("x = " + x_p);
        fn_txt_elem.innerHTML = calculate_text(graphed_fns[i].name);
        console.log(graphed_fns[i].name);
        calculate_text("x = " + old_x);
    }
}

function undisplay_trace_coordinates() {
    exists_terminal_input_prev_val = false;
    TEXT_INPUT_ELEMENT.value = terminal_input_prev_val;

    for(let i = 0; i < graphed_fns.length; i++) {
        graphed_fns[i].fn_text.innerHTML = graphed_fns[i].name;
    }
}
