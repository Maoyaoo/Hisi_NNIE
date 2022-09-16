#ifndef __WK_KCF_H__
#define __WK_KCF_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "wk_svp_type.h"





HI_S32 wk_kcf_prm_init(void);


HI_S32 wk_kcf_prm_deinit(void);


HI_S32 wk_kcf_proc(WK_KCF_INPUT_PRM *pstkcf_prm);




#ifdef __cplusplus
}
#endif


#endif