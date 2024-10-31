# include "Blif.h"
# include "My_Netlist.h"

Blif* Blif_Reader(string filepath)
{
	Blif* blif = new Blif();

	ifstream file(filepath);
	if (!file.is_open()) {
		cerr << "Unable to open file!" << endl;
		exit(0);
	}

	string line;
	vector<BlifGate> gates;
	BlifGate cur_gate;

	while (getline(file, line)) {
		if (line.find(".modle") != string::npos) {
			blif->module_name = line.substr(7);
		}
		else if (line.find(".inputs") != string::npos) {
			stringstream ss(line);
			string newline = line.substr(8);
			string port;
			ss >> port;
			while (ss >> port) blif->input_list.push_back(port);
		}
		else if (line.find(".outputs") != string::npos) {   //读取输出端口
			stringstream ss(line);
			string newline = line.substr(8);
			string port;
			ss >> port;
			while (ss >> port) blif->output_list.push_back(port);
		}
		else if (line.find(".names") != string::npos) {
			if (!cur_gate.gate_port.empty()) {    //再次读取到.names行，上一个门电路定义结束
				gates.push_back(cur_gate);
				cur_gate = BlifGate();
			}
			// cur_gate.gate_port.push_back(line);
			stringstream ss(line);
			string port;
			ss >> port;
			while (ss >> port) {
				cur_gate.gate_port.push_back(port);
			}
		}
		else if (line.find(".end") != string::npos) {       //文件结尾
			// cout << "EOF" << endl;
		}
		else {                                               //读取门电路定义细节
			cur_gate.gate_def.push_back(line);
		}
	}

	if (!cur_gate.gate_port.empty()) {
		gates.push_back(cur_gate);
	}

	blif->blifGates = gates;

	file.close();
	return blif;
}

MyDesign* Elaborate(Blif* blif)
{
	MyDesign* des = new MyDesign();
	MyScope* scope = des->get_scope();
	
	scope->scope_name = blif->module_name;
	scope->input_ports = blif->input_list;
	scope->output_ports = blif->output_list;

	vector<CELL> cells;
	vector<BlifGate> gates = blif->blifGates;
	for (BlifGate g : gates) {
		if (g.gate_port.size() == 2) {	//非门
			PORT inport = { g.gate_port[0], IN };
			CELL cell;
			cell.operands.push_back(inport);
			cell.op = '!';
			cell.cell_name = g.gate_port[1];
			cells.push_back(cell);
		}
		else {
			CELL cell;
			cell.cell_name = g.gate_port[g.gate_port.size() - 1];
			if (g.gate_def.size() == 1) cell.op = '&';	//与门
			else cell.op = '|';

			for (int i = 0; i < g.gate_port.size() - 1; i++) {
				PORT p = { g.gate_port[i], IN };
				cell.operands.push_back(p);
			}
			cells.push_back(cell);
		}
	}

	scope->cells = cells;
	return des;
}

void Update_Conns(MyDesign* des)
{
	MyScope* scope = des->get_scope();
	vector<CELL>& cells = scope->cells;

	for (CELL& cell : cells) {
		vector<PORT> ports = cell.operands;
		for (PORT& p : ports) {
			string port_name = p.port_name;

			//端口是模块的输入端口，则其一定没有子树
			vector<string> input_ports = scope->input_ports;	
			if (find(input_ports.begin(), input_ports.end(), port_name) != input_ports.end()) continue;
			
			for (CELL& child_cell : cells) {
				if (child_cell.cell_name == port_name) {
					cell.cell_conns[port_name] = child_cell;
					break;
				}
			}
		}
	}
}
