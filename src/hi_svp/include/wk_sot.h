#ifndef __WK_SOT_H__
#define __WK_SOT_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "wk_svp_type.h"



int wk_sot_init(const char *model_path);


int wk_sot_deinit(void);


int wk_sot_start(WK_SOT_INPUT_PRM *pstSotPrm);





#ifdef __cplusplus
}
#endif


#endif
