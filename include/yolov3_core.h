#ifndef __YOLOV3_CORE_H__
#define __YOLOV3_CORE_H__


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
#include <dirent.h>
#include <sys/time.h>


#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_svp.h"
#include "sample_comm.h"
#include "sample_comm_svp.h"
#include "sample_comm_nnie.h"
#include "sample_comm_ive.h"
#include "sample_svp_nnie_software.h"


#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_CLREAR()
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_PRE_DST_FLUSH_TIME()
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_OP_TIME()
#define SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_AFTER_DST_FLUSH_TIME()

#define WK_YOLOV3_MAX_CLASS_NUM 81
#define WK_YOLOV3_MAX_ROI_NUM_OF_CLASS 50


typedef struct wk_FLOAT_POINT_S {
    HI_FLOAT s32X;
    HI_FLOAT s32Y;
} WK_FLOAT_POINT_S;

typedef struct wk_UINT_POINT_S {
    HI_S32 s32X;
    HI_S32 s32Y;
} WK_UINT_POINT_S;


#if 0
    typedef struct wk_RECT_S
{
    wk_FLOAT_POINT_S astPoint[4]; 
    HI_FLOAT f32Score;
} WK_RECT_S;
#endif

typedef struct wk_RECT_S
{
    WK_FLOAT_POINT_S ast_fPoint[4]; /* The float point */
    WK_UINT_POINT_S ast_uPoint[4]; /* The uint point */
    HI_FLOAT f32Score;
} WK_RECT_S;

/*yolov3 Array rect info(normalization)*/
typedef struct wk_RECT_ARRAY_S
{
    HI_U32 u32ClsNum;                                                           /*model class num*/
    HI_U32 u32TotalNum;                                                         /*all roi num*/
    HI_U32 au32RoiNum[WK_YOLOV3_MAX_CLASS_NUM];                                 /*each class roi num*/
    WK_RECT_S astRect[WK_YOLOV3_MAX_CLASS_NUM][WK_YOLOV3_MAX_ROI_NUM_OF_CLASS];
} WK_RECT_ARRAY_S;



/*Load model*/
int _yolov3_load_model(const char *model_path, SAMPLE_SVP_NNIE_MODEL_S *s_stModel);

/*Init parameters*/
int yolov3_param_init(SAMPLE_SVP_NNIE_MODEL_S *pstModel, SAMPLE_SVP_NNIE_CFG_S *pstCfg,
                     SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S *pstSoftwareParam);

/*DeInit parameters*/
int yolov3_param_deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam,SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel);


/*Yolov3 inference*/
int yolov3_inference(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S *pstSoftWareParam, VIDEO_FRAME_INFO_S *pstExtFrmInfo,
        WK_RECT_ARRAY_S *pstRect);


// /*print result*/
// int yolov3_print_result(SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S *pstSoftWareParam);




#ifdef __cplusplus
}
#endif


#endif