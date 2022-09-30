#ifndef __WK_UTILS_H__
#define __WK_UTILS_H__





#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "wk_svp_type.h"



void NORMALIZATION_TO_S24Q8(WK_FLOAT_RECT_S *pstInputBbox, IVE_RECT_S24Q8_S *pstSlectBbox, HI_S32 img_W, HI_S32 img_H);

void S24Q8_TO_NORMALIZATION(WK_FLOAT_RECT_S *pstResBbox, IVE_RECT_S24Q8_S *pstTrackBbox, HI_S32 img_W, HI_S32 img_H);




void RECT_TO_NORMALIZATION(WK_RECT_ARRAY_S *pastRect_f, WK_RECT_ARRAY_S *pastRect_u, HI_S32 img_W, HI_S32 img_H);

void RECT_TO_DENRMALIZATION(WK_RECT_ARRAY_S *pastRect_f, WK_RECT_ARRAY_S *pastRect_u, HI_S32 img_W, HI_S32 img_H);

void RECT_TO_POINT(WK_RECT_ARRAY_S *pastRect_u, WK_POINT_ARRAY_S *pastPoint_u);



void Filter_Object(WK_YOLO_RECT_ARRAY_S *raw_pastRect_f, WK_RECT_ARRAY_S *out_pastRect_f, WK_YOLO_CLASS eClass);


HI_FLOAT WK_CalcIOU(WK_FLOAT_RECT_S *pstRect1, WK_FLOAT_RECT_S *pstRect2);

//HI_VOID Rect_To_Point_Array(IVE_RECT_S24Q8_S astBbox[], HI_U32 u32BboxObjNum, WK_POINT_ARRAY_S *pstRect);

/*fill Rect to frame*/
HI_S32 WK_Fill_ONE_Rect(VIDEO_FRAME_INFO_S *pstFrmInfo, WK_POINT_ARRAY_S* pstbbox, HI_U32 u32Color);

/*get output frame Bdbox(get unnomalization points)*/
//void get_frame_bdbox(WK_RECT_ARRAY_S *pstRect, HI_U32 s32width, HI_U32 s32heigth ,HI_BOOL bfilter);

void YOLOV3_RECT_TO_DENRMALIZATION(WK_YOLO_RECT_ARRAY_S *pastRect_f, WK_YOLO_RECT_ARRAY_S *pastRect_u, HI_S32 img_W, HI_S32 img_H);

HI_S32 SAMPLE_COMM_SVP_NNIE_YOLOV3_FillRect(VIDEO_FRAME_INFO_S *pstFrmInfo, WK_YOLO_RECT_ARRAY_S *pstRect, HI_U32 u32Color);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __WK_UTILS_H__ */