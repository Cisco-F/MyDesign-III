#pragma once

#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
#include <commdlg.h>

using namespace std;

// 定义逻辑门结构体
struct M_Gate {
    string m_output;                   // 输出信号
    vector<string> m_inputs;           // 输入信号列表
    vector<string> truthTable;       // 真值表
    string type;                     // 逻辑门类型：AND, OR, NOT
    int num;                         // 逻辑门数量
    int remainingCycles;             // 剩余周期数
};

// 解析 BLIF 文件，返回逻辑门信息
vector<M_Gate> parseBlif(const string& filename, string& modelName, vector<string>& inputs, vector<string>& outputs);

// 构建前驱节点映射（用于调度判断）
unordered_map<string, vector<string>> buildPredecessorMap(const vector<M_Gate>& gates);

// 构建后继节点映射
unordered_map<string, vector<string>> buildSuccessorMap(const vector<M_Gate>& gates);

// 查找所有就绪状态的节点
vector<M_Gate> findReadyM_Gates(const vector<M_Gate>& gates, const unordered_set<string>& scheduledM_Gates, unordered_map<string, vector<string>>& predecessorMap);

// 确定逻辑门类型
string determineM_GateType(M_Gate& gate);

// 执行多级逻辑门调度
void runMLRCS(vector<M_Gate>& gates, const vector<string>& inputs, const vector<string>& outputs, int maxAND, int maxOR, int maxNOT);

void ML_RCS_Scheduling(string blifFilename);