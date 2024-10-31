# pragma once
# ifndef _BLIF_H_
# define _BLIF_H_

#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

# include "My_Netlist.h"

using namespace std;

static int ADD_GATE_NUM = 0;
static int OR_GATE_NUM = 0;
static int NOT_GATE_NUM = 0;

typedef struct _Blif_Gate {
    vector<string> gate_port;   //门电路端口 包含输入和输出（最后一位为输出）
    vector<string> gate_def;    //门电路的描述 blif源表达式
}BlifGate;

class Blif {
public:
    Blif() {};

    string module_name;             //模块名
    vector<string> input_list;      //输入端口
    vector<string> output_list;     //输出端口
    vector<BlifGate> blifGates;     //门电路
};

Blif* Blif_Reader(string filepath);
MyDesign* Elaborate(Blif* blif);
void Update_Conns(MyDesign* des);
//void Verilog_Converter(Blif* blif);

# endif