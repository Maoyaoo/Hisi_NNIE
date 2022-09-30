#include <sys/time.h>

#include "mpi_vpss.h"
#include "mpi_vo.h"


#include "wk_svp_type.h"
#include "wk_kcf.h"
#include "wk_yolov3.h"
#include "kalman_filter.h"
#include "wk_utils.h"

#include "sot_thread.h"



#define WK_QUEUE_LEN 16   //视频帧队列长度


typedef struct wk_SOT_THREAD_S
{
    pthread_mutex_t DetectMutex;
    pthread_mutex_t GetTraFrmMutex;
    //pthread_mutex_t CaptureMutex;/*跟踪显示图像*/
    pthread_mutex_t GetDetFrmMutex; /*检测图像*/
}WK_SOT_THREAD_S;


typedef enum wk_YOLO_PROC_STATUS_E
{
    YOLO_PROC_START = 0x0,
    YOLO_PROC_END = 0x1,

    YOLO_PROC_BUTT
}YOLO_PROC_STATUS_E;

YOLO_PROC_STATUS_E enYoloProcStat;    

static WK_SOT_THREAD_S s_stSotThread = {0};

static VIDEO_FRAME_INFO_S stBaseFrmInfo;
static VIDEO_FRAME_INFO_S stExtFrmInfo;

WK_FLOAT_RECT_S KF_PreRect = {0};      //Kalman 滤波预测ｂｂｏｘ
WK_FLOAT_RECT_S YOLO_MatchRect = {0};  //YOLO匹配的Ｂｂｏｘ
bool bMatch = false;

bool bStartSOT = false;



#define WK_PRT(fmt...)   \
    do {\
        printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
        printf(fmt);\
    }while(0)

#define WK_CHECK_EXPR_GOTO(expr, label, fmt...)\
do\
{\
    if(expr)\
    {\
        printf(fmt);\
        goto label;\
    }\
}while(0)


#define WK_MUTEX_INIT_LOCK(mutex)	\
do                                          \
{											\
	(void)pthread_mutex_init(&mutex, NULL); \
}while(0)
#define WK_MUTEX_LOCK(mutex)		\
do                                          \
{                                           \
	(void)pthread_mutex_lock(&mutex);       \
}while(0)
#define WK_MUTEX_UNLOCK(mutex)		\
do                                          \
{                                           \
	(void)pthread_mutex_unlock(&mutex);     \
}while(0)
#define WK_MUTEX_DESTROY(mutex)		\
do                                          \
{                                           \
	(void)pthread_mutex_destroy(&mutex);    \
}while(0)


//声明跟踪线程标识符ID 
static pthread_t t_CaptureThread = 0;   //CaptureThread
static pthread_t t_DetectThread = 0;    //DetectThread
static pthread_t t_TrackingThread = 0;  //TrackingThread
// static pthread_t t_ShowThread = 0;      //ShowThread



// 定义一个延时xms毫秒的延时函数
void delay(unsigned int xms)  // xms代表需要延时的毫秒数
{
    unsigned int x,y;
    for(x=xms;x>0;x--)
        for(y=110;y>0;y--);
}


static WK_FLOAT_RECT_S stRectInfo;
static HI_BOOL bKCF_SelectBoxFlag;
static HI_BOOL bKF_SelectBoxFlag;
static HI_BOOL bYOLO_SelectBoxFlag;

void wk_sot_select_box_input(float f_SetX, float f_SetY, float f_SetW, float f_SetH)
{
    stRectInfo.fX = f_SetX;
    stRectInfo.fY = f_SetY;
    stRectInfo.fWidth = f_SetW;
    stRectInfo.fHeight = f_SetH;
    bKCF_SelectBoxFlag = HI_TRUE;
    bKF_SelectBoxFlag = HI_TRUE;
    bYOLO_SelectBoxFlag = HI_TRUE;
}


/******************************************************************************
 * function : get frame into queue
 ******************************************************************************/
static void *WK_CaptureThread(void *pArgs)
{
    HI_S32 s32Ret;

    // VIDEO_FRAME_INFO_S stBaseFrmInfo;
    // VIDEO_FRAME_INFO_S stExtFrmInfo;
    VIDEO_FRAME_INFO_S astVideoFrame[2];
    HI_S32 s32MilliSec = 20000;

    HI_S32 s32VpssGrp = 0;
    HI_S32 as32VpssChn[] = {VPSS_CHN0, VPSS_CHN1};

    VO_LAYER voLayer = 0;
    VO_CHN voChn = 0;

    // int QueueLen;
    // SAMPLE_IVE_NODE_S *pstQueueNode;

    while (bStartSOT)
    {
        /****************vpss 0 1920*1080 to Track and Display************/
        s32Ret = HI_MPI_VPSS_GetChnFrame(s32VpssGrp, as32VpssChn[0], &astVideoFrame[0], s32MilliSec);
        WK_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
            "Error(%#x),HI_MPI_VPSS_GetChnFrame failed, VPSS_GRP(%d), VPSS_CHN(%d)!\n",
            s32Ret,s32VpssGrp, as32VpssChn[0]);

        /*获取1080P图像放入跟踪图像队列*/
        WK_MUTEX_LOCK(s_stSotThread.GetTraFrmMutex);
        memcpy(&stBaseFrmInfo, &astVideoFrame[0], sizeof(VIDEO_FRAME_INFO_S));
        WK_MUTEX_UNLOCK(s_stSotThread.GetTraFrmMutex);

        /***************************************************************/

       
        /**************vpss 1 416*416 to YOLOv3**************************/
        s32Ret = HI_MPI_VPSS_GetChnFrame(s32VpssGrp, as32VpssChn[1], &astVideoFrame[1], s32MilliSec);
        if(HI_SUCCESS != s32Ret)
        {
            WK_PRT("Error(%#x),HI_MPI_VPSS_GetChnFrame failed, VPSS_GRP(%d), VPSS_CHN(%d)!\n",
                s32Ret,s32VpssGrp, as32VpssChn[1]);
            continue;
        }
        /*获取416*416图像放入识别*/
        WK_MUTEX_LOCK(s_stSotThread.GetDetFrmMutex);
        memcpy(&stExtFrmInfo, &astVideoFrame[1], sizeof(VIDEO_FRAME_INFO_S));
        enYoloProcStat = YOLO_PROC_START;
        WK_MUTEX_UNLOCK(s_stSotThread.GetDetFrmMutex);
        /***************************************************************/







        /************Plot AND Display*************/
        /*plot*/


        /*display*/
        s32Ret = HI_MPI_VO_SendFrame(0,0,&astVideoFrame[0],20000);
        if (HI_SUCCESS != s32Ret)
        {
            WK_PRT("Error(%#x),HI_MPI_VO_SendFrame failed!\n", s32Ret);
            return s32Ret;
        } 
        /****************************************/
        

        BASE_RELEASE:
            s32Ret = HI_MPI_VPSS_ReleaseChnFrame(s32VpssGrp, as32VpssChn[0], &astVideoFrame[0]);
            if (HI_SUCCESS != s32Ret)
            {
                WK_PRT("Error(%#x),HI_MPI_VPSS_ReleaseChnFrame failed,Grp(%d) chn(%d)!\n",
                    s32Ret,s32VpssGrp,as32VpssChn[0]);
            }

        EXT_RELEASE:
            s32Ret = HI_MPI_VPSS_ReleaseChnFrame(s32VpssGrp, as32VpssChn[1], &astVideoFrame[1]);
            if (HI_SUCCESS != s32Ret)
            {
                WK_PRT("Error(%#x),HI_MPI_VPSS_ReleaseChnFrame failed,Grp(%d) chn(%d)!\n",
                    s32Ret,s32VpssGrp,as32VpssChn[1]);
            }
    }
    return NULL;
}


/******************************************************************************
 * function : yolov3 detect 
 ******************************************************************************/
static void *WK_DetectThread(void *pArgs)
{
    HI_S32 s32Ret;
    VIDEO_FRAME_INFO_S stYoloFrmInfo;
    while (bStartSOT)
    {
        WK_MUTEX_LOCK(s_stSotThread.GetDetFrmMutex);
        if(enYoloProcStat == YOLO_PROC_START)
        {
            memcpy(&stYoloFrmInfo, &stExtFrmInfo, sizeof(VIDEO_FRAME_INFO_S));
        }
        else
        {
            WK_MUTEX_UNLOCK(s_stSotThread.GetDetFrmMutex);
            continue;
        }
        WK_MUTEX_UNLOCK(s_stSotThread.GetDetFrmMutex);

        /*************YOLOv3 Detecting***********/
        WK_YOLO_RECT_ARRAY_S Det_stRect = {0};
        s32Ret = wk_yolov3_run(&stYoloFrmInfo,&Det_stRect);
        WK_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, YOLOV3_FAIL_0, "Error,yolov3_run failed!\n");
        /***************************************/
        
        // >>>>> Filter Object
        WK_RECT_ARRAY_S Obj_stRect = {0};
        WK_YOLO_CLASS Keep_Class = Person;
        Filter_Object(&Det_stRect,&Obj_stRect,Keep_Class);
        // <<<<< Filter Object

        // //把选择框给到预测框，做ＩＯＵ
        // if(bYOLO_SelectBoxFlag)
        // {
        //     KF_PreRect.fX = stRectInfo.fX;
        //     KF_PreRect.fY = stRectInfo.fY;
        //     KF_PreRect.fWidth = stRectInfo.fWidth;
        //     KF_PreRect.fHeight = stRectInfo.fHeight;
        // }


        // >>>>>IOU (objBbox with Predect bbox)
        
        if(Obj_stRect.u16Num == 0)
        {
            bMatch = false;
        }
        else
        {
            // bMatch = true;
            #if 1
            /*IOU:last kalman predict bbox WITH all bbox detect bbox*/
            int Ret;
            Ret = Kalman_Filter_ObjIOU(&Obj_stRect, &KF_PreRect, &YOLO_MatchRect);   //获取匹配到的ｂｂｏｘ，输送给ＫＣＦ
            if(Ret)
            {
                bMatch = true;
            }
            else
            {
                bMatch = false;
            }
            #endif
        }
        // <<<<< IOU
       

    }

    YOLOV3_FAIL_0:
        s32Ret = wk_yolov3_clear();
        if (HI_SUCCESS != s32Ret)
        {
             printf("wk_yolov3_clear fault!!!\n");
        }

    return NULL;
}


/******************************************************************************
 * function : KCF + KF track 
 ******************************************************************************/
static void *WK_TrackingThread(void *pArgs)
{
    /***********KＣＦ Prm***********/
    HI_S32 s32Ret;
    VIDEO_FRAME_INFO_S stFrmInfo;

    /*********************************/


    /***********Kalman Prm************/
    int stateSize = 6;  //state的维度
    int measSize = 4;   //测量维度
    int contrSize = 0;  //控制向量
    unsigned int type = 5; //创建的matrices  CV_32F
    KalmanFilter kf = KalmanFilter_OBJ_Create(stateSize, measSize, contrSize, type);


    struct timeval tv;
    long t1, t2;
   
    bool found = false;  //object found flag
    int notFoundCount = 0;
    /*********************************/


    while (bStartSOT)
    {
         /*获取帧*/
        WK_MUTEX_LOCK(s_stSotThread.GetTraFrmMutex);
        memcpy(&stFrmInfo, &stBaseFrmInfo, sizeof(VIDEO_FRAME_INFO_S));
        WK_MUTEX_UNLOCK(s_stSotThread.GetTraFrmMutex);
        //memcpy(&stShowFrame, &pstQueueNode->stFrameInfo, sizeof(VIDEO_FRAME_INFO_S)); //传递图像到显示线程


        /**************KF + KCF****************/
        /*获取时间差*/
        struct timeval prectv = tv;
        gettimeofday(&tv, NULL);
        t1 = tv.tv_sec - prectv.tv_sec;
        t2 = tv.tv_usec - prectv.tv_usec;
        double dT = (double) (t1  + t2 / 1000000); 

        if(found)
        {
            WK_FLOAT_RECT_S Pre_stRect;
            KalmanFilter_Predict(kf,dT,&KF_PreRect);

            printf("PreX:%f\n",Pre_stRect.fX);
            printf("PreY:%f\n",Pre_stRect.fY);
            printf("PreW:%f\n",Pre_stRect.fWidth);
            printf("PreZ:%f\n",Pre_stRect.fHeight);
        }


        /*========KCF=======*/
        WK_KCF_INPUT_PRM stkcf_prm;
        //输入图像及选择框到KCF算法
        if(bMatch)
        {
            WK_MUTEX_LOCK(s_stSotThread.DetectMutex);
            //YOLO检测结果输入
            stkcf_prm.stInputBboxInfo.fX = YOLO_MatchRect.fX;
            stkcf_prm.stInputBboxInfo.fY = YOLO_MatchRect.fY;
            stkcf_prm.stInputBboxInfo.fWidth = YOLO_MatchRect.fWidth;
            stkcf_prm.stInputBboxInfo.fHeight = YOLO_MatchRect.fHeight;
            stkcf_prm.bFlashSelectBbox = true;
            stkcf_prm.stKcfFrmInfo = stFrmInfo;   //get base frame
            WK_MUTEX_UNLOCK(s_stSotThread.DetectMutex);
        }
        else
        {
            //人工框选结果输入
            stkcf_prm.stInputBboxInfo.fX = stRectInfo.fX;
            stkcf_prm.stInputBboxInfo.fY = stRectInfo.fY;
            stkcf_prm.stInputBboxInfo.fWidth = stRectInfo.fWidth;
            stkcf_prm.stInputBboxInfo.fHeight = stRectInfo.fHeight;
            stkcf_prm.bFlashSelectBbox = bKCF_SelectBoxFlag;
            stkcf_prm.stKcfFrmInfo = stFrmInfo;   //get base frame
        }
        //KCF推理
        s32Ret = wk_kcf_proc(&stkcf_prm);
        if(s32Ret != HI_SUCCESS)
        {   
            printf("wk_kcf_proc failed!\n");
        }
        bKCF_SelectBoxFlag = HI_FALSE;

        /*========KCF END=======*/


        //如果重新选择框，重新初始化ＫＡＬＭＡＮ
        if (bKF_SelectBoxFlag)  
        {
           found = false;
           KalmanFilter_Update(kf,&stRectInfo,&found);
           bKF_SelectBoxFlag = false;
           continue;
        }


        if(!bMatch)
        {
            notFoundCount++;
            if(notFoundCount%90 == 0)
            {
                //printf("notFoundCount:%d\n",notFoundCount);
            }
            
            if( notFoundCount >= 100 )
            {
                found = false;
            }
            /*else
                kf.statePost = state;*/

            //如果匹配不上，卡尔曼使用ＫＣＦ的预测框作为观测值
            if(notFoundCount < 100)
            {
                if(stkcf_prm.stKcfRes.bTrackState)//如果ＫＣＦ跟踪成功，则更新ＫＡＬＭＡＮ。
                {
                    KalmanFilter_Update(kf,&stkcf_prm.stKcfRes.stResBbox,&found);
                }
            }
        }
        else
        {
            //如果匹配，卡尔曼使用YOLO的预测框作为观测值
            notFoundCount = 0;
            //KalmanFilter_Update(kf,&Match_Rect,&found);
            KalmanFilter_Update(kf,&YOLO_MatchRect,&found);
        }

        /***************************************/




       



        /************ To do KCF **************/


        /****************************************/
    


    }
    /*释放kalman对象*/
    KalmanFilter_OBJ_Destroy(kf);
    
    return NULL;
}




int Sot_Thread_Init(void)
{
    int Ret;
    /*初始化变量*/
    memset(&stBaseFrmInfo,0x00,sizeof(VIDEO_FRAME_INFO_S));
    memset(&stExtFrmInfo,0x00,sizeof(VIDEO_FRAME_INFO_S));

    bKCF_SelectBoxFlag = false;
    bKF_SelectBoxFlag = false;
    bYOLO_SelectBoxFlag = false;

    /*初始化ＹＯＬＯ检测状态*/
    enYoloProcStat = YOLO_PROC_END;


    /*初始化YOLOV3*/
    const char *model_path = "./data/nnie_model/detection/inst_yolov3_cycle.wk";
    wk_yolov3_load_model(model_path);

    /*初始化KCF*/
    wk_kcf_prm_init();


    memset_s(&s_stSotThread, sizeof(WK_SOT_THREAD_S), 0, sizeof(WK_SOT_THREAD_S));


    /*初始化锁*/
    // WK_MUTEX_INIT_LOCK(s_stSotThread.CaptureMutex);
    WK_MUTEX_INIT_LOCK(s_stSotThread.GetDetFrmMutex);
    WK_MUTEX_INIT_LOCK(s_stSotThread.DetectMutex);
    WK_MUTEX_INIT_LOCK(s_stSotThread.GetTraFrmMutex);


    /******************************************
    Create work thread
    ******************************************/
    bStartSOT = true;   
    char acThreadName[16] = {0};
    

    /*TrackingThread*/
    snprintf(acThreadName, 16, "TrackingThread");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0, 0, 0);
    pthread_create(&t_TrackingThread, 0, WK_TrackingThread, NULL);


    /*DetectThread*/
    snprintf(acThreadName, 16, "DetectThread");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0, 0, 0);
    pthread_create(&t_DetectThread, 0, WK_DetectThread, NULL);


    /*CaptureThread*/
    snprintf(acThreadName, 16, "CaptureThread");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0, 0, 0);
    pthread_create(&t_CaptureThread, 0, WK_CaptureThread, NULL);


    

    //  /*ShowThread*/
    // snprintf(acThreadName, 16, "ShowThread");
    // prctl(PR_SET_NAME, (unsigned long)acThreadName, 0, 0, 0);
    // pthread_create(&t_ShowThread, 0, WK_ShowThread, NULL);

    
    return Ret;

    
    FAIL_0:
        return Ret;
}


void Sot_Thread_DeInit(void)
{

    bStartSOT = false;

    pthread_join(t_TrackingThread, NULL);
    t_DetectThread = 0;

    pthread_join(t_DetectThread, NULL);
    t_DetectThread = 0;

    pthread_join(t_CaptureThread, NULL);
    t_CaptureThread = 0;




    WK_MUTEX_DESTROY(s_stSotThread.GetTraFrmMutex);
    WK_MUTEX_DESTROY(s_stSotThread.GetDetFrmMutex);
    WK_MUTEX_DESTROY(s_stSotThread.DetectMutex);

   
    /*去初始化YOLOV3*/
    wk_yolov3_clear();

    /*去初始化ＫＣＦ*/
    wk_kcf_prm_deinit();
}