#ifndef _KALMAN_FILTER_H_
#define _KALMAN_FILTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "wk_svp_type.h"


typedef struct kalman_filter Kalman_Filter;

typedef Kalman_Filter* KalmanFilter;



KalmanFilter KalmanFilter_OBJ_Create(int stateSize, int measSize, int contrSize, unsigned int type);

// void Init_KalmanFilter(KalmanFilter kf);

void KalmanFilter_Predict(KalmanFilter kf, double dT, WK_FLOAT_RECT_S *pstRect);

void KalmanFilter_Update(KalmanFilter kf, WK_FLOAT_RECT_S *pstRect, bool *bFound);

void KalmanFilter_OBJ_Destroy(KalmanFilter kf);

int Kalman_Filter_ObjIOU(WK_RECT_ARRAY_S *Det_astRect, WK_FLOAT_RECT_S *Kf_pstRect, WK_FLOAT_RECT_S *pstRect);



#ifdef __cplusplus
};
#endif

#endif
