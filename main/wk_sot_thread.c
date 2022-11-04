#include <sys/time.h>

#include "mpi_vpss.h"
#include "mpi_vo.h"


#include "wk_svp_type.h"
#include "wk_kcf.h"
#include "wk_yolov3.h"
#include "kalman_filter.h"
#include "wk_utils.h"

#include "wk_sot_queue.h"


#include "wk_sot_thread.h"


#define VO_Display 1


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



//跟踪标志　
bool stopFlag = false;
//中断队列标志
bool setBreak = false;

//声明跟踪线程标识符ID 
static pthread_t t_CaptureThread = 0;   //CaptureThread


// pthread_mutex_t DetectMutex;
// pthread_mutex_t GetTraFrmMutex;
// pthread_mutex_t CaptureMutex;/*跟踪显示图像*/
// pthread_mutex_t GetDetFrmMutex; /*检测图像*/


//静态方式初始化互斥锁和条件变量
pthread_mutex_t QueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t QueuePushed = PTHREAD_COND_INITIALIZER;


/***********************应用队列接口**************************/

#define SOT_QUEUE_LEN 16  //队列长度
SOT_QUEUE_S *stQueueHead = NULL;  //队列头

//创建队列
void SotQueueInit(void)
{
	stQueueHead = SOT_QueueCreate(SOT_QUEUE_LEN);
}

//增加图片进入队列(生产者)
int AddNewFrame(SotFrameInfo *pstframeInfo)
{
	HI_S32 s32Ret;
    pthread_mutex_lock(&QueueMutex);         //加锁
	s32Ret = SOT_QueueAddNode(stQueueHead,pstframeInfo);
	if (HI_SUCCESS != s32Ret)
	{
		printf("Error(%#x),SOT_QueueAddNode failed!\n",s32Ret);
	}
    pthread_mutex_unlock(&QueueMutex);       //解锁
    pthread_cond_signal(&QueuePushed); //唤醒消费者线程
	return s32Ret;
}


//获取最新没有检测的图像帧(消费者)
SotFrameInfo *GetLastUndetectedFrame()
{
    pthread_mutex_lock(&QueueMutex);         //加锁
    //如果队列是空的或者队列最后一个元素的检测状态是处理了的,则进入等待，直到生产者发送信号过来
	while (SOT_QueueIsEmpty(stQueueHead) || stQueueHead->rear->stFrameInfo.m_inDetector != StateNotProcessed)
	{
		/* code */
        if(setBreak)
        {
            break;
        }

		//阻塞线程并且释放锁，等待生产者发送信号
        pthread_cond_wait(&QueuePushed, &QueueMutex);   //我们通常在一个循环内使用该函数，/* 程序会停在这里，等待信号，信号来时，条件不满足 ，跳出循环*会重新加锁/

	}
    
    if (!setBreak)
    {
        SotFrameInfo *frameInfo = &stQueueHead->rear->stFrameInfo; //获取队列最新一帧图像地址
        assert(frameInfo->m_inDetector == StateNotProcessed);
        assert(frameInfo->m_inTracker == StateNotProcessed);
        frameInfo->m_inDetector = StateInProcess; //赋值检测状态为在检测

        SOT_NODE_S *cur; //存储当前遍历的节点地址

        
        for (cur = &stQueueHead->rear; cur != &stQueueHead->front; cur = cur->prve) //循环到队列的开头
        {
            if(cur->stFrameInfo.m_inDetector == StateNotProcessed)
                cur->stFrameInfo.m_inDetector == StateSkipped;
            else
                break;
        }
        pthread_mutex_unlock(&QueueMutex);         //释放锁
        return frameInfo;
    }
    pthread_mutex_unlock(&QueueMutex);         //释放锁
    return NULL;
}


//查找没有跟踪的队列节点
SOT_NODE_S *SearchUntracked()
{
    SOT_NODE_S *res_it = stQueueHead->rear;
    for (SOT_NODE_S *it = stQueueHead->front; it != stQueueHead->rear; it = it->next) //循环到队列尾部
    {
        if(it->stFrameInfo.m_inDetector == StateInProcess || it->stFrameInfo.m_inDetector == StateNotProcessed)//如果检测的状态是在检测或者没检测的，直接返回
        {
            break;
        }
        else if (it->stFrameInfo.m_inTracker == StateNotProcessed)
        {
            res_it = it;
            break;
        }    
    }
    return res_it;
}


//获取第一张检测图像(消费者)
SotFrameInfo *GetFirstDetectedFrame()
{
    pthread_mutex_lock(&QueueMutex); //上锁
    SOT_NODE_S *it = SearchUntracked(); //查找没有被跟踪的队列节点
    while (it == stQueueHead->rear)
    {
        if(setBreak)
        {
            break;
        }

        //阻塞线程并且释放锁，等待生产者发送信号
        pthread_cond_wait(&QueuePushed, &QueueMutex);
        it = SearchUntracked(); //直到查找到的不是最后一张没有被跟踪的图像为止
    }
    if(!setBreak)
    {
        SotFrameInfo *frameInfo = &it->stFrameInfo; //获取没有被跟踪图像的地址
        assert(frameInfo->m_inTracker == StateNotProcessed);
        assert(frameInfo->m_inDetector != StateInProcess && frameInfo->m_inDetector != StateNotProcessed);
        frameInfo->m_inTracker = StateInProcess;
        pthread_mutex_unlock(&QueueMutex); //释放锁
        return frameInfo;
    }
    pthread_mutex_unlock(&QueueMutex); //释放锁
    return NULL;
}


//获取第一张被处理的图像并出列
int GetFirstProcessedFrame(SotFrameInfo *pstframeinfo)
{     
    // static SotFrameInfo frameinfo;                          
    pthread_mutex_lock(&QueueMutex); //上锁
    while (SOT_QueueIsEmpty(stQueueHead) || stQueueHead->front->stFrameInfo.m_inTracker != StateCompleted)
    {
        if(setBreak)
        {
            break;
        }
        //阻塞线程并且释放锁，等待生产者发送信号
        pthread_cond_wait(&QueuePushed, &QueueMutex);
    }
    if(!setBreak)
    {
        SOT_NODE_S *TmpNode = SOT_QueueGetNode(&stQueueHead); //出列
        memcpy(pstframeinfo, &TmpNode->stFrameInfo, sizeof(SotFrameInfo));
        SOT_QueueFreeNode(TmpNode); //释放节点
        pthread_mutex_unlock(&QueueMutex); //释放锁
        return 0;
    }
    pthread_mutex_unlock(&QueueMutex); //释放锁
    return -1;
}



/******************************************************************************
 * function : yolov3 detect 
 ******************************************************************************/
static void *WK_DetectThread(void *pArgs)
{
    HI_S32 s32Ret;
    VIDEO_FRAME_INFO_S stYoloFrmInfo;

    while (!stopFlag)
    {
       

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


    while (!stopFlag)
    {



    }
    return NULL;
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
    HI_S32 as32VpssChn[] = {VPSS_CHN0, VPSS_CHN1, VPSS_CHN2}; //0: 1080p     1:416       

    

    /*创建检测跟踪线程*/
    char acThreadName[16] = {0};
    static pthread_t t_DetectThread = 0;    //DetectThread
    static pthread_t t_TrackingThread = 0;  //TrackingThread
    

    /*DetectThread*/
    snprintf(acThreadName, 16, "DetectThread");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0, 0, 0);
    pthread_create(&t_DetectThread, 0, WK_DetectThread, NULL);

    /*TrackingThread*/
    snprintf(acThreadName, 16, "TrackingThread");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0, 0, 0);
    pthread_create(&t_TrackingThread, 0, WK_TrackingThread, NULL);



    while (!stopFlag)
    {
        /*************Capture frame***********************/
        SotFrameInfo frameInfo = {0}; //初始化帧变量，赋值帧索引
        

        /*vpss 0 1920*1080 to Track and Display*/
        s32Ret = HI_MPI_VPSS_GetChnFrame(s32VpssGrp, as32VpssChn[0], &astVideoFrame[0], s32MilliSec);
        WK_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
            "Error(%#x),HI_MPI_VPSS_GetChnFrame failed, VPSS_GRP(%d), VPSS_CHN(%d)!\n",
            s32Ret,s32VpssGrp, as32VpssChn[0]);

        // WK_MUTEX_LOCK(s_stSotThread.GetTraFrmMutex);
        // WK_MUTEX_UNLOCK(s_stSotThread.GetTraFrmMutex);
       
        /*vpss 1 416*416 to YOLOv3*/
        s32Ret = HI_MPI_VPSS_GetChnFrame(s32VpssGrp, as32VpssChn[1], &astVideoFrame[1], s32MilliSec);
        WK_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, EXT_RELEASE,
            "Error(%#x),HI_MPI_VPSS_GetChnFrame failed, VPSS_GRP(%d), VPSS_CHN(%d)!\n",
            s32Ret,s32VpssGrp, as32VpssChn[1]);
        
        /*把1080P图像和416*416图像放入队列　　ＴＯ　ＤＯ*/
        gettimeofday(&frameInfo.m_dt, NULL);//获取取帧时间
        memcpy(&frameInfo.m_TraFrame, &astVideoFrame[1], sizeof(VIDEO_FRAME_INFO_S));       //获取1080P图像
        memcpy(&frameInfo.m_DetFrame, &astVideoFrame[1], sizeof(VIDEO_FRAME_INFO_S));       //获取416*416图像
        frameInfo.m_inDetector = StateNotProcessed;
        frameInfo.m_inTracker = StateNotProcessed;

        /***************************************************************/


        /************Plot AND Display*************/
        /*根据推理结果画框:1.VO display;2．推流*/
    #if VO_Display
        s32Ret = HI_MPI_VO_SendFrame(0,0,&astVideoFrame[0],20000);
        if (HI_SUCCESS != s32Ret)
        {
            WK_PRT("Error(%#x),HI_MPI_VO_SendFrame failed!\n", s32Ret);
            return s32Ret;
        } 
    #else
        WK_RECT_ARRAY_S stRect_u = {0};
        WK_RECT_ARRAY_S stRect_f = {0};
 
        stRect_f.u16Num = 1;
        stRect_f.astRect[0].stRect_f = KF_PreRect;  //预测框反归一化
        RECT_TO_DENRMALIZATION(&stRect_f, &stRect_u, astVideoFrame[0].stVFrame.u32Width, astVideoFrame[0].stVFrame.u32Height);

        HI_U32 s32Bbox_X = stRect_u.astRect->stRect_u.uX;
        HI_U32 s32Bbox_Y = stRect_u.astRect->stRect_u.uY;
        HI_U32 s32Bbox_W = stRect_u.astRect->stRect_u.uWidth;
        HI_U32 s32Bbox_H = stRect_u.astRect->stRect_u.uHeight;  
        //printf("=== polt reat ===\n");
        wk_mpp_rgn_draw_rects(s32Bbox_X, s32Bbox_Y, s32Bbox_W, s32Bbox_H);
    #endif

        

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

    pthread_join(t_DetectThread, NULL);
    t_DetectThread = 0;

    pthread_join(t_TrackingThread, NULL);
    t_DetectThread = 0;
    return NULL;
}






/*初始化跟踪任务*/
int Sot_Thread_Init(void)
{

    /*初始化队列*/



    /*初始化线程*/
    char acThreadName[16] = {0};
    /*CaptureThread*/
    snprintf(acThreadName, 16, "CaptureThread");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0, 0, 0);
    pthread_create(&t_CaptureThread, 0, WK_CaptureThread, NULL);
}


/*去初始化跟踪任务*/
void Sot_Thread_DeInit(void)
{

    stopFlag = false;



    pthread_join(t_CaptureThread, NULL);
    t_CaptureThread = 0;




    // WK_MUTEX_DESTROY(s_stSotThread.GetTraFrmMutex);
    // WK_MUTEX_DESTROY(s_stSotThread.GetDetFrmMutex);
    // WK_MUTEX_DESTROY(s_stSotThread.DetectMutex);

   
    /*去初始化YOLOV3*/
    wk_yolov3_clear();

    /*去初始化ＫＣＦ*/
    wk_kcf_prm_deinit();
}