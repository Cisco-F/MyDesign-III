#pragma once
# ifndef _ASAP_
# define _ASAP_

# include <stdio.h>
# include <string>
# include <vector>
# include <queue>
# include <malloc.h>
# include <iostream>
#include <unordered_map>
#include <set>

# include "My_Netlist.h"

struct ScheduleInfo {
    int start_time;
    int finish_time;
    int earliest_start;
    int latest_start;
};

void ASAP_Scheduling(string filepath);
void Schedule_ASAP(MyDesign* des);
void Print_ASAP_Schedule(MyDesign* des);

# endif
