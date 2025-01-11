# include "ILP.h"
# include "Blif.h"
# include "ASAP.h"

void generate_ilp(string filepath) {
	Blif* blif = Blif_Reader(filepath);
	MyDesign* des = Elaborate(blif);
	Schedule_ASAP(des);

    MyScope* scope = des->get_scope();
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
}