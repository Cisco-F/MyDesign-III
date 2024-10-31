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

// ALAP�����㷨
void Schedule_ALAP(MyDesign* design) {
    MyScope* scope = design->get_scope();
    vector<CELL>& cells = scope->cells;
    unordered_map<string, int> schedule_time; // �洢ÿ���ڵ������ִ��ʱ��
    int max_time = 0;

    // ��ʼ������ڵ�ĵ���ʱ��
    for (const auto& output_port : scope->output_ports) {
        schedule_time[output_port] = 0; // ����ڵ��ʱ���ʼ��Ϊ 0
    }

    // �����˱���������ÿ���ڵ������ִ��ʱ��
    for (auto it = cells.rbegin(); it != cells.rend(); ++it) {
        CELL* cell = &(*it);

        // ���������ڵ㣬�Ѿ���ʼ��
        if (schedule_time.find(cell->cell_name) != schedule_time.end()) {
            continue;
        }

        int min_time = INT_MAX; // �ҵ���������Сʱ��
        for (const auto& operand : cell->operands) {
            // ���ÿ���������ĵ���ʱ��
            if (schedule_time.find(operand.port_name) != schedule_time.end()) {
                min_time = std::min(min_time, schedule_time[operand.port_name] - 1);
            }
        }

        // ���������Ч����Сʱ�䣬���µ�ǰ�ڵ�ĵ���ʱ��
        if (min_time != INT_MAX) {
            schedule_time[cell->cell_name] = min_time;
        }
        else {
            // ���û��������ϵ��Ĭ�ϸ������ʱ��
            schedule_time[cell->cell_name] = max_time + 1;
        }
    }

    // ������ĵ���ʱ��洢����Ƶ� cell ���ݽṹ��
    for (auto& cell : cells) {
        if (schedule_time.find(cell.cell_name) != schedule_time.end()) {
            cell.scheduled_time = schedule_time[cell.cell_name];
        }
    }
}


// �������������ո�ʽ������Ƚ��
void Print_ALAP_Schedule(MyDesign* design) {
    MyScope* scope = design->get_scope();
    vector<CELL>& cells = scope->cells;

    // ������������˿���Ϣ
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

    // ���ִ���֯ÿ���ŵĵ�����Ϣ
    map<int, vector<string>> and_gates, or_gates, not_gates;
    int max_cycle = 0;

    // �������е�Ԫ�񣬰�����ʱ�����
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

    // ������ȵ��ִ�����
    cout << "Total " << (max_cycle + 1) << " Cycles" << endl;

    // ���ÿһ�ֵĵ�����Ϣ
    for (int i = 0; i <= max_cycle; ++i) {
        cout << "Cycle " << i << ":{";

        // ���������Ϣ
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

        // ���������Ϣ
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

        // ���������Ϣ
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
