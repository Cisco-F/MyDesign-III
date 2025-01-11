# ifndef _SCHEDULING_H_
# define _SCHEDULING_H_

# include <iostream>
#include <windows.h>
#include <commdlg.h>
#include "My_Netlist.h"

using namespace std;

//������
void menu();
//ѡ���ļ�����
string select_file();
//���ȴ���
void Schedule_menu(string filepath);

void generate_ilp(string filepath);
void pre_process(MyDesign* design);


# endif
