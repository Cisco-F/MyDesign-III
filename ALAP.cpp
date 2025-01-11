# include "Blif.h"
# include "ALAP.h"

static vector<a_Node*> tree_nodes;

void ALAP_Scheduling(string filepath)
{
    cout << "ALAP ordering:" << endl;
    Blif* blif = Blif_Reader(filepath);
    MyDesign* des = Elaborate(blif);
    Schedule_ALAP(des);
    Print_ALAP_Schedule(des);

    cout << endl << endl << endl;
}

// ALAP调度算法
void Schedule_ALAP(MyDesign* design) {
    
}


// 辅助函数：按照格式输出调度结果
void Print_ALAP_Schedule(MyDesign* design) {
    MyScope* scope = design->get_scope();
    vector<CELL>& cells = scope->cells;

    // 输出输入和输出端口信息
    cout << "Input :";
    for (size_t i = 0; i < scope->input_ports.size(); ++i) {
        cout << scope->input_ports[i];
        if (i < scope->input_ports.size() - 1) cout << ", ";
    }
    cout << " Output :";
    for (size_t i = 0; i < scope->output_ports.size(); ++i) {
        cout << scope->output_ports[i];
        if (i < scope->output_ports.size() - 1) cout << ", ";
    }
    cout << endl;

    // 按轮次组织每个门的调度信息
    map<int, vector<string>> and_gates, or_gates, not_gates;
    int max_cycle = 0;

    // 遍历所有单元格，按调度时间分类
    for (const auto& cell : cells) {
        int cycle = cell.scheduled_time;
        max_cycle = max(max_cycle, cycle);

        if (cell.op == '&') {
            and_gates[cycle].push_back(cell.cell_name);
        }
        else if (cell.op == '|') {
            or_gates[cycle].push_back(cell.cell_name);
        }
        else if (cell.op == '!') {
            not_gates[cycle].push_back(cell.cell_name);
        }
    }

    // 输出调度的轮次总数
    cout << "Total " << (max_cycle + 1) << " Cycles" << endl;

    // 输出每一轮的调度信息
    for (int i = 0; i <= max_cycle; ++i) {
        cout << "Cycle " << i << ":{";

        // 输出与门信息
        if (and_gates[i].empty()) {
            cout << " ";
        }
        else {
            for (size_t j = 0; j < and_gates[i].size(); ++j) {
                cout << and_gates[i][j];
                if (j < and_gates[i].size() - 1) cout << " ";
            }
        }
        cout << "},{";

        // 输出或门信息
        if (or_gates[i].empty()) {
            cout << " ";
        }
        else {
            for (size_t j = 0; j < or_gates[i].size(); ++j) {
                cout << or_gates[i][j];
                if (j < or_gates[i].size() - 1) cout << " ";
            }
        }
        cout << "},{";

        // 输出非门信息
        if (not_gates[i].empty()) {
            cout << " ";
        }
        else {
            for (size_t j = 0; j < not_gates[i].size(); ++j) {
                cout << not_gates[i][j];
                if (j < not_gates[i].size() - 1) cout << " ";
            }
        }
        cout << "}" << endl;
    }
}

void Generate_Tree_a(MyDesign* des)
{
    MyScope* scope = des->get_scope();
    vector<CELL> cells = scope->cells;

    // generate tree root
    a_Node* root = new a_Node();
    if (!root) {
        cerr << "No space!" << endl;
        exit(0);
    }
    root->node_name = "root";

    for (string s : scope->output_ports) { // add children for root
        a_Node* child = new a_Node();
        if (!child) {
            cerr << "No space!" << endl;
            exit(0);
        }
        child->node_name = s;
        child->depth = 0;
        for (CELL cell : cells) {
            if (cell.cell_name == s) {
                child->op = cell.op;
                break;
            }
        }

        // add root's children
        Generate_Subtree_a(cells, child, 1);
        tree_nodes.push_back(child);
        root->children.push_back(child);
    }
}

void Generate_Subtree_a(vector<CELL> cells, a_Node* root, int depth)
{
    string cell_output_name = root->node_name;

    vector<PORT> children;
    // search the output name in cells from the module
    for (CELL cell : cells) {
        if (cell.cell_name == cell_output_name) {
            children = cell.operands;
            break;
        }
    }

    for (PORT p : children) {
        // check if the node already exists
        a_Node* child = new a_Node();
        bool is_found = false;
        for (a_Node* node : tree_nodes) {
            if (node->node_name == p.port_name) {
                child = node;
                child->depth = depth > child->depth ? depth : child->depth;
                is_found = true;
                break;
            }
        }

        if (!is_found) {
            child->node_name = p.port_name;
            child->depth = depth;
            for (CELL cell : cells) {
                if (cell.cell_name == child->node_name) {
                    child->op = cell.op;
                    break;
                }
            }
            Generate_Subtree_a(cells, child, depth + 1);
            tree_nodes.push_back(child);
        }

        root->children.push_back(child);
    }
}
