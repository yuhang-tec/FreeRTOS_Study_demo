﻿#ifndef __COMMAND_H
#define __COMMAND_H

#ifdef __cpluscplus
export "C" {
#endif

#include "stdbool.h"

void CPU_Percent(void);
bool RunComFun(void);
void CAN_Test(void);








#ifdef __cpluscplus
}
#endif


#endif
