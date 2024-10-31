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

// 解析BLIF文件，构建逻辑门操作和依赖关系
int parseBLIF(const char* filename);
// MR-LCS调度算法
void MRLCS(int cycleLimit);

# endif
