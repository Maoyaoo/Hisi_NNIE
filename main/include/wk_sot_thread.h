#ifndef __WK_SOT_THREAD_H__
#define __WK_SOT_THREAD_H__

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
#include <sys/time.h>

#include "wk_svp_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif    /*__cplusplus*/


/*日志打印控制*/
#define SHOW_LOG 0

/*定义状态*/
enum ProcessState
{
    StateNotProcessed = 0,
    StateInProcess,
    StateCompleted,
    StateSkipped
};

typedef enum ProcessState DetectorState;
typedef enum ProcessState TrackerState;


/*定义跟踪图像信息*/
typedef struct wkFrameInfo_st
{
    struct timeval m_dt;               //时间
    VIDEO_FRAME_INFO_S m_DetFrame;     //检测帧
    VIDEO_FRAME_INFO_S m_TraFrame;     //跟踪帧
    WK_YOLO_RECT_ARRAY_S m_DetRect;    //YOLO检测结果
    WK_FLOAT_RECT_S  m_TraRect;        //kf+KCF跟踪结果
    DetectorState m_inDetector;   //检测状态　0 - not in Detector, 1 - detector started processing, 2 - objects was detected, 3 - detector skip this frame
    TrackerState m_inTracker;     //跟踪状态　0 - not in Tracker, 1 - tracker started processing, 2 - objects was tracked
}SotFrameInfo;



/*初始化跟踪线程*/
int Sot_Thread_Init(void);

/*去初始化跟踪线程*/
void Sot_Thread_DeInit(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*cplusplus*/


#endif