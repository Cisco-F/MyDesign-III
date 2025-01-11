# include "ML_RCS.h"

// ���� .blif �ļ��������߼��ŵ���Ϣ
vector<M_Gate> parseBlif(const string& filename, string& modelName, vector<string>& inputs, vector<string>& outputs) {
    ifstream file(filename);  // ���ļ�
    if (!file.is_open()) {
        cerr << "Unable to open file " << filename << endl;
        exit(1);
    }

    string line;
    vector<M_Gate> gates;  // �洢�߼�����Ϣ
    M_Gate currentM_Gate;  // ��ǰ�������߼���
    bool inNamesSection = false;  // ����Ƿ������Ʋ���

    // ���ж�ȡ�ļ�����
    while (getline(file, line)) {
        stringstream ss(line);
        string token;
        ss >> token;

        // ����ģ������
        if (token == ".model") {
            ss >> modelName;
        }
        // ���������ź�
        else if (token == ".inputs") {
            while (ss >> token) {
                inputs.push_back(token);
            }
        }
        // ��������ź�
        else if (token == ".outputs") {
            while (ss >> token) {
                outputs.push_back(token);
            }
        }
        // ������־
        else if (token == ".end") {
            break;
        }
        // �����߼�������
        else if (token == ".names") {
            if (inNamesSection) {
                gates.push_back(currentM_Gate);
            }
            inNamesSection = true;
            currentM_Gate = M_Gate();  // ��ʼ����ǰ�߼���
            while (ss >> token) {
                currentM_Gate.m_inputs.push_back(token);  // ��������ź�
            }
            currentM_Gate.m_output = currentM_Gate.m_inputs.back();  // ��������ź�
            currentM_Gate.m_inputs.pop_back();  // �Ƴ����һ�����룬��Ϊ���
        }
        // ������ֵ��
        else if (!token.empty()) {
            currentM_Gate.truthTable.push_back(line);  // �����ֵ����
        }
    }
    // ��������Ʋ��֣�������һ���߼���
    if (inNamesSection) {
        gates.push_back(currentM_Gate);
    }

    return gates;  // ���ؽ��������߼���
}

// ����ǰ���ڵ�ӳ�䣨���ڵ����жϣ�
unordered_map<string, vector<string>> buildPredecessorMap(const vector<M_Gate>& gates) {
    unordered_map<string, vector<string>> predecessorMap;  // ǰ���ڵ�ӳ��
    for (const auto& gate : gates) {
        for (const auto& input : gate.m_inputs) {
            predecessorMap[gate.m_output].push_back(input);  // ��������ӵ���Ӧ�����
        }
    }
    return predecessorMap;  // ����ǰ���ڵ�ӳ��
}

// ������̽ڵ�ӳ��
unordered_map<string, vector<string>> buildSuccessorMap(const vector<M_Gate>& gates) {
    unordered_map<string, vector<string>> successorMap;  // ��̽ڵ�ӳ��
    for (const auto& gate : gates) {
        for (const auto& input : gate.m_inputs) {
            successorMap[input].push_back(gate.m_output);  // �������ӵ���Ӧ������
        }
    }
    return successorMap;  // ���غ�̽ڵ�ӳ��
}

// �������о���״̬�Ľڵ�
vector<M_Gate> findReadyM_Gates(const vector<M_Gate>& gates, const unordered_set<string>& scheduledM_Gates, unordered_map<string, vector<string>>& predecessorMap) {
    vector<M_Gate> readyM_Gates;  // �洢�������߼���

    for (const auto& gate : gates) {
        // ����ýڵ��Ѿ����ȹ���������
        if (scheduledM_Gates.find(gate.m_output) != scheduledM_Gates.end()) {
            continue;
        }

        bool ready = true;  // ��Ǹ����Ƿ����
        // �������ǰ���ڵ��Ƿ��Ѿ�������
        for (const auto& input : gate.m_inputs) {
            if (scheduledM_Gates.find(input) == scheduledM_Gates.end()) {
                ready = false;  // ����δ���ȵ�ǰ���ڵ㣬������Ϊδ����
                break;
            }
        }

        if (ready) {
            readyM_Gates.push_back(gate);  // ���������߼�����ӵ��б�
        }
    }
    return readyM_Gates;  // �������о������߼���
}

// ȷ���߼�������
string determineM_GateType(M_Gate& gate) {
    if (gate.truthTable.size() > 1) {
        return "OR";  // ����һ�����Ϊ OR ��
    }
    for (const auto& row : gate.truthTable) {
        for (size_t i = 0; i < gate.m_inputs.size(); ++i) {
            if (row[i] == '0') {
                return "NOT";  // ��������Ϊ 0����Ϊ NOT ��
            }
        }
    }
    return "AND";  // Ĭ��Ϊ AND ��
}

// ִ�ж༶�߼��ŵ���
void runMLRCS(vector<M_Gate>& gates, const vector<string>& inputs, const vector<string>& outputs, int maxAND, int maxOR, int maxNOT) {
    unordered_set<string> scheduledM_Gates;  // ��¼�ѵ��ȵ���
    unordered_map<string, vector<string>> predecessorMap = buildPredecessorMap(gates);  // ǰ���ڵ�ӳ��
    unordered_map<string, vector<string>> successorMap = buildSuccessorMap(gates);  // ��̽ڵ�ӳ��

    unordered_map<string, int> completionTime;  // ÿ���źŵ����ʱ��

    // ��ʼ�������źŵ����ʱ��Ϊ���� 0
    for (const auto& input : inputs) {
        scheduledM_Gates.insert(input);
        completionTime[input] = 0;
    }

    int cycle = 1;  // ��ǰ����

    // Ԥ����ÿ���ţ�ȷ�������ͺ�������Դ����
    for (auto& gate : gates) {
        gate.type = determineM_GateType(gate);
        gate.num = 1;  // ����ÿ���ŵ�����Ϊ 1
    }

    // ��Դ�ͷŶ��У�ÿ��Ԫ���� {��Դ�ͷ�������, ռ����Դ��}
    queue<pair<int, int>> andResourceQueue;  // AND ����Դ�ͷŶ���
    queue<pair<int, int>> orResourceQueue;   // OR ����Դ�ͷŶ���
    queue<pair<int, int>> notResourceQueue;  // NOT ����Դ�ͷŶ���

    // ��ԴԼ����ر���
    int availableAND = maxAND, availableOR = maxOR, availableNOT = maxNOT;

    // ���ڴ洢ÿ�����ڵĵ��Ƚ��
    vector<string> cycleResults;  // �洢ÿ�����ڵĵ��Ƚ��

    // ����ѭ����ֱ�����нڵ㶼������
    while ((scheduledM_Gates.size() - inputs.size()) < gates.size()) {
        vector<M_Gate> readyM_Gates = findReadyM_Gates(gates, scheduledM_Gates, predecessorMap);  // ���Ҿ������߼���

        vector<string> scheduledAND, scheduledOR, scheduledNOT;

        // �ͷ�����ɵ��ȵ���Դ
        while (!andResourceQueue.empty() && andResourceQueue.front().first == cycle) {
            availableAND += andResourceQueue.front().second;
            andResourceQueue.pop();
        }
        while (!orResourceQueue.empty() && orResourceQueue.front().first == cycle) {
            availableOR += orResourceQueue.front().second;
            orResourceQueue.pop();
        }
        while (!notResourceQueue.empty() && notResourceQueue.front().first == cycle) {
            availableNOT += notResourceQueue.front().second;
            notResourceQueue.pop();
        }

        // ��˳����Ƚڵ㣬ȷ��ÿ����Դ����������
        for (const auto& gate : readyM_Gates) {
            bool canSchedule = true;  // ����Ƿ���Ե���
            int maxInputCompletionTime = 0;  // ��¼���������������ʱ��

            // �����ŵ����������Ƿ��Ѿ��������
            for (const string& input : gate.m_inputs) {
                if (completionTime.find(input) == completionTime.end()) {
                    canSchedule = false;  // ��ĳ������δ��ɣ�����Ϊ���ɵ���
                    break;
                }
                // ��¼���������������ʱ��
                maxInputCompletionTime = max(maxInputCompletionTime, completionTime[input]);
            }

            if (!canSchedule) continue;  // ���������������δ��ɣ���������

            // ������Ե��ȵ���������
            int earliestCycle = max(maxInputCompletionTime + 1, cycle);

            // ���������ͽ��е���
            if (gate.type == "AND") {
                if (gate.num <= availableAND && earliestCycle == cycle) {
                    availableAND -= gate.num;
                    scheduledAND.push_back(gate.m_output);
                    scheduledM_Gates.insert(gate.m_output);
                    andResourceQueue.push({ cycle + 2, gate.num });  // AND ����Ҫ 2 ������
                    completionTime[gate.m_output] = cycle + 2 - 1;  // ���ʱ��
                }
            }
            else if (gate.type == "OR") {
                if (gate.num <= availableOR && earliestCycle == cycle) {
                    availableOR -= gate.num;
                    scheduledOR.push_back(gate.m_output);
                    scheduledM_Gates.insert(gate.m_output);
                    orResourceQueue.push({ cycle + 3, gate.num });  // OR ����Ҫ 3 ������
                    completionTime[gate.m_output] = cycle + 3 - 1;  // ���ʱ��
                }
            }
            else if (gate.type == "NOT") {
                if (gate.num <= availableNOT && earliestCycle == cycle) {
                    availableNOT -= gate.num;
                    scheduledNOT.push_back(gate.m_output);
                    scheduledM_Gates.insert(gate.m_output);
                    notResourceQueue.push({ cycle + 1, gate.num });  // NOT ����Ҫ 1 ������
                    completionTime[gate.m_output] = cycle + 1 - 1;  // ���ʱ��
                }
            }
        }

        // ��¼��ǰ���ڵĵ��Ƚ��
        if (!scheduledAND.empty() || !scheduledOR.empty() || !scheduledNOT.empty()) {
            stringstream ss;  // ʹ�� stringstream �����������ַ���
            ss << "cycle " << cycle << ": { ";
            for (size_t i = 0; i < scheduledAND.size(); i++) {
                if (i > 0) ss << " ";
                ss << scheduledAND[i];
            }
            ss << " } , {";
            for (size_t i = 0; i < scheduledOR.size(); i++) {
                if (i > 0) ss << " ";
                ss << scheduledOR[i];
            }
            ss << " } , {";
            for (size_t i = 0; i < scheduledNOT.size(); i++) {
                if (i > 0) ss << " ";
                ss << scheduledNOT[i];
            }
            ss << "}";
            cycleResults.push_back(ss.str());  // �������ӵ����ڽ���б�
        }

        cycle++;  // ������һ������
    }

    // �����ܵ�����������
    int totalCycles = 0;
    for (const string& output : outputs) {
        totalCycles = max(totalCycles, completionTime[output]);  // ����������ʱ��
    }

    cout << "Total " << totalCycles << " Cycles" << endl;  // �����������

    // ����������ڵĵ��Ƚ��
    for (const auto& result : cycleResults) {
        cout << result << endl;  // ���ÿ�����ڵĵ��Ƚ��
    }
}

void ML_RCS_Scheduling(string blifFilename)
{
    string modelName;  // ģ������
    vector<string> inputs, outputs;  // ��������ź�
    vector<M_Gate> gates = parseBlif(blifFilename, modelName, inputs, outputs);  // �����ļ�

    // ��ȡ�û�������߼�����������
    int maxAND, maxOR, maxNOT;

    cout << "Please enter the number of AND gates: ";
    cin >> maxAND;
    cout << "Please enter the number of OR  gates: ";
    cin >> maxOR;
    cout << "Please enter the number of NOT gates: ";
    cin >> maxNOT;
    cout << endl;

    // ������������ź�
    cout << "Input: ";
    for (const auto& input : inputs) {
        cout << input << " ";  // ���ÿ�������ź�
    }


    cout << "Output: ";
    for (const auto& output : outputs) {
        cout << output << " ";  // ���ÿ������ź�
    }
    cout << endl;

    // ���õ��Ⱥ�����ִ���߼��ŵ���
    runMLRCS(gates, inputs, outputs, maxAND, maxOR, maxNOT);

    cout << endl << endl << endl;
}