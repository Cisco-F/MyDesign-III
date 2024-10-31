# include "MR_LCS.h"

const int MAX_GATES = 100;
const int MAX_INPUTS = 10;
const int MAX_CYCLE = 100;

struct GateOperation {
    string name;
    string type;
    vector<string> inputs;
    int inputCount;
    int scheduledCycle;
    int earliestStart;
    int latestStart;
    int mobility;
    vector<int> predecessors;
};

vector<GateOperation> operations;
vector<string> inputSignals;
vector<string> outputSignals;
int numInputs = 0;
int numOutputs = 0;

int parseBLIF(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "错误：无法打开文件 " << filename << endl;
        return -1;
    }

    string line;
    string pendingLine;
    int gateIndex = 0;

    // 添加名称到索引的映射
    unordered_map<string, int> nameToIndex;

    while (true) {
        if (!pendingLine.empty()) {
            line = pendingLine;
            pendingLine.clear();
        }
        else {
            if (!getline(file, line)) {
                break;
            }
            line.erase(line.find_last_not_of("\r\n") + 1);
        }

        if (line.empty()) {
            continue;
        }

        if (line.compare(0, 7, ".inputs") == 0) {
            istringstream iss(line.substr(7));
            string token;
            while (iss >> token) {
                inputSignals.push_back(token);
                numInputs++;
            }
        }
        else if (line.compare(0, 8, ".outputs") == 0) {
            istringstream iss(line.substr(8));
            string token;
            while (iss >> token) {
                outputSignals.push_back(token);
                numOutputs++;
            }
        }
        else if (line.compare(0, 6, ".names") == 0) {
            istringstream iss(line.substr(6));
            vector<string> tokens;
            string token;
            while (iss >> token) {
                tokens.push_back(token);
            }

            GateOperation gate;
            gate.inputCount = static_cast<int>(tokens.size()) - 1;
            gate.scheduledCycle = -1;
            gate.earliestStart = 1;

            for (int i = 0; i < gate.inputCount; i++) {
                gate.inputs.push_back(tokens[i]);
            }
            gate.name = tokens.back();

            // 读取真值表
            vector<string> truthTable;
            while (true) {
                if (!getline(file, line)) {
                    break;
                }
                line.erase(line.find_last_not_of("\r\n") + 1);
                if (line.empty()) {
                    continue;
                }
                if (line[0] == '.') {
                    pendingLine = line;
                    break;
                }
                truthTable.push_back(line);
            }

            // 根据真值表确定门的类型
            gate.type = determineGateType(gate.inputCount, truthTable);

            // 将门添加到operations中
            operations.push_back(gate);
            // 将名称和索引添加到映射中
            nameToIndex[gate.name] = gateIndex;

            gateIndex++;
        }
        else if (line.compare(0, 4, ".end") == 0) {
            break;
        }
    }

    file.close();

    // 构建前驱关系
    for (int i = 0; i < gateIndex; i++) {
        GateOperation& gate = operations[i];
        for (const auto& inputSignal : gate.inputs) {
            bool isInputSignal = false;
            for (const auto& input : inputSignals) {
                if (inputSignal == input) {
                    isInputSignal = true;
                    break;
                }
            }
            if (!isInputSignal) {
                if (nameToIndex.find(inputSignal) != nameToIndex.end()) {
                    int predIndex = nameToIndex[inputSignal];
                    gate.predecessors.push_back(predIndex);
                    if (operations[predIndex].earliestStart + 1 > gate.earliestStart) {
                        gate.earliestStart = operations[predIndex].earliestStart + 1;
                    }
                }
                else {
                    cerr << "警告：未找到输入信号 " << inputSignal << " 对应的逻辑门。" << endl;
                }
            }
        }
    }

    return gateIndex;
}





string determineGateType(int inputCount, const vector<string>& truthTable) {
    // 单输入，可能是NOT门
    if (inputCount == 1) {
        if (truthTable.size() == 1) {
            string pattern = truthTable[0];
            if (pattern[0] == '0') {
                return "NOT";
            }
        }
    }
    // 检查是否为AND门
    if (truthTable.size() == 1) {
        string pattern = truthTable[0];
        bool isAndGate = true;
        for (int i = 0; i < inputCount; i++) {
            if (pattern[i] != '1') {
                isAndGate = false;
                break;
            }
        }
        if (isAndGate) {
            return "AND";
        }
    }
    // 检查是否为OR门
    bool isOrGate = true;
    if (truthTable.size() == inputCount) {
        for (const string& pattern : truthTable) {
            int oneCount = 0;
            for (int i = 0; i < inputCount; i++) {
                if (pattern[i] == '1') {
                    oneCount++;
                }
                else if (pattern[i] != '-') {
                    isOrGate = false;
                    break;
                }
            }
            if (oneCount != 1) {
                isOrGate = false;
                break;
            }
            if (!isOrGate) break;
        }
        if (isOrGate) {
            return "OR";
        }
    }
    // 默认类型
    return "OTHER";
}


void calculateLatestStart(int cycleLimit) {
    for (auto& operation : operations) {
        operation.latestStart = cycleLimit;
    }

    for (const auto& outputSignal : outputSignals) {
        for (auto& operation : operations) {
            if (outputSignal == operation.name) {
                operation.latestStart = cycleLimit;
                break;
            }
        }
    }

    bool updated;
    do {
        updated = false;
        for (auto& gate : operations) {
            for (auto& otherGate : operations) {
                for (int pred : otherGate.predecessors) {
                    if (pred == &gate - &operations[0]) {
                        if (otherGate.latestStart - 1 < gate.latestStart) {
                            gate.latestStart = otherGate.latestStart - 1;
                            updated = true;
                        }
                    }
                }
            }
        }
    } while (updated);
}

void calculateMobility() {
    for (auto& operation : operations) {
        operation.mobility = operation.latestStart - operation.earliestStart;
    }
}

void MRLCS(int cycleLimit) {
    int scheduledCount = 0;
    int cycle = 1;

    vector<int> andUsage(MAX_CYCLE, 0);
    vector<int> orUsage(MAX_CYCLE, 0);
    vector<int> notUsage(MAX_CYCLE, 0);

    calculateLatestStart(cycleLimit);
    calculateMobility();

    vector<bool> isScheduled(MAX_GATES, false);

    // 用于记录调度结果，键为周期，值为门名称列表
    map<int, vector<string>> schedulePerCycle;

    while (scheduledCount < operations.size() && cycle <= cycleLimit) {
        vector<int> priorityList;

        for (int i = 0; i < operations.size(); i++) {
            if (!isScheduled[i] && operations[i].earliestStart <= cycle && operations[i].latestStart >= cycle) {
                bool canSchedule = true;
                for (int j = 0; j < operations[i].predecessors.size(); j++) {
                    if (!isScheduled[operations[i].predecessors[j]]) {
                        canSchedule = false;
                        break;
                    }
                }
                if (canSchedule) {
                    priorityList.push_back(i);
                }
            }
        }

        sort(priorityList.begin(), priorityList.end(), [](int a, int b) {
            return operations[a].mobility < operations[b].mobility;
            });

        for (int opIndex : priorityList) {
            operations[opIndex].scheduledCycle = cycle;
            isScheduled[opIndex] = true;
            scheduledCount++;

            // 记录调度信息
            schedulePerCycle[cycle].push_back(operations[opIndex].name);

            if (operations[opIndex].type == "AND") {
                andUsage[cycle]++;
            }
            else if (operations[opIndex].type == "OR") {
                orUsage[cycle]++;
            }
            else if (operations[opIndex].type == "NOT") {
                notUsage[cycle]++;
            }
        }

        cycle++;
    }

    int maxAnd = 0, maxOr = 0, maxNot = 0;
    int totalAnd = 0, totalOr = 0, totalNot = 0;

    for (int i = 1; i <= cycleLimit; i++) {
        maxAnd = max(maxAnd, andUsage[i]);
        maxOr = max(maxOr, orUsage[i]);
        maxNot = max(maxNot, notUsage[i]);

        totalAnd += andUsage[i];
        totalOr += orUsage[i];
        totalNot += notUsage[i];
    }

    cout << "With the limit of " << cycleLimit << " ,minimum resource usage is:" << endl;
    cout << "Max num of AND gates: " << maxAnd << ", total consumption: " << totalAnd << endl;
    cout << "Max num of OR  gates: " << maxOr << ", total consumption: " << totalOr << endl;
    cout << "Max num of NOT gates: " << maxNot << ", total consumption: " << totalNot << endl;

    if (scheduledCount < operations.size()) {
        cout << "Unable to schedule all operations within the given limit!" << endl;
    }

    // 输出每个周期的调度结果，按照门的类型分组
    cout << "\nMR_LCS ordering: " << endl;
    for (int c = 1; c <= cycleLimit; c++) {
        if (schedulePerCycle.count(c)) {
            cout << "Cycle " << c << ":";

            // 创建一个类型到门名称列表的映射
            map<string, vector<string>> gatesByType;
            for (const auto& gateName : schedulePerCycle[c]) {
                // 根据名称查找 gate 的类型
                string gateType;
                for (const auto& op : operations) {
                    if (op.name == gateName) {
                        gateType = op.type;
                        break;
                    }
                }
                gatesByType[gateType].push_back(gateName);
            }

            // 按照类型输出
            for (const auto& typePair : gatesByType) {
                cout << "{" << typePair.first << ": ";
                for (const auto& name : typePair.second) {
                    cout << name << " ";
                }
                cout << "} ";
            }
            cout << endl;
        }
    }
}



void MR_LCS_Scheduling(string filepath) {
    int numGates = parseBLIF(filepath);

    if (numGates > 0) {
        int cycleLimit;
        cout << "Please enter delay limits(cycles): ";
        cin >> cycleLimit;
        MRLCS(cycleLimit);
    }
}