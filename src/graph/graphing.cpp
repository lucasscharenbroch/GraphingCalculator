#include "../calculator.h"

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Graphing Backend ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

constexpr int MAX_GRAPH_FUNCTIONS = 30; // limit to 30 because bitset is used when drawing
constexpr int MIN_TICS = 3, MAX_TICS = 30;
vector<unique_ptr<TreeNode>> graphed_functions(MAX_GRAPH_FUNCTIONS); // index corresponds to id
int graph_buffer[1000 * 1000] = {0}; // contains bitsets, each bit corresponding to a function/axis
                                     // (0th bit is axis, nth bit is graphed_functions[n - 1])
int graph_height = 1000, graph_width = 1000; // changed dynamically on browser resize
double x_min = -10, x_max = 10, y_min = -10, y_max = 10; // changed dynamically on browser resize
bool axes_enabled = true;
int tic_px = 2;

/* ~ ~ ~ ~ ~ Backend Graphing Functions ~ ~ ~ ~ ~ */

// generates a set evenly-spaced of tic-marks on powers of 10 on the range [min, max]
vector<double> get_tic_coords(double min, double max) {
    double factor = 1;
    while((max - min) * factor < MIN_TICS) factor *= 10;
    while((max - min) * factor > MAX_TICS) factor /= 10;

    vector<double> tics;

    for(long long p = min * factor; p <= max * factor; p++) {
        if((p / factor) < min || (p / factor) > max) continue;
        tics.push_back(p / factor);
    }

    return tics;
}

// connects the points in the vector (if possible) according to the graph size,
// and draws them onto the graph_buffer
void draw_point_vector(const vector<int>& vec, int index) {
    for(int j = 0; j < vec.size(); j++) {
        if(vec[j] == INT_MAX) continue; // never draw NAN
        if(vec[j] >= 0 && vec[j] < graph_height) // draw the point if it is on the canvas
            graph_buffer[vec[j] * graph_width + j] |= 2 << index;

        // draw vertical line between this and next column if possible
        if(j < vec.size() - 1 && abs(vec[j] - vec[j + 1]) > 1) {
            int low = min(vec[j], vec[j + 1]), high = max(vec[j], vec[j + 1]);
            for(int i = max(0, low); i < min(graph_height, high); i++) {
                graph_buffer[i * graph_width + (j + 1)] |= 2 << index;
            }
        }
    }
}

// draws graphed_functions[index] to graph_buffer
void draw(int index) {
    double old_x_value = get_id_value("x"); // x is used as the drawing variable - save its old val

    // x_c means "x on canvas" (uses int units), x_p means "x on plane" (uses float units)

    double x_ratio = (x_max - x_min) / (graph_width);
    double y_ratio = (graph_height) / (y_max - y_min);
    vector<int> y_c_vec; // holds the value of y_c at each x_c.

    for(int x_c = 0; x_c < graph_width; x_c++) {
        double x_p = x_min + (x_c * x_ratio);
        set_id_value("x", x_p);
        double y_p = graphed_functions[index]->eval();
        int y_c;
        if(isinf(y_p) || isnan(y_p)) y_c = INT_MAX;
        else {
            y_c = (y_p - y_min) * y_ratio; // TODO join this line and below
            y_c = graph_height - y_c; // 0 = bottom => 0 = top
        }

        y_c_vec.push_back(y_c);
        draw_point_vector(y_c_vec, index);
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

    vector<double> x_tics = get_tic_coords(x_min, x_max);
    vector<double> y_tics = get_tic_coords(y_min, y_max);

    // x axis
    if(y_0_c >= 0 && y_0_c < graph_height) {
        for(int j = 0; j < graph_width; j++) graph_buffer[y_0_c * graph_width + j] |= 1; // axis

        if(get_id_value("TICS_ENABLED"))
        for(double& x_p : x_tics) { // tics
            int x_c = (x_p - x_min) * x_ratio;
            if(x_c < 0 || x_c >= graph_width) continue;
            for(int y_c = y_0_c - tic_px; y_c <= y_0_c + tic_px; y_c++) {
                if(y_c < 0 || y_c >= graph_height) continue;
                graph_buffer[y_c * graph_width + x_c] |= 1;
            }
        }
    }

    // y axis
    if(x_0_c >= 0 && x_0_c < graph_width) {
        for(int i = 0; i < graph_height; i++) graph_buffer[i * graph_width + x_0_c] |= 1; // axis

        if(get_id_value("TICS_ENABLED"))
        for(double& y_p : y_tics) { // tics
            int y_c = (y_min - y_p) * y_ratio + graph_height;
            if(y_c < 0 || y_c >= graph_height) continue;
            for(int x_c = x_0_c - tic_px; x_c <= x_0_c + tic_px; x_c++) {
                if(x_c < 0 || x_c >= graph_width) continue;
                graph_buffer[y_c * graph_width + x_c] |= 1;
            }
        }
    }
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
