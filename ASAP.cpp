# include "Blif.h"
# include "ASAP.h"

void ASAP_Scheduling(string filepath)
{
    cout << "ASAP ordering:" << endl;
    Blif* blif = Blif_Reader(filepath);
    MyDesign* des = Elaborate(blif);
    Schedule_ASAP(des);
    Print_ASAP_Schedule(des);

    cout << endl << endl << endl;
}

void Schedule_ASAP(MyDesign* design) {
    MyScope* scope = design->get_scope();
    vector<CELL>& cells = scope->cells;
    unordered_map<string, int> schedule_time; // �洢ÿ���ڵ������ִ��ʱ��

    // ��ʼ������ڵ�ĵ���ʱ��
    for (const auto& input_port : scope->input_ports) {
        schedule_time[input_port] = 0; // v0 ��ʼ��Ϊ 1
    }


    // ��������Ĵ������
    queue<CELL*> ready_queue;
    for (auto& cell : cells) {
        bool is_ready = true;
        for (const auto& operand : cell.operands) {
            if (schedule_time.find(operand.port_name) == schedule_time.end()) {
                is_ready = false;
                break;
            }
        }
        if (is_ready) {
            ready_queue.push(&cell);
        }
    }

    // �������в��𲽵���ÿ���ڵ�
    while (!ready_queue.empty()) {
        CELL* cell = ready_queue.front();
        ready_queue.pop();

        int max_time = 1;
        for (const auto& operand : cell->operands) {
            if (schedule_time.find(operand.port_name) != schedule_time.end()) {
                max_time = std::max(max_time, schedule_time[operand.port_name] + 1);
            }
        }
        schedule_time[cell->cell_name] = max_time;

        // ���µ��ȶ��У�ȷ�������ڵ������ɺ������ڵ���Լ�����ȶ���
        for (auto& next_cell : cells) {
            bool is_ready = true;
            for (const auto& operand : next_cell.operands) {
                if (schedule_time.find(operand.port_name) == schedule_time.end()) {
                    is_ready = false;
                    break;
                }
            }
            if (is_ready && schedule_time.find(next_cell.cell_name) == schedule_time.end()) {
                ready_queue.push(&next_cell);
            }
        }
    }

    // ������ĵ���ʱ��洢����Ƶ� cell ���ݽṹ��
    for (auto& cell : cells) {
        cell.scheduled_time = schedule_time[cell.cell_name] - 1;
    }
}


void Print_ASAP_Schedule(MyDesign* design) {
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
        cout << "Cycle " << i << ": { ";

        // ���������Ϣ
        if (and_gates[i].empty()) {
            cout << "";
        }
        else {
            for (size_t j = 0; j < and_gates[i].size(); ++j) {
                cout << and_gates[i][j];
                if (j < and_gates[i].size() - 1) cout << " ";
            }
        }
        cout << " } , { ";

        // ���������Ϣ
        if (or_gates[i].empty()) {
            cout << "";
        }
        else {
            for (size_t j = 0; j < or_gates[i].size(); ++j) {
                cout << or_gates[i][j];
                if (j < or_gates[i].size() - 1) cout << " ";
            }
        }
        cout << " } , { ";

        // ���������Ϣ
        if (not_gates[i].empty()) {
            cout << "";
        }
        else {
            for (size_t j = 0; j < not_gates[i].size(); ++j) {
                cout << not_gates[i][j];
                if (j < not_gates[i].size() - 1) cout << " ";
            }
        }
        cout << " }" << endl;
    }
}