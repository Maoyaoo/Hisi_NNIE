#ifndef __KCF_CORE_H__
#define __KCF_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <math.h>

#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_svp.h"
#include "sample_comm.h"
#include "sample_comm_svp.h"
#include "sample_comm_nnie.h"
#include "sample_comm_ive.h"
#include "sample_ive_queue.h"


#include "wk_svp_type.h"



typedef struct wkIVE_KCF_PRM_S
{
    // IVE_ROI_INFO_S astTrainRoiInfo[64];

    IVE_MEM_INFO_S stTotalMem; /*Initialize KCF parameters*/
    IVE_MEM_INFO_S stListMem;
    IVE_MEM_INFO_S stCosWinX;
    IVE_MEM_INFO_S stCosWinY;
    IVE_MEM_INFO_S stGaussPeak;
    IVE_KCF_OBJ_LIST_S stObjList;    /*Defines the target link table structure parameters.*/
    IVE_KCF_PRO_CTRL_S stKcfProCtrl; /*Define trace processing control parameters.*/
    IVE_KCF_BBOX_S astBbox[64];      /*Define target area information parameters*/
    HI_U32 u32BboxObjNum;
    IVE_KCF_BBOX_CTRL_S stKcfBboxCtrl; /*Define target area information control parameters*/
    HI_U3Q5 u3q5Padding;               /*The magnification of the target area. Value range：[48, 160]*/
    // HI_U8 u8Reserved;

} IVE_KCF_PRM_S;



HI_S32 kcf_prm_init(HI_VOID);


HI_S32 kcf_prm_deinit(HI_VOID);


HI_S32 kcf_object_init(IVE_RECT_S24Q8_S *pstObjInfo,IVE_IMAGE_S *pstSrc);


HI_S32 kcf_track_update(IVE_IMAGE_S *pstSrc, IVE_RECT_S24Q8_S *pstTrackBox, HI_BOOL *pbTracking_State);




#ifdef __cplusplus
}
#endif


#endif