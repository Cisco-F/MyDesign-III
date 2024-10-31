# include "Blif.h"
# include "ALAP.h"

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
    MyScope* scope = design->get_scope();
    vector<CELL>& cells = scope->cells;
    unordered_map<string, int> schedule_time; // 存储每个节点的最晚执行时间
    int max_time = 0;

    // 初始化输出节点的调度时间
    for (const auto& output_port : scope->output_ports) {
        schedule_time[output_port] = 0; // 输出节点的时间初始化为 0
    }

    // 逆拓扑遍历，计算每个节点的最晚执行时间
    for (auto it = cells.rbegin(); it != cells.rend(); ++it) {
        CELL* cell = &(*it);

        // 如果是输出节点，已经初始化
        if (schedule_time.find(cell->cell_name) != schedule_time.end()) {
            continue;
        }

        int min_time = INT_MAX; // 找到操作的最小时间
        for (const auto& operand : cell->operands) {
            // 检查每个操作数的调度时间
            if (schedule_time.find(operand.port_name) != schedule_time.end()) {
                min_time = std::min(min_time, schedule_time[operand.port_name] - 1);
            }
        }

        // 如果存在有效的最小时间，更新当前节点的调度时间
        if (min_time != INT_MAX) {
            schedule_time[cell->cell_name] = min_time;
        }
        else {
            // 如果没有依赖关系，默认赋予最大时间
            schedule_time[cell->cell_name] = max_time + 1;
        }
    }

    // 将计算的调度时间存储到设计的 cell 数据结构中
    for (auto& cell : cells) {
        if (schedule_time.find(cell.cell_name) != schedule_time.end()) {
            cell.scheduled_time = schedule_time[cell.cell_name];
        }
    }
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
