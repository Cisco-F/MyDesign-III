
# ifndef _HU_H_
# define _HU_H_

# include <stdio.h>
# include <string>
# include <vector>
# include <queue>
# include <malloc.h>
# include <iostream>

# include "My_Netlist.h"

using namespace std;

typedef struct _Node {
	string node_name;
	vector<_Node*> children;
	int depth;
	char op;
} Node;

void Hu_Scheduling(string filepath);
// generate a tree from MyDesign
void Generate_Tree(MyDesign* des);
// tree generation util
void Generate_Subtree(vector<CELL> cells, Node* root, int depth);
//show scheduling results
void Hu_Ordering();

# endif
