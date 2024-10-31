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

# include "My_Netlist.h"


void ALAP_Scheduling(string filepath);
void Schedule_ALAP(MyDesign* design);
void Print_ALAP_Schedule(MyDesign* design);

# endif

