# include "ML_RCS.h"

// 解析 .blif 文件并返回逻辑门的信息
vector<M_Gate> parseBlif(const string& filename, string& modelName, vector<string>& inputs, vector<string>& outputs) {
    ifstream file(filename);  // 打开文件
    if (!file.is_open()) {
        cerr << "Unable to open file " << filename << endl;
        exit(1);
    }

    string line;
    vector<M_Gate> gates;  // 存储逻辑门信息
    M_Gate currentM_Gate;  // 当前解析的逻辑门
    bool inNamesSection = false;  // 标记是否在名称部分

    // 逐行读取文件内容
    while (getline(file, line)) {
        stringstream ss(line);
        string token;
        ss >> token;

        // 解析模型名称
        if (token == ".model") {
            ss >> modelName;
        }
        // 解析输入信号
        else if (token == ".inputs") {
            while (ss >> token) {
                inputs.push_back(token);
            }
        }
        // 解析输出信号
        else if (token == ".outputs") {
            while (ss >> token) {
                outputs.push_back(token);
            }
        }
        // 结束标志
        else if (token == ".end") {
            break;
        }
        // 解析逻辑门名称
        else if (token == ".names") {
            if (inNamesSection) {
                gates.push_back(currentM_Gate);
            }
            inNamesSection = true;
            currentM_Gate = M_Gate();  // 初始化当前逻辑门
            while (ss >> token) {
                currentM_Gate.m_inputs.push_back(token);  // 添加输入信号
            }
            currentM_Gate.m_output = currentM_Gate.m_inputs.back();  // 设置输出信号
            currentM_Gate.m_inputs.pop_back();  // 移除最后一个输入，作为输出
        }
        // 解析真值表
        else if (!token.empty()) {
            currentM_Gate.truthTable.push_back(line);  // 添加真值表行
        }
    }
    // 如果在名称部分，添加最后一个逻辑门
    if (inNamesSection) {
        gates.push_back(currentM_Gate);
    }

    return gates;  // 返回解析出的逻辑门
}

// 构建前驱节点映射（用于调度判断）
unordered_map<string, vector<string>> buildPredecessorMap(const vector<M_Gate>& gates) {
    unordered_map<string, vector<string>> predecessorMap;  // 前驱节点映射
    for (const auto& gate : gates) {
        for (const auto& input : gate.m_inputs) {
            predecessorMap[gate.m_output].push_back(input);  // 将输入添加到对应的输出
        }
    }
    return predecessorMap;  // 返回前驱节点映射
}

// 构建后继节点映射
unordered_map<string, vector<string>> buildSuccessorMap(const vector<M_Gate>& gates) {
    unordered_map<string, vector<string>> successorMap;  // 后继节点映射
    for (const auto& gate : gates) {
        for (const auto& input : gate.m_inputs) {
            successorMap[input].push_back(gate.m_output);  // 将输出添加到对应的输入
        }
    }
    return successorMap;  // 返回后继节点映射
}

// 查找所有就绪状态的节点
vector<M_Gate> findReadyM_Gates(const vector<M_Gate>& gates, const unordered_set<string>& scheduledM_Gates, unordered_map<string, vector<string>>& predecessorMap) {
    vector<M_Gate> readyM_Gates;  // 存储就绪的逻辑门

    for (const auto& gate : gates) {
        // 如果该节点已经调度过，则跳过
        if (scheduledM_Gates.find(gate.m_output) != scheduledM_Gates.end()) {
            continue;
        }

        bool ready = true;  // 标记该门是否就绪
        // 检查所有前驱节点是否都已经被调度
        for (const auto& input : gate.m_inputs) {
            if (scheduledM_Gates.find(input) == scheduledM_Gates.end()) {
                ready = false;  // 若有未调度的前驱节点，则设置为未就绪
                break;
            }
        }

        if (ready) {
            readyM_Gates.push_back(gate);  // 将就绪的逻辑门添加到列表
        }
    }
    return readyM_Gates;  // 返回所有就绪的逻辑门
}

// 确定逻辑门类型
string determineM_GateType(M_Gate& gate) {
    if (gate.truthTable.size() > 1) {
        return "OR";  // 多于一个输出为 OR 门
    }
    for (const auto& row : gate.truthTable) {
        for (size_t i = 0; i < gate.m_inputs.size(); ++i) {
            if (row[i] == '0') {
                return "NOT";  // 若有输入为 0，则为 NOT 门
            }
        }
    }
    return "AND";  // 默认为 AND 门
}

// 执行多级逻辑门调度
void runMLRCS(vector<M_Gate>& gates, const vector<string>& inputs, const vector<string>& outputs, int maxAND, int maxOR, int maxNOT) {
    unordered_set<string> scheduledM_Gates;  // 记录已调度的门
    unordered_map<string, vector<string>> predecessorMap = buildPredecessorMap(gates);  // 前驱节点映射
    unordered_map<string, vector<string>> successorMap = buildSuccessorMap(gates);  // 后继节点映射

    unordered_map<string, int> completionTime;  // 每个信号的完成时间

    // 初始化输入信号的完成时间为周期 0
    for (const auto& input : inputs) {
        scheduledM_Gates.insert(input);
        completionTime[input] = 0;
    }

    int cycle = 1;  // 当前周期

    // 预处理每个门，确定门类型和所需资源数量
    for (auto& gate : gates) {
        gate.type = determineM_GateType(gate);
        gate.num = 1;  // 假设每种门的数量为 1
    }

    // 资源释放队列：每个元素是 {资源释放周期数, 占用资源数}
    queue<pair<int, int>> andResourceQueue;  // AND 门资源释放队列
    queue<pair<int, int>> orResourceQueue;   // OR 门资源释放队列
    queue<pair<int, int>> notResourceQueue;  // NOT 门资源释放队列

    // 资源约束相关变量
    int availableAND = maxAND, availableOR = maxOR, availableNOT = maxNOT;

    // 用于存储每个周期的调度结果
    vector<string> cycleResults;  // 存储每个周期的调度结果

    // 调度循环，直到所有节点都被调度
    while ((scheduledM_Gates.size() - inputs.size()) < gates.size()) {
        vector<M_Gate> readyM_Gates = findReadyM_Gates(gates, scheduledM_Gates, predecessorMap);  // 查找就绪的逻辑门

        vector<string> scheduledAND, scheduledOR, scheduledNOT;

        // 释放已完成调度的资源
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

        // 按顺序调度节点，确保每种资源不超过限制
        for (const auto& gate : readyM_Gates) {
            bool canSchedule = true;  // 标记是否可以调度
            int maxInputCompletionTime = 0;  // 记录所有输入的最大完成时间

            // 检查该门的所有输入是否已经调度完成
            for (const string& input : gate.m_inputs) {
                if (completionTime.find(input) == completionTime.end()) {
                    canSchedule = false;  // 若某输入尚未完成，设置为不可调度
                    break;
                }
                // 记录所有输入的最大完成时间
                maxInputCompletionTime = max(maxInputCompletionTime, completionTime[input]);
            }

            if (!canSchedule) continue;  // 如果依赖的输入尚未完成，跳过调度

            // 计算可以调度的最早周期
            int earliestCycle = max(maxInputCompletionTime + 1, cycle);

            // 根据门类型进行调度
            if (gate.type == "AND") {
                if (gate.num <= availableAND && earliestCycle == cycle) {
                    availableAND -= gate.num;
                    scheduledAND.push_back(gate.m_output);
                    scheduledM_Gates.insert(gate.m_output);
                    andResourceQueue.push({ cycle + 2, gate.num });  // AND 门需要 2 个周期
                    completionTime[gate.m_output] = cycle + 2 - 1;  // 完成时间
                }
            }
            else if (gate.type == "OR") {
                if (gate.num <= availableOR && earliestCycle == cycle) {
                    availableOR -= gate.num;
                    scheduledOR.push_back(gate.m_output);
                    scheduledM_Gates.insert(gate.m_output);
                    orResourceQueue.push({ cycle + 3, gate.num });  // OR 门需要 3 个周期
                    completionTime[gate.m_output] = cycle + 3 - 1;  // 完成时间
                }
            }
            else if (gate.type == "NOT") {
                if (gate.num <= availableNOT && earliestCycle == cycle) {
                    availableNOT -= gate.num;
                    scheduledNOT.push_back(gate.m_output);
                    scheduledM_Gates.insert(gate.m_output);
                    notResourceQueue.push({ cycle + 1, gate.num });  // NOT 门需要 1 个周期
                    completionTime[gate.m_output] = cycle + 1 - 1;  // 完成时间
                }
            }
        }

        // 记录当前周期的调度结果
        if (!scheduledAND.empty() || !scheduledOR.empty() || !scheduledNOT.empty()) {
            stringstream ss;  // 使用 stringstream 来构建周期字符串
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
            cycleResults.push_back(ss.str());  // 将结果添加到周期结果列表
        }

        cycle++;  // 进入下一个周期
    }

    // 计算总的运行周期数
    int totalCycles = 0;
    for (const string& output : outputs) {
        totalCycles = max(totalCycles, completionTime[output]);  // 计算最大完成时间
    }

    cout << "Total " << totalCycles << " Cycles" << endl;  // 输出总周期数

    // 输出各个周期的调度结果
    for (const auto& result : cycleResults) {
        cout << result << endl;  // 输出每个周期的调度结果
    }
}

void ML_RCS_Scheduling(string blifFilename)
{
    string modelName;  // 模型名称
    vector<string> inputs, outputs;  // 输入输出信号
    vector<M_Gate> gates = parseBlif(blifFilename, modelName, inputs, outputs);  // 解析文件

    // 获取用户输入的逻辑门数量限制
    int maxAND, maxOR, maxNOT;

    cout << "Please enter the number of AND gates: ";
    cin >> maxAND;
    cout << "Please enter the number of OR  gates: ";
    cin >> maxOR;
    cout << "Please enter the number of NOT gates: ";
    cin >> maxNOT;
    cout << endl;

    // 输出输入和输出信号
    cout << "Input: ";
    for (const auto& input : inputs) {
        cout << input << " ";  // 输出每个输入信号
    }


    cout << "Output: ";
    for (const auto& output : outputs) {
        cout << output << " ";  // 输出每个输出信号
    }
    cout << endl;

    // 调用调度函数，执行逻辑门调度
    runMLRCS(gates, inputs, outputs, maxAND, maxOR, maxNOT);

    cout << endl << endl << endl;
}