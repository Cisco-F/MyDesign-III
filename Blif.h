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
    vector<string> gate_port;   //�ŵ�·�˿� �����������������һλΪ�����
    vector<string> gate_def;    //�ŵ�·������ blifԴ���ʽ
}BlifGate;

class Blif {
public:
    Blif() {};

    string module_name;             //ģ����
    vector<string> input_list;      //����˿�
    vector<string> output_list;     //����˿�
    vector<BlifGate> blifGates;     //�ŵ�·
};

Blif* Blif_Reader(string filepath);
MyDesign* Elaborate(Blif* blif);
void Update_Conns(MyDesign* des);
//void Verilog_Converter(Blif* blif);

# endif