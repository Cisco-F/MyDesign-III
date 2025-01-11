# include "Hu.h"
# include "Blif.h"

const int MAX_DEPTH = 20;

static vector<Node*> tree_nodes;

void Hu_Scheduling(string filepath)
{
	cout << "Hu's ordering:" << endl;
	Blif* blif = Blif_Reader(filepath);
	MyDesign* des = Elaborate(blif);
	Generate_Tree(des);
	Hu_Ordering();

	cout << endl << endl << endl;
}

void Generate_Tree(MyDesign* des)
{
	MyScope* scope = des->get_scope();
	vector<CELL> cells = scope->cells;

	// generate tree root
	Node* root = new Node();
	if (!root) {
		cerr << "No space!" << endl;
		exit(0);
	}
	root->node_name = "root";

	for (string s : scope->output_ports) { // add children for root
		Node* child = new Node();
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
		Generate_Subtree(cells, child, 1);
		tree_nodes.push_back(child);
		root->children.push_back(child);
	}
}

void Generate_Subtree(vector<CELL> cells, Node* root, int depth)
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
		Node* child = new Node();
		bool is_found = false;
		for (Node* node : tree_nodes) {
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
			Generate_Subtree(cells, child, depth + 1);
			tree_nodes.push_back(child);
		}

		root->children.push_back(child);
	}
}

void Hu_Ordering()
{
	// check the urgentest level
	int max_depth = 0;
	for (Node* n : tree_nodes) {
		max_depth = max(max_depth, n->depth);
	}

	cout << "Total " << max_depth << " Cycles:" << endl;

	int cycle = 0;
	while (max_depth-- > 0) {
		vector<Node*> _and, _or, _not;
		for (Node* node : tree_nodes) {
			if (node->depth == max_depth) {
				if (node->op == '&') _and.push_back(node);
				else if (node->op == '|') _or.push_back(node);
				else if (node->op == '!') _not.push_back(node);
				else continue;
			}
		}

		string and_output = "{ ";
		string or_output = and_output, not_output = and_output;
		for (Node* node : _and) {
			and_output += node->node_name + " ";
		}
		and_output += "} ,";
		for (Node* node : _or) {
			or_output += node->node_name + " ";
		}
		or_output += "} ,";
		for (Node* node : _not) {
			not_output += node->node_name + " ";
		}
		not_output += "}";

		cout << "Cycle " << cycle << ":" << and_output << " " << or_output << " " << not_output << endl;
		cycle++;
	}
}