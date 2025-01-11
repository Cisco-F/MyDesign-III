#pragma once
# ifndef _ALAP_
# define _ALAP_

# include <stdio.h>
# include <string>
# include <vector>
# include <queue>
# include <malloc.h>
# include <iostream>
#include <unordered_map>
# include <unordered_set>

# include "My_Netlist.h"

using namespace std;

typedef struct _Node_a {
	string node_name;
	vector<_Node_a*> children;
	int depth;
	char op;
} a_Node;

void ALAP_Scheduling(string filepath);
void Schedule_ALAP(MyDesign* design);
void Print_ALAP_Schedule(a_Node* root);

// generate a tree from MyDesign
void Generate_Tree_a(MyDesign* des);
// tree generation util
void Generate_Subtree_a(vector<CELL> cells, a_Node* root, int depth);

# endif

