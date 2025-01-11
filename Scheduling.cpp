# include "Scheduling.h"
# include "ASAP.h"
# include "ALAP.h"
# include "Hu.h"
# include "ML_RCS.h"
# include "MR_LCS.h"
# include "Verilog.h"
# include "Blif.h"
# include <algorithm>

const string main_menu = 
    "****************************************\n"
    "*           Scheduling Tool            *\n"
    "****************************************\n"
    "*          1. select file              *\n"
    "*          2. exit                     *\n"
    "*          3. Scheduling Verilog       *\n"
    "****************************************\n";

const string schedule_menu = 
    "****************************************\n"
    "*          Scheduling Tool             *\n"
    "****************************************\n"
    "*         1. ASAP Scheduling           *\n"
    "*         2. ALAP Scheduling           *\n"
    "*         3. Hu Scheduling             *\n"
    "*         4. ML_RCS                    *\n"
    "*         5. MR_LCS                    *\n"
    "*         6. Gerenate lp file          *\n"
    "*         7. exit                      *\n"
    "****************************************\n";

void print_main_menu() {
    cout << main_menu << endl << endl;
}

void print_schedule_menu() {
    cout << schedule_menu << endl << endl;
}

void menu()
{
    int opt;
    string filepath;
    while (true) {
        print_main_menu();
        cout << "Please enter a number: ";
        cin >> opt;
        system("cls");
        switch (opt) {
            case 1: {
                filepath = select_file();
                cout << filepath << endl;
                Schedule_menu(filepath);
                break;
            }
            case 2: {
                cout << "Thanks for using!" << endl;
                exit(0);
            }
            case 3: {
                string verilog_file = select_file();  // ѡ�� Verilog �ļ�
                if (!verilog_file.empty()) {
                    // ת�� Verilog �ļ�Ϊ BLIF �ļ�
                    string blif_filepath = convert_verilog_to_blif(verilog_file);
                    // ���õ��Ȳ˵����������ɵ� BLIF �ļ�·��
                    if (!blif_filepath.empty()) {
                        Schedule_menu(blif_filepath);
                    }
                }
                break;
            }
            default: {
                cout << "Invalid input!" << endl;
            }
        }
    }
}

string select_file()
{
    OPENFILENAME ofn;
    WCHAR szFile[260];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        std::wstring wideFilePath = ofn.lpstrFile;
        std::string filepath(wideFilePath.begin(), wideFilePath.end());
        return filepath;
    }
    else {
        MessageBox(NULL, L"δѡ���κ��ļ�", L"��ʾ", MB_OK);
        return "\0";
    }
}

void Schedule_menu(string filepath)
{
    int opt;
    print_schedule_menu();
    cout << "You can continue or enter number 7 to exit: ";
    while (true) {
        cin >> opt;
        system("cls");
        switch (opt) {
            case 1: {
                print_schedule_menu();
                ASAP_Scheduling(filepath);
                cout << "You can continue or enter number 7 to exit: ";
                break;
            }
            case 2: {
                print_schedule_menu();
                ALAP_Scheduling(filepath);
                cout << "You can continue or enter number 7 to exit: ";
                break;
            }
            case 3: {
                print_schedule_menu();
                Hu_Scheduling(filepath);
                cout << "You can continue or enter number 7 to exit: ";
                break;
            }
            case 4: {
                print_schedule_menu();
                ML_RCS_Scheduling(filepath);
                cout << "You can continue or enter number 7 to exit: ";
                break;
            }
            case 5: {
                print_schedule_menu();
                MR_LCS_Scheduling(filepath);
                cout << "You can continue or enter number 7 to exit: ";
                break;
            }
            case 6: {
                print_schedule_menu();
                generate_ilp(filepath);
                cout << "lp file has been saved as \"output.lp\"" << endl;
                cout << "You can continue or enter number 7 to exit: ";
                break;
            }
            case 7: {
                cout << "Thanks for using!" << endl;
                exit(0);
            }
            default: {
                cout << "Invalid input!" << endl;
            }
        }
    }
}

void generate_ilp(string filepath) {
    Blif* blif = Blif_Reader(filepath);
    MyDesign* des = Elaborate(blif);
	pre_process(des);

    MyScope* scope = des->get_scope();
    vector<CELL>& cells = scope->cells;

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

    /////////////////////////////////////////////////////////////
    
	unordered_set<string> binary_vars;
    max_cycle += 1;
	// ��ʼ���� ILP
    // �����ļ�
	ofstream ilp_file("output.lp");
	if (!ilp_file.is_open()) {
		cout << "Failed to open file!" << endl;
		return;
	}
	// д�� ILP �ļ�ͷ
	ilp_file << "Min" << endl;
	string target = scope->output_ports[0];
	target = "X" + target;
    string min_param = "";
    for (int i = 1; i <= max_cycle; i++) {
        min_param += to_string(i) + target + to_string(i);
        if (i != max_cycle) {
            min_param += " + ";
        }
    }
	ilp_file << min_param << endl;

	// д��Լ������
    ilp_file << "Subject To" << endl;
    for (auto cell : scope->cells) {
		string name = cell.cell_name;
        if (cell.operands.size() == 0) continue; // �ýڵ�������ڵ�֮һ

        string output = to_string(cell.scheduled_time + 1) + "X" + name + to_string(cell.scheduled_time + 1) + "-";
		binary_vars.insert("X" + name + to_string(cell.scheduled_time + 1)); // ��¼����

        for (auto it : cell.operands) {
            string port_name = it.port_name;
            for (auto c : scope->cells) {
                if (c.cell_name == port_name) {
                    output += to_string(cell.scheduled_time + 1) + "X" + port_name + to_string(cell.scheduled_time + 1) + "-";
					binary_vars.insert("X" + port_name + to_string(cell.scheduled_time + 1)); // ��¼����
                }
            }
        }
		output.erase(output.size() - 1);
		ilp_file << output << " >= 1" << endl;
    }

    // д�����
	ilp_file << "Binary" << endl;
	for (string s : binary_vars) {
		ilp_file << s << endl;
	}

	ilp_file << "End" << endl;
	ilp_file.close();
}

// ����asap������ÿ���ڵ������ִ��ʱ��
void pre_process(MyDesign* design) {
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
                max_time = max(max_time, schedule_time[operand.port_name] + 1);
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
