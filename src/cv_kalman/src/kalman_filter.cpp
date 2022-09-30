// Module "core"
// #include <opencv2/core/core.hpp>

// Module "highgui"
// #include <opencv2/highgui/highgui.hpp>

// Module "imgproc"
// #include <opencv2/imgproc/imgproc.hpp>

// Module "video"
#include <opencv2/video/video.hpp>

// Vector
// #include <vector>

// Output
#include <iostream>

#include "wk_utils.h"

#include "kalman_filter.h"

using namespace std;


#ifdef __cplusplus
extern "C" {
#endif


cv::Mat state;  
cv::Mat meas;

typedef struct kalman_filter
{
    cv::KalmanFilter *obj;
    // cv::Mat *Mat;
}Kalman_Filter;



// stateSize = 6;  //state的维度
// measSize = 4;   //测量维度
// contrSize = 0;  //控制向量
//type = CV_32F  //创建的matrices
KalmanFilter KalmanFilter_OBJ_Create(int stateSize, int measSize, int contrSize, unsigned int type)
{
    cv::Mat state_M(stateSize, 1, type);  // [x,y,v_x,v_y,w,h],定义了状态向量；x, y:球的质心、v_x, v_y：质心的速度(像素/秒)、w, h : 边界框的大小
    cv::Mat meas_M(measSize, 1, type);    // [z_x,z_y,z_w,z_h],定义测量向量；z_x, z_y : 已识别球的质心,z_w, z_h : 识别球的大小
    state = state_M;
    meas = meas_M;

    KalmanFilter kf = (KalmanFilter)malloc(sizeof(Kalman_Filter));
    kf->obj = new cv::KalmanFilter(stateSize, measSize, contrSize, type);

    // Transition State Matrix A
    // Note: set dT at each processing step!
    // [ 1 0 dT 0  0 0 ]
    // [ 0 1 0  dT 0 0 ]
    // [ 0 0 1  0  0 0 ]
    // [ 0 0 0  1  0 0 ]
    // [ 0 0 0  0  1 0 ]
    // [ 0 0 0  0  0 1 ]
    cv::setIdentity(kf->obj->transitionMatrix);   //设置状态转移矩阵A为单位对角矩阵

    // Measure Matrix H
    // [ 1 0 0 0 0 0 ]
    // [ 0 1 0 0 0 0 ]
    // [ 0 0 0 0 1 0 ]
    // [ 0 0 0 0 0 1 ]
    kf->obj->measurementMatrix = cv::Mat::zeros(measSize, stateSize, type);   //设置测量矩阵 H
    kf->obj->measurementMatrix.at<float>(0) = 1.0f;
    kf->obj->measurementMatrix.at<float>(7) = 1.0f;
    kf->obj->measurementMatrix.at<float>(16) = 1.0f;
    kf->obj->measurementMatrix.at<float>(23) = 1.0f;

    // Process Noise Covariance Matrix Q
    // [ Ex   0   0     0     0    0  ]
    // [ 0    Ey  0     0     0    0  ]
    // [ 0    0   Ev_x  0     0    0  ]
    // [ 0    0   0     Ev_y  0    0  ]
    // [ 0    0   0     0     Ew   0  ]
    // [ 0    0   0     0     0    Eh ]
    //cv::setIdentity(kf.processNoiseCov, cv::Scalar(1e-2));
    kf->obj->processNoiseCov.at<float>(0) = 1e-2;                   //设置过程噪声协方差矩阵 Q
    kf->obj->processNoiseCov.at<float>(7) = 1e-2;
    kf->obj->processNoiseCov.at<float>(14) = 5.0f;
    kf->obj->processNoiseCov.at<float>(21) = 5.0f;
    kf->obj->processNoiseCov.at<float>(28) = 1e-2;
    kf->obj->processNoiseCov.at<float>(35) = 1e-2;

    // Measures Noise Covariance Matrix R
    cv::setIdentity(kf->obj->measurementNoiseCov, cv::Scalar(1e-1));   //设置测量噪声协方差矩阵 R

    return kf;
}


// void Init_KalmanFilter(KalmanFilter kf)
// {

// }

void KalmanFilter_Predict(KalmanFilter kf, double dT, WK_FLOAT_RECT_S *pstRect)
{
    // >>>> Matrix A
    kf->obj->transitionMatrix.at<float>(2) = dT;
    kf->obj->transitionMatrix.at<float>(9) = dT;
    // <<<< Matrix A

    cout << "dT:" << endl << dT << endl;

    state =  kf->obj->predict();                           //计算预测的状态值
    cout << "State post:" << endl << state << endl;

    pstRect->fWidth = state.at<float>(4);
    pstRect->fHeight = state.at<float>(5);
    pstRect->fX = state.at<float>(0) - pstRect->fWidth / 2;
    pstRect->fY = state.at<float>(1) - pstRect->fHeight / 2;

    cv::Point center;
    center.x = state.at<float>(0);
    center.y = state.at<float>(1);
}


void KalmanFilter_Update(KalmanFilter kf, WK_FLOAT_RECT_S *pstRect, bool *bFound)
{
    meas.at<float>(0) = pstRect->fX + pstRect->fWidth / 2;
    meas.at<float>(1) = pstRect->fY + pstRect->fHeight / 2;
    meas.at<float>(2) = (float)pstRect->fWidth;
    meas.at<float>(3) = (float)pstRect->fHeight;

    if (!(*bFound)) // First detection!
    {
        // >>>> Initialization
        kf->obj->errorCovPre.at<float>(0) = 1; // px
        kf->obj->errorCovPre.at<float>(7) = 1; // px
        kf->obj->errorCovPre.at<float>(14) = 1;
        kf->obj->errorCovPre.at<float>(21) = 1;
        kf->obj->errorCovPre.at<float>(28) = 1; // px
        kf->obj->errorCovPre.at<float>(35) = 1; // px

        state.at<float>(0) = meas.at<float>(0);
        state.at<float>(1) = meas.at<float>(1);
        state.at<float>(2) = 0;
        state.at<float>(3) = 0;
        state.at<float>(4) = meas.at<float>(2);
        state.at<float>(5) = meas.at<float>(3);
        // <<<< Initialization

        kf->obj->statePost = state;
        
        *bFound = true;
    }
    else
        kf->obj->correct(meas); // Kalman Correction 根据测量值更新状态值

    cout << "Measure matrix:" << endl << meas << endl;
}


void KalmanFilter_OBJ_Destroy(KalmanFilter kf)
{
    delete (kf->obj); // 释放
    free(kf);
}






int Kalman_Filter_ObjIOU(WK_RECT_ARRAY_S *Det_astRect, WK_FLOAT_RECT_S *Kf_pstRect, WK_FLOAT_RECT_S *pstRect)
{
    printf("=============== IOU ==============\n");
    int Ret;
    int i;
    float tmpIou = 0;
    int matchIndex = -1;
    for(i = 0; i < Det_astRect->u16Num; i++)
    {
        bool bSuppressed;
        float f32Iou;
        f32Iou = WK_CalcIOU(&Det_astRect->astRect[i].stRect_f,Kf_pstRect);
        if(f32Iou > tmpIou)
        {
            tmpIou = f32Iou;
            matchIndex = i;
        }
    }
    if(matchIndex)
    {
        pstRect->fX = Det_astRect->astRect[matchIndex].stRect_f.fX;
        pstRect->fY = Det_astRect->astRect[matchIndex].stRect_f.fY;
        pstRect->fWidth = Det_astRect->astRect[matchIndex].stRect_f.fWidth;
        pstRect->fHeight = Det_astRect->astRect[matchIndex].stRect_f.fHeight;
        return 0;
    }
    else
    {
        return -1;
    }

}


#ifdef __cplusplus
}
#endif