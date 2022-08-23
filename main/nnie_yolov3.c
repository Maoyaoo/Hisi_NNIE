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

#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_svp.h"
#include "sample_comm.h"
#include "sample_comm_svp.h"
#include "sample_comm_nnie.h"
#include "sample_comm_ive.h"


#include "wk_yolov3.h"


void SAMPLE_IVE_Kcf_HandleSig(HI_S32 s32Signo);


WK_RECT_ARRAY_S stRect = {0};

static SAMPLE_VI_CONFIG_S s_stViConfig = {0};
static SAMPLE_VO_CONFIG_S s_stVoConfig = {0};

static SAMPLE_IVE_SWITCH_S s_stYolov3Switch = {HI_FALSE,HI_FALSE};


static pthread_t s_Yolov3_Detect_Thread = 0;
static pthread_t s_VIVO_HDMI_Show_Thread = 0;

HI_BOOL s_bYOLOv3StopSignal;
HI_BOOL s_bVIVOStopSignal;

VIDEO_FRAME_INFO_S *stDetFrmInfo;



HI_S32 SAMPLE_IVE_DispProcess(VIDEO_FRAME_INFO_S *pstFrameInfo, SAMPLE_RECT_ARRAY_S *pstRect)
{
    HI_S32 s32Ret;
    VO_LAYER voLayer = 0;
    VO_CHN voChn = 0;
    HI_S32 s32MilliSec = 20000;

    s32Ret = SAMPLE_COMM_VGS_FillRect(pstFrameInfo, pstRect, 0x0000FF00);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),SAMPLE_COMM_VGS_FillRect failed!\n", s32Ret);
    }

    s32Ret = HI_MPI_VO_SendFrame(voLayer, voChn, pstFrameInfo, s32MilliSec);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),HI_MPI_VO_SendFrame failed!\n", s32Ret);
        return s32Ret;
    } 

    return s32Ret;
}


/******************************************************************************
 * function : VIVO HDMI display thread entry
 ******************************************************************************/
static HI_VOID *VIVO_HDMI_Showing(HI_VOID *pArgs)
{
    HI_S32 s32Ret;

    VIDEO_FRAME_INFO_S stBaseFrmInfo;
    VIDEO_FRAME_INFO_S stExtFrmInfo;
    HI_S32 s32MilliSec = 20000;

    HI_S32 s32VpssGrp = 0;
    HI_S32 as32VpssChn[] = {VPSS_CHN0, VPSS_CHN1};

    VO_LAYER voLayer = 0;
    VO_CHN voChn = 0;

   
    while (HI_FALSE == s_bVIVOStopSignal)
    {
        /*vpss 1 416*416 to YOLOv3*/
        s32Ret = HI_MPI_VPSS_GetChnFrame(s32VpssGrp, as32VpssChn[1], &stExtFrmInfo, s32MilliSec);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_VPSS_GetChnFrame failed, VPSS_GRP(%d), VPSS_CHN(%d)!\n",
                s32Ret,s32VpssGrp, as32VpssChn[1]);
            continue;
        }
        stDetFrmInfo = &stExtFrmInfo;

        /*vpss 1 1920*1080 to VO*/
        s32Ret = HI_MPI_VPSS_GetChnFrame(s32VpssGrp, as32VpssChn[0], &stBaseFrmInfo, s32MilliSec);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
            "Error(%#x),HI_MPI_VPSS_GetChnFrame failed, VPSS_GRP(%d), VPSS_CHN(%d)!\n",
            s32Ret,s32VpssGrp, as32VpssChn[0]);
        
        


        // /*YOLOv3 Detecting*/
        // s32Ret = yolov3_run(&stExtFrmInfo,&stRect);
        // if (HI_SUCCESS != s32Ret)
        // {
        //     printf("yolov3_run fault!!!\n");
        //     s_bYOLOv3StopSignal = HI_TRUE;
        // }
       

        /*VGS Draw rect*/
        #if 1

        get_frame_bdbox(&stRect, stBaseFrmInfo.stVFrame.u32Width, stBaseFrmInfo.stVFrame.u32Height,HI_FALSE);

        s32Ret = SAMPLE_COMM_SVP_NNIE_YOLOV3_FillRect(&stBaseFrmInfo, &stRect, 0x0000FF00);				
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,"SAMPLE_COMM_SVP_NNIE_YOLOV3_FillRect failed, Error(%#x)!\n", s32Ret);
        #endif
        
        /*Show*/
        s32Ret = HI_MPI_VO_SendFrame(voLayer, voChn, &stBaseFrmInfo, s32MilliSec);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_VO_SendFrame failed!\n", s32Ret);
            return s32Ret;
        } 



    BASE_RELEASE:
        s32Ret = HI_MPI_VPSS_ReleaseChnFrame(s32VpssGrp,as32VpssChn[0], &stBaseFrmInfo);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_VPSS_ReleaseChnFrame failed,Grp(%d) chn(%d)!\n",
                s32Ret,s32VpssGrp,as32VpssChn[0]);
        }

    EXT_RELEASE:
        s32Ret = HI_MPI_VPSS_ReleaseChnFrame(s32VpssGrp,as32VpssChn[1], &stExtFrmInfo);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_VPSS_ReleaseChnFrame failed,Grp(%d) chn(%d)!\n",
                s32Ret,s32VpssGrp,as32VpssChn[1]);
        }
       
        
    }

    return NULL;
}



/******************************************************************************
 * function : YoloV3 Detecting and vo display thread entry
 ******************************************************************************/
static HI_VOID *WK_YOLOV3_Detecting(HI_VOID *pArgs)
{
    HI_S32 s32Ret;

    struct timeval tv1;
    struct timeval tv2;
    long t1, t2, time;

   
    while (HI_FALSE == s_bYOLOv3StopSignal)
    {
        gettimeofday(&tv1, NULL);

        #if 1
        /*YOLOv3 Detecting*/
        s32Ret = yolov3_run(stDetFrmInfo,&stRect);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV3_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
	    "Error,yolov3_run failed!\n");
        if (HI_SUCCESS != s32Ret)
        {
            printf("yolov3_run fault!!!\n");
            s_bYOLOv3StopSignal = HI_TRUE;
        }
        #endif

        gettimeofday(&tv2, NULL);
        t1 = tv2.tv_sec - tv1.tv_sec;
        t2 = tv2.tv_usec - tv1.tv_usec;
        time = (long) (t1 * 1000 + t2 / 1000);
        printf("NNIE inference time : %dms\n", time);
       
       
    }
    YOLOV3_FAIL_0:
        s32Ret = yolov3_clear();
        if (HI_SUCCESS != s32Ret)
        {
             printf("yolov3_clear fault!!!\n");
        }

    return NULL;
}


/******************************************************************************
* function : Init Vb
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_VbInit_yolov3(PIC_SIZE_E *paenSize,SIZE_S *pastSize,HI_U32 u32VpssChnNum)
{
    HI_S32 s32Ret;
    HI_U32 i;
    HI_U64 u64BlkSize;
    VB_CONFIG_S stVbConf;

    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 128;


    for (i = 0; i < u32VpssChnNum; i++)
    {
        s32Ret = SAMPLE_COMM_SYS_GetPicSize(paenSize[i], &pastSize[i]);

//		pastSize[1].u32Width  = 800;
//		pastSize[1].u32Height = 600;
//6666666

		pastSize[1].u32Width  = 416;
		pastSize[1].u32Height = 416;

		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, VB_FAIL_0,
            "SAMPLE_COMM_SYS_GetPicSize failed,Error(%#x)!\n",s32Ret);
		
        u64BlkSize = COMMON_GetPicBufferSize(pastSize[i].u32Width, pastSize[i].u32Height,
                            SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
        /* comm video buffer */
        stVbConf.astCommPool[i].u64BlkSize = u64BlkSize;
        // stVbConf.astCommPool[i].u32BlkCnt  = 10;
        stVbConf.astCommPool[i].u32BlkCnt  = 16;
    }

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, VB_FAIL_1,
        "SAMPLE_COMM_SYS_Init failed,Error(%#x)!\n", s32Ret);

    return s32Ret;
VB_FAIL_1:
    SAMPLE_COMM_SYS_Exit();
VB_FAIL_0:

    return s32Ret;
}


/******************************************************************************
* function : Start Vi/Vpss/Venc/Vo
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_StartViVpssVencVo_yolov3(SAMPLE_VI_CONFIG_S *pstViConfig,
    SAMPLE_IVE_SWITCH_S *pstSwitch,PIC_SIZE_E *penExtPicSize)
{
    SIZE_S 			astSize[VPSS_CHN_NUM];
    PIC_SIZE_E 		aenSize[VPSS_CHN_NUM];
    VI_CHN_ATTR_S 	stViChnAttr;
    SAMPLE_RC_E 	enRcMode 		= SAMPLE_RC_CBR;
    PAYLOAD_TYPE_E 	enStreamType 	= PT_H264;
    HI_BOOL bRcnRefShareBuf         = HI_FALSE;
    VENC_GOP_ATTR_S stGopAttr;
    VI_DEV 			ViDev0 			= 0;
    VI_PIPE 		ViPipe0 		= 0;
    VI_CHN 			ViChn 			= 0;
    HI_S32 			s32ViCnt 		= 1;
    HI_S32 			s32WorkSnsId  	= 0;
    VPSS_GRP 		VpssGrp 		= 0;
    HI_S32 			s32Ret 			= HI_SUCCESS;
    VENC_CHN 		VeH264Chn 		= 0;
    WDR_MODE_E 		enWDRMode 		= WDR_MODE_NONE;
    DYNAMIC_RANGE_E enDynamicRange 	= DYNAMIC_RANGE_SDR8;
    PIXEL_FORMAT_E 	enPixFormat 	= PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    VIDEO_FORMAT_E 	enVideoFormat  	= VIDEO_FORMAT_LINEAR;
    COMPRESS_MODE_E enCompressMode 	= COMPRESS_MODE_NONE;
    VI_VPSS_MODE_E 	enMastPipeMode 	= VI_ONLINE_VPSS_OFFLINE;

    memset(pstViConfig,0,sizeof(*pstViConfig));

    SAMPLE_COMM_VI_GetSensorInfo(pstViConfig);
    pstViConfig->s32WorkingViNum                           = s32ViCnt;

    pstViConfig->as32WorkingViId[0]                        = 0;
    pstViConfig->astViInfo[0].stSnsInfo.MipiDev            = SAMPLE_COMM_VI_GetComboDevBySensor(pstViConfig->astViInfo[0].stSnsInfo.enSnsType, 0);
    pstViConfig->astViInfo[0].stSnsInfo.s32BusId           = 0;

    pstViConfig->astViInfo[0].stDevInfo.ViDev              = ViDev0;
    pstViConfig->astViInfo[0].stDevInfo.enWDRMode          = enWDRMode;

    pstViConfig->astViInfo[0].stPipeInfo.enMastPipeMode    = enMastPipeMode;
    pstViConfig->astViInfo[0].stPipeInfo.aPipe[0]          = ViPipe0;
    pstViConfig->astViInfo[0].stPipeInfo.aPipe[1]          = -1;
    pstViConfig->astViInfo[0].stPipeInfo.aPipe[2]          = -1;
    pstViConfig->astViInfo[0].stPipeInfo.aPipe[3]          = -1;

    pstViConfig->astViInfo[0].stChnInfo.ViChn              = ViChn;
    pstViConfig->astViInfo[0].stChnInfo.enPixFormat        = enPixFormat;
    pstViConfig->astViInfo[0].stChnInfo.enDynamicRange     = enDynamicRange;
    pstViConfig->astViInfo[0].stChnInfo.enVideoFormat      = enVideoFormat;
    pstViConfig->astViInfo[0].stChnInfo.enCompressMode     = enCompressMode;

    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.enSnsType, &aenSize[0]);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_0,
        "Error(%#x),SAMPLE_COMM_VI_GetSizeBySensor failed!\n",s32Ret);

    aenSize[1] = *penExtPicSize; //SET THE YOLO PROCESS SIZE

    /******************************************
     step  1: Init vb
    ******************************************/
    s32Ret = SAMPLE_COMM_IVE_VbInit_yolov3(aenSize,astSize,VPSS_CHN_NUM); //translate w and h
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_0,
        "Error(%#x),SAMPLE_COMM_IVE_VbInit failed!\n",s32Ret);
    /******************************************
     step 2: Start vi
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_SetParam(pstViConfig);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_1,
        "Error(%#x),SAMPLE_COMM_VI_SetParam failed!\n",s32Ret);

    s32Ret = SAMPLE_COMM_VI_StartVi(pstViConfig);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_1,
        "Error(%#x),SAMPLE_COMM_VI_StartVi failed!\n",s32Ret);
    /******************************************
     step 3: Start vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_IVE_StartVpss(astSize,VPSS_CHN_NUM);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_2,
        "Error(%#x),SAMPLE_IVS_StartVpss failed!\n",s32Ret);
    /******************************************
      step 4: Bind vpss to vi
     ******************************************/
    s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe0, ViChn, VpssGrp);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_3,
        "Error(%#x),SAMPLE_COMM_VI_BindVpss failed!\n",s32Ret);
    //Set vi frame
    s32Ret = HI_MPI_VI_GetChnAttr(ViPipe0, ViChn,&stViChnAttr);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_4,
        "Error(%#x),HI_MPI_VI_GetChnAttr failed!\n",s32Ret);

    s32Ret = HI_MPI_VI_SetChnAttr(ViPipe0, ViChn,&stViChnAttr);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_4,
        "Error(%#x),HI_MPI_VI_SetChnAttr failed!\n",s32Ret);
    /******************************************
     step 5: Start Vo
     ******************************************/
    #if 0
    if (HI_TRUE == pstSwitch->bVo)
    {
        s32Ret = SAMPLE_COMM_IVE_StartVo();
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_4,
            "Error(%#x),SAMPLE_COMM_IVE_StartVo failed!\n", s32Ret);
    }
    #endif
     /*config vo*/
    SAMPLE_COMM_VO_GetDefConfig(&s_stVoConfig);
    s_stVoConfig.enDstDynamicRange = enDynamicRange;
    s_stVoConfig.enVoIntfType = VO_INTF_HDMI;
    s_stVoConfig.enPicSize = PIC_1080P;

    /*start vo*/
    s32Ret = SAMPLE_COMM_VO_StartVO(&s_stVoConfig);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_4,
                           "Error(%#x),SAMPLE_COMM_VO_StartVO failed!\n", s32Ret);



    /******************************************
     step 6: Start Venc
    ******************************************/
    if (HI_TRUE == pstSwitch->bVenc)
    {
        s32Ret = SAMPLE_COMM_VENC_GetGopAttr(VENC_GOPMODE_NORMALP,&stGopAttr);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_5,
            "Error(%#x),SAMPLE_COMM_VENC_GetGopAttr failed!\n",s32Ret);
        s32Ret = SAMPLE_COMM_VENC_Start(VeH264Chn, enStreamType,aenSize[0],enRcMode,0,bRcnRefShareBuf,&stGopAttr);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_5,
            "Error(%#x),SAMPLE_COMM_VENC_Start failed!\n",s32Ret);
        s32Ret = SAMPLE_COMM_VENC_StartGetStream(&VeH264Chn, 1);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_6,
            "Error(%#x),SAMPLE_COMM_VENC_StartGetStream failed!\n",s32Ret);
    }

    return HI_SUCCESS;

END_INIT_6:
    if (HI_TRUE == pstSwitch->bVenc)
    {
        SAMPLE_COMM_VENC_Stop(VeH264Chn);
    }
END_INIT_5:
    if (HI_TRUE == pstSwitch->bVo)
    {
        SAMPLE_COMM_IVE_StopVo();
    }
END_INIT_4:
    SAMPLE_COMM_VI_UnBind_VPSS(ViPipe0, ViChn, VpssGrp);
END_INIT_3:
    SAMPLE_COMM_IVE_StopVpss(VPSS_CHN_NUM);
END_INIT_2:
    SAMPLE_COMM_VI_StopVi(pstViConfig);
END_INIT_1:    //system exit
    SAMPLE_COMM_SYS_Exit();
    memset(pstViConfig,0,sizeof(*pstViConfig));
END_INIT_0:

    return s32Ret;
}





/******************************************************************************
 * function : YOLOV3 VI TO VO
 ******************************************************************************/
int main(int argc, char *argv[])
{
    
    signal(SIGINT, SAMPLE_IVE_Kcf_HandleSig);
    signal(SIGTERM, SAMPLE_IVE_Kcf_HandleSig);

    const char *model_path = "./data/nnie_model/detection/inst_yolov3_cycle.wk";
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR acThreadName[16] = {0};
    SIZE_S 	stSize;
    //PIC_SIZE_E enSize = PIC_CIF;
    PIC_SIZE_E enSize = {0};


    /*Sys init*/
    SAMPLE_COMM_SVP_CheckSysInit(); //去初始化 MPP 系统->始化 MPP 系统

    /******************************************
    step 1: start vi vpss vo
    ******************************************/
    s_stYolov3Switch.bVenc = HI_FALSE;
    s_stYolov3Switch.bVo   = HI_TRUE;
    
    s32Ret = SAMPLE_COMM_IVE_StartViVpssVencVo_yolov3(&s_stViConfig,&s_stYolov3Switch,&enSize);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, YOLOV3_FAIL_1,
        "Error(%#x),SAMPLE_COMM_IVE_StartViVpssVencVo failed!\n", s32Ret);

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enSize, &stSize);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, YOLOV3_FAIL_1,
        "Error(%#x),SAMPLE_COMM_SYS_GetPicSize failed!\n", s32Ret);

	stSize.u32Width  = 416;
	stSize.u32Height = 416;

    /******************************************
    step 2: init YOLO NNIE param
    ******************************************/
    #if 1
    s32Ret = yolov3_load_model(model_path);
    if (HI_SUCCESS != s32Ret)
    {
        printf("load model fault!\n");
    }
    else{
        printf("load model success!!!!!!!!!\n");
    }
    #endif


    /******************************************
     step 3: Create work thread
    ******************************************/

    /*VIVO HDMI Thread*/
    snprintf(acThreadName, 16, "VIVOHDMIShow");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0, 0, 0);
    pthread_create(&s_VIVO_HDMI_Show_Thread, 0, VIVO_HDMI_Showing, NULL);


    /*Detecting Thread*/
    snprintf(acThreadName, 16, "YOLOv3Detecting");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0, 0, 0);
    pthread_create(&s_Yolov3_Detect_Thread, 0, WK_YOLOV3_Detecting, NULL);

    


    SAMPLE_PAUSE();

    s_bYOLOv3StopSignal = HI_TRUE;
    s_bVIVOStopSignal = HI_TRUE;

    pthread_join(s_VIVO_HDMI_Show_Thread, HI_NULL);
    s_VIVO_HDMI_Show_Thread = 0;

    pthread_join(s_Yolov3_Detect_Thread, NULL);
    s_Yolov3_Detect_Thread = 0;


YOLOV3_FAIL_1:
    SAMPLE_COMM_IVE_StopViVpssVencVo(&s_stViConfig,&s_stYolov3Switch);
    return 0;
}


/******************************************************************************
 * function : Kcf sample signal handle
 ******************************************************************************/
void SAMPLE_IVE_Kcf_HandleSig(HI_S32 s32Signo)
{
    printf("====== signal handle deinit ======\n");
    signal(SIGINT,SIG_IGN);
    signal(SIGTERM,SIG_IGN);

    if (SIGINT == s32Signo || SIGTERM == s32Signo)
    {
        VI_PIPE ViPipe = 0;
        VI_CHN ViChn = 0;
        VPSS_GRP VpssGrp = 0;
        HI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {1, 1, 0};

        s_bYOLOv3StopSignal = HI_TRUE;
        s_bVIVOStopSignal = HI_TRUE;
       
        if (0 != s_Yolov3_Detect_Thread)
        {
            pthread_join(s_Yolov3_Detect_Thread, NULL);
            s_Yolov3_Detect_Thread = 0;
        }

         if (0 != s_bVIVOStopSignal)
        {
            pthread_join(s_bVIVOStopSignal, NULL);
            s_bVIVOStopSignal = 0;
        }
        SAMPLE_COMM_IVE_StopViVpssVencVo(&s_stViConfig,&s_stYolov3Switch);
        // SAMPLE_COMM_VO_StopVO(&s_stVoConfig);
        // SAMPLE_COMM_VI_UnBind_VPSS(ViPipe, ViChn, VpssGrp);
        // SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
        // SAMPLE_COMM_VI_StopVi(&s_stViConfig);
        // SAMPLE_COMM_SYS_Exit();
    }
}
