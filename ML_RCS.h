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

// �����߼��Žṹ��
struct M_Gate {
    string m_output;                   // ����ź�
    vector<string> m_inputs;           // �����ź��б�
    vector<string> truthTable;       // ��ֵ��
    string type;                     // �߼������ͣ�AND, OR, NOT
    int num;                         // �߼�������
    int remainingCycles;             // ʣ��������
};

// ���� BLIF �ļ��������߼�����Ϣ
vector<M_Gate> parseBlif(const string& filename, string& modelName, vector<string>& inputs, vector<string>& outputs);

// ����ǰ���ڵ�ӳ�䣨���ڵ����жϣ�
unordered_map<string, vector<string>> buildPredecessorMap(const vector<M_Gate>& gates);

// ������̽ڵ�ӳ��
unordered_map<string, vector<string>> buildSuccessorMap(const vector<M_Gate>& gates);

// �������о���״̬�Ľڵ�
vector<M_Gate> findReadyM_Gates(const vector<M_Gate>& gates, const unordered_set<string>& scheduledM_Gates, unordered_map<string, vector<string>>& predecessorMap);

// ȷ���߼�������
string determineM_GateType(M_Gate& gate);

// ִ�ж༶�߼��ŵ���
void runMLRCS(vector<M_Gate>& gates, const vector<string>& inputs, const vector<string>& outputs, int maxAND, int maxOR, int maxNOT);

void ML_RCS_Scheduling(string blifFilename);