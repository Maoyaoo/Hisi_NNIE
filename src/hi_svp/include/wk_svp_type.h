#ifndef __WK_SVP_TYPE__
#define __WK_SVP_TYPE__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


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

#include "hi_common.h"
#include "hi_debug.h"
#include "hi_comm_video.h"






#define WK_BBOX_NUM 64

/*===========Point For Bbox============*/
typedef struct wk_FLOAT_POINT_S {
    HI_FLOAT fX;
    HI_FLOAT fY;
} WK_FLOAT_POINT_S;

typedef struct wk_UINT_POINT_S {
    HI_S32 uX;
    HI_S32 uY;
} WK_UINT_POINT_S;


typedef union wk_POINT_S
{
    WK_UINT_POINT_S astPoint_u[4];
    WK_FLOAT_POINT_S astPoint_f[4];
}WK_POINT_S;


typedef struct wk_POINT_ARRAY_S
{
    HI_U16 u16Num;
    WK_POINT_S astPoint[WK_BBOX_NUM];
} WK_POINT_ARRAY_S;
/*=====================================*/

/*===========Rect For Bbox============*/
typedef struct wk_FLOAT_RECT_S
{
    HI_FLOAT fX;
    HI_FLOAT fY;
    HI_FLOAT fWidth;
    HI_FLOAT fHeight;
}WK_FLOAT_RECT_S;

typedef struct wk_UINT_RECT_S
{
    HI_S32 uX;
    HI_S32 uY;
    HI_S32 uWidth;
    HI_S32 uHeight;
}WK_UINT_RECT_S;

typedef union wk_RECT_S
{
    WK_UINT_RECT_S stRect_u;
    WK_FLOAT_RECT_S stRect_f;
}WK_RECT_S;
 
typedef struct wk_RECT_ARRAY_S
{
    HI_U16 u16Num;
    WK_RECT_S astRect[WK_BBOX_NUM];
} WK_RECT_ARRAY_S;
/*=====================================*/


/*KCF PRM*/
typedef struct wk_KCF_RES
{
    WK_FLOAT_RECT_S stResBbox;
    HI_BOOL bTrackState;
}WK_KCF_RES;

typedef struct wk_KCF_INPUT_PRM
{
    HI_BOOL bFlashSelectBbox;              /*是否更新框*/
    WK_FLOAT_RECT_S stInputBboxInfo;      /*更新框的坐标 X、Y、W、H*/
   
    VIDEO_FRAME_INFO_S stKcfFrmInfo;      /*kcf img(1920*1080)*/
 
    WK_KCF_RES stKcfRes;            /*Result*/
}WK_KCF_INPUT_PRM;


/*SOT PRM*/
typedef struct wk_SOT_INPUT_PRM
{
    HI_BOOL bFlashSelectBbox;              /*是否更新框*/
    WK_FLOAT_RECT_S stInputBboxInfo;      /*更新框的坐标 X、Y、W、H*/
   
    VIDEO_FRAME_INFO_S stKcfFrmInfo;      /*kcf img(1920*1080)*/
    VIDEO_FRAME_INFO_S stDetectFrmInfo;   /*nnie img(416*416)*/

    WK_FLOAT_RECT_S stResBbox;            /*Result*/
}WK_SOT_INPUT_PRM;


/*YOLOV3 PRM*/
#define WK_YOLOV3_MAX_CLASS_NUM 81
#define WK_YOLOV3_MAX_ROI_NUM_OF_CLASS 50


// typedef struct wk_RECT_S
// {
//     WK_FLOAT_POINT_S ast_fPoint[4]; /* The float point */  /*TODO: use union struct*/
//     WK_UINT_POINT_S ast_uPoint[4];  /* The uint point */
//     HI_FLOAT f32Score;
// } WK_RECT_S;





typedef struct wk_YOLO_RECT_S
{
    WK_RECT_S stRect;
    HI_FLOAT f32Score;
} WK_YOLO_RECT_S;


/*yolov3 Array rect info(normalization)*/
typedef struct wk_YOLO_RECT_ARRAY_S
{
    HI_U32 u32ClsNum;                                                           /*model class num*/
    HI_U32 u32TotalNum;                                                         /*all roi num*/
    HI_U32 au32RoiNum[WK_YOLOV3_MAX_CLASS_NUM];                                 /*each class roi num*/
    WK_YOLO_RECT_S astRect[WK_YOLOV3_MAX_CLASS_NUM][WK_YOLOV3_MAX_ROI_NUM_OF_CLASS];
} WK_YOLO_RECT_ARRAY_S;









#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __WK_SVP_TYPE_H__ */
