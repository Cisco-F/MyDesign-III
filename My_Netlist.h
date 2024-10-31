# pragma once
# ifndef _MY_NETLIST_H_
# define _MY_NETLIST_H_

# include <string>
# include <map>
# include <vector>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <limits.h>

using namespace std;

# define IN 0
# define OUT 1

struct PORT {
	string port_name;
	int inout_type;
	int depth = 0;
};

struct CELL {
	char op;
	int scheduled_time;
	string cell_name;	//³ö¶Ë¿ÚÃû
	vector<PORT> operands;
	map<string, CELL> cell_conns;
};

class MyScope {
public:
	MyScope() {};
	~MyScope() {};

	string scope_name;
	vector<CELL> cells;
	vector<string> input_ports;
	vector<string> output_ports;

};

class MyDesign {
public:
	MyDesign() {};
	~MyDesign() {};

	void gen_netlist();

	MyScope* get_scope() { return &(this->scope); };
private:
	MyScope scope;
};

# endif
