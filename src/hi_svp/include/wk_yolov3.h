#ifndef __WK_YOLOV3_H__
#define __WK_YOLOV3_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "wk_svp_type.h"




/*input model path*/
int wk_yolov3_load_model(const char *model_path);

/*input frame (get nomalization points)*/
int wk_yolov3_run(VIDEO_FRAME_INFO_S *pstExtFrmInfo,WK_YOLO_RECT_ARRAY_S *pstRect);

/*release yolov3*/
int wk_yolov3_clear(void);




#ifdef __cplusplus
}
#endif

#endif