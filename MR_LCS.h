# ifndef _MR_LCS_H_
# define _MR_LCS_H_

# include <iostream>
# include <fstream>
# include <sstream>
# include <vector>
# include <string>
# include <cstring>
# include <algorithm>
#include <map>
#include <unordered_map>

# define _CRT_SECURE_NO_WARNINGS

using namespace std;

void MR_LCS_Scheduling(string filepath);

string determineGateType(int inputCount, const vector<string>& truthTable);

// ����BLIF�ļ��������߼��Ų�����������ϵ
int parseBLIF(const char* filename);
// MR-LCS�����㷨
void MRLCS(int cycleLimit);

# endif
