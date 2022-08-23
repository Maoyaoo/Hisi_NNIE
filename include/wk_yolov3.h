#ifndef __WK_YOLOV3_H__
#define __WK_YOLOV3_H__


#include "yolov3_core.h"




#ifdef __cplusplus
extern "C" {
#endif

/*input model path*/
int yolov3_load_model(const char *model_path);

/*input frame (get nomalization points)*/
int yolov3_run(VIDEO_FRAME_INFO_S *pstExtFrmInfo,WK_RECT_ARRAY_S *pstRect);

/*release yolov3*/
int yolov3_clear(void);


/*get output frame Bdbox(get unnomalization points)*/
void get_frame_bdbox(WK_RECT_ARRAY_S *pstRect, HI_U32 s32width, HI_U32 s32heigth ,HI_BOOL bfilter);


HI_S32 SAMPLE_COMM_SVP_NNIE_YOLOV3_FillRect(VIDEO_FRAME_INFO_S *pstFrmInfo, WK_RECT_ARRAY_S* pstRect, HI_U32 u32Color);

#ifdef __cplusplus
}
#endif

#endif