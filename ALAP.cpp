# include "Blif.h"
# include "ALAP.h"

static vector<a_Node*> tree_nodes;
static vector<string> input_ports;

void ALAP_Scheduling(string filepath)
{
    cout << "ALAP ordering:" << endl;
    Blif* blif = Blif_Reader(filepath);
    MyDesign* des = Elaborate(blif);
	Generate_Tree_a(des);

    cout << endl << endl << endl;
}

// ALAP调度算法
void Schedule_ALAP(MyDesign* design) {
    
}


// 辅助函数：按照格式输出调度结果
void Print_ALAP_Schedule(a_Node* root) {
	unordered_set<string> visited;
	visited.insert(root->node_name);
	queue<a_Node*> q;
	q.push(root);
	int cycle = 1;
	while (!q.empty()) {
		

		vector<vector<string>> output(3, vector<string>());
        int n = q.size();
        // single layer
        for (int i = 0; i < n; i++) {
            a_Node* item = q.front();
            q.pop();

            for (a_Node* child : item->children) {
                // record each cell's type
				if (find(input_ports.begin(), input_ports.end(), child->node_name) != input_ports.end()) {
					continue;
				}
				if (visited.find(child->node_name) != visited.end()) {
					continue;
				}

                if (child->op == '&') {
                    output[0].push_back(child->node_name);
                }
                else if (child->op == '|') {
                    output[1].push_back(child->node_name);
                }
                else {
                    output[2].push_back(child->node_name);
                }

                q.push(child);
                visited.insert(child->node_name);

            }
        }
		// output the result
		if (output[0].size() == 0 && output[1].size() == 0 && output[2].size() == 0) {
			break;
		}
		string result = "Cycle " + to_string(cycle) + ": { ";
		for (string s : output[0]) {
			result += s + " ";
		}
		result += "} { ";

		for (string s : output[1]) {
			result += s + " ";
		}
		result += "} { ";

		for (string s : output[2]) {
			result += s + " ";
		}
		result += "}";
		cout << result << endl;
        cycle++;
	}
    
}

void Generate_Tree_a(MyDesign* des)
{
    MyScope* scope = des->get_scope();
    vector<CELL> cells = scope->cells;
	input_ports = scope->input_ports;

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

	Print_ALAP_Schedule(root);
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
