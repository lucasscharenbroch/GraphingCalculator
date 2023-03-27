#include "../calculator.h"

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Graphing Backend ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

constexpr int MAX_GRAPH_FUNCTIONS = 30; // limit to 30 because bitset is used when drawing
vector<unique_ptr<TreeNode>> graphed_functions(MAX_GRAPH_FUNCTIONS); // index corresponds to id
int graph_buffer[1000 * 1000] = {0}; // contains bitsets, each bit corresponding to a function/axis
                                     // (0th bit is axis, nth bit is graphed_functions[n - 1])
int graph_height = 1000, graph_width = 1000; // changed dynamically on browser resize
double x_min = -10, x_max = 10, y_min = -10, y_max = 10; // changed dynamically on browser resize
bool axes_enabled = true;

/* ~ ~ ~ ~ ~ Backend Graphing Functions ~ ~ ~ ~ ~ */

// draws graphed_functions[index] to graph_buffer
void draw(int index) { // TODO improve drawing algorithm to better support vertical lines
    double old_x_value = get_id_value("x"); // x is used as the drawing variable - save its old val

    // x_c means "x on canvas" (uses int units), x_p means "x on plane" (uses float units)

    double x_ratio = (x_max - x_min) / (graph_width);
    double y_ratio = (graph_height) / (y_max - y_min);

    for(int x_c = 0; x_c < graph_width; x_c++) {
        double x_p = x_min + (x_c * x_ratio);
        set_id_value("x", x_p);
        double y_p = graphed_functions[index]->eval();
        int y_c = (y_p - y_min) * y_ratio;
        y_c = graph_height - y_c; // 0 = bottom => 0 = top
        if(y_c >= 0 && y_c < graph_height)
            graph_buffer[y_c * graph_width + x_c] |= (2 << index);
    }

    set_id_value("x", old_x_value);
}

// entirely removes graphed_functions[index] from graph_buffer
void undraw(int index) {
    for(int i = 0; i < graph_height; i++) {
        for(int j = 0; j < graph_width; j++) {
            graph_buffer[i * graph_width + j] &= ~(2 << index);
        }
    }
}

void draw_axes() {
    double x_ratio = (graph_width) / (x_max - x_min) ;
    double y_ratio = (graph_height) / (y_max - y_min);

    int x_0_c = -x_min * x_ratio; // x = 0 on canvas pixel
    int y_0_c = y_min * y_ratio + graph_height; // y = 0 on canvas pixel

    // x axis
    if(y_0_c >= 0 && y_0_c < graph_height)
    for(int j = 0; j < graph_width; j++) graph_buffer[y_0_c * graph_width + j] |= 1;

    // y axis
    if(x_0_c >= 0 && x_0_c < graph_width)
    for(int i = 0; i < graph_height; i++) graph_buffer[i * graph_width + x_0_c] |= 1;
}

void undraw_axes() {
    for(int i = 0; i < graph_height; i++) {
        for(int j = 0; j < graph_width; j++) {
            graph_buffer[i * graph_width + j] &= ~1;
        }
    }
}

/* ~ ~ ~ ~ ~ Frontend Graphing Functions ~ ~ ~ ~ ~ */

int *get_graph_buffer() {
    return graph_buffer;
}

// attempts to add the given parsing-tree-node-expression to the graph;
// return false if graph is full (graphed_functions has no nullptr elements)
bool add_to_graph(unique_ptr<TreeNode>&& expr) {
    for(int i = 0; i < MAX_GRAPH_FUNCTIONS; i++) {
        if(graphed_functions[i] == nullptr) {
            emscripten_run_script(("add_graph_fn(\"" + expr->to_string() +
                                   "\", " + to_string(i) + ")").data());
            graphed_functions[i] = std::move(expr);
            draw(i);
            return true;
        }
    }

    return false;
}

// ungraphs and erases graphed_functions[index]
bool remove_from_graph(int index) {
    if(graphed_functions[index] == nullptr) return false;

    undraw(index);
    graphed_functions[index].reset(); // destruct graphed_functions[index]; set it to nullptr
    return true;
}

// undraws all functions, resizes the graph, then draws the functions again.
void resize_graph(int new_height, int new_width, double new_x_min, double new_x_max,
                                                 double new_y_min, double new_y_max) {
    for(int i = 0; i < MAX_GRAPH_FUNCTIONS; i++) if(graphed_functions[i] != nullptr) undraw(i);
    if(axes_enabled) undraw_axes();

    graph_height = new_height, graph_width = new_width;
    x_min = new_x_min, x_max = new_x_max, y_min = new_y_min, y_max = new_y_max;

    if(axes_enabled) draw_axes();
    for(int i = 0; i < MAX_GRAPH_FUNCTIONS; i++) if(graphed_functions[i] != nullptr) draw(i);
}

void toggle_axes() {
    axes_enabled = !axes_enabled;

    if(axes_enabled) draw_axes();
    else undraw_axes();
}
