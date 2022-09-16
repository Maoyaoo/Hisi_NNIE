#include "kcf_core.h"



#define SAMPLE_IVE_KCF_NODE_MAX_NUM 64
//#define SAMPLE_IVE_KCF_NODE_MAX_NUM 1
#define SAMPLE_IVE_KCF_GAUSS_PEAK_TOTAL_SIZE 455680
#define SAMPLE_IVE_KCF_COS_WINDOW_TOTAL_SIZE 832
#define SAMPLE_IVE_KCF_COS_WINDOW_SIZE 64
#define SAMPLE_IVE_KCF_TEMP_BUF_SIZE 47616
#define SAMPLE_IVE_KCF_HOG_FEATRUE_BUF_SIZE 47616

IVE_HANDLE hIveHandle;
HI_BOOL bInstant = HI_TRUE;

HI_BOOL bFinish;
HI_BOOL bBlock = HI_TRUE;

static IVE_KCF_PRM_S s_stIveKcfInfo = {0}; // KCF tracking parameters
static IVE_KCF_BBOX_S astBbox[1] = {0};
HI_BOOL bTrackState = HI_FALSE;

static HI_S32 s32LastResPones = 0;

HI_S32 kcf_prm_init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Size;
    HI_U32 u32TotalSize;
    IVE_KCF_PRM_S *pstIveKcfInfo = &s_stIveKcfInfo;
    
    memset_s(pstIveKcfInfo, sizeof(IVE_KCF_PRM_S), 0, sizeof(IVE_KCF_PRM_S));

    s32Ret = HI_MPI_IVE_KCF_GetMemSize(SAMPLE_IVE_KCF_NODE_MAX_NUM, &u32Size);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, FAIL_0,
                           "Error(%#x),HI_MPI_IVE_KCF_GetMemSize failed!\n", s32Ret);
    /* (HOGFeatrue + Alpha + DstBuf) + Guasspeak + CosWinX + CosWinY + TmpBuf*/
    u32TotalSize = u32Size + SAMPLE_IVE_KCF_GAUSS_PEAK_TOTAL_SIZE + SAMPLE_IVE_KCF_COS_WINDOW_TOTAL_SIZE * 2 + SAMPLE_IVE_KCF_TEMP_BUF_SIZE;
    s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&pstIveKcfInfo->stTotalMem, u32TotalSize);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, FAIL_0,
                           "Error(%#x),SAMPLE_COMM_IVE_CreateMemInfo failed!\n", s32Ret);
    memset((HI_U8 *)(HI_UL)pstIveKcfInfo->stTotalMem.u64VirAddr, 0x0, u32TotalSize);

    pstIveKcfInfo->stListMem.u64PhyAddr = pstIveKcfInfo->stTotalMem.u64PhyAddr;
    pstIveKcfInfo->stListMem.u64VirAddr = pstIveKcfInfo->stTotalMem.u64VirAddr;
    pstIveKcfInfo->stListMem.u32Size = u32Size;

    pstIveKcfInfo->stGaussPeak.u64PhyAddr = pstIveKcfInfo->stListMem.u64PhyAddr + u32Size;
    pstIveKcfInfo->stGaussPeak.u64VirAddr = pstIveKcfInfo->stListMem.u64VirAddr + u32Size;
    pstIveKcfInfo->stGaussPeak.u32Size = SAMPLE_IVE_KCF_GAUSS_PEAK_TOTAL_SIZE;

    pstIveKcfInfo->stCosWinX.u64PhyAddr = pstIveKcfInfo->stGaussPeak.u64PhyAddr + SAMPLE_IVE_KCF_GAUSS_PEAK_TOTAL_SIZE;
    pstIveKcfInfo->stCosWinX.u64VirAddr = pstIveKcfInfo->stGaussPeak.u64VirAddr + SAMPLE_IVE_KCF_GAUSS_PEAK_TOTAL_SIZE;
    pstIveKcfInfo->stCosWinX.u32Size = SAMPLE_IVE_KCF_COS_WINDOW_TOTAL_SIZE;

    pstIveKcfInfo->stCosWinY.u64PhyAddr = pstIveKcfInfo->stCosWinX.u64PhyAddr + SAMPLE_IVE_KCF_COS_WINDOW_TOTAL_SIZE;
    pstIveKcfInfo->stCosWinY.u64VirAddr = pstIveKcfInfo->stCosWinX.u64VirAddr + SAMPLE_IVE_KCF_COS_WINDOW_TOTAL_SIZE;
    pstIveKcfInfo->stCosWinY.u32Size = SAMPLE_IVE_KCF_COS_WINDOW_TOTAL_SIZE;

    pstIveKcfInfo->stKcfProCtrl.stTmpBuf.u64PhyAddr = pstIveKcfInfo->stCosWinY.u64PhyAddr + SAMPLE_IVE_KCF_COS_WINDOW_TOTAL_SIZE;
    pstIveKcfInfo->stKcfProCtrl.stTmpBuf.u64VirAddr = pstIveKcfInfo->stCosWinY.u64VirAddr + SAMPLE_IVE_KCF_COS_WINDOW_TOTAL_SIZE;
    pstIveKcfInfo->stKcfProCtrl.stTmpBuf.u32Size = SAMPLE_IVE_KCF_TEMP_BUF_SIZE;

    pstIveKcfInfo->stKcfProCtrl.enCscMode = IVE_CSC_MODE_VIDEO_BT709_YUV2RGB;
    pstIveKcfInfo->stKcfProCtrl.u1q15InterFactor = 0.02 * 1024 * 32;
    pstIveKcfInfo->stKcfProCtrl.u0q16Lamda = 10;
    pstIveKcfInfo->stKcfProCtrl.u0q8Sigma = 0.5 * 256;
    pstIveKcfInfo->stKcfProCtrl.u4q12TrancAlfa = 0.2 * 4096;
    pstIveKcfInfo->stKcfProCtrl.u8RespThr = 32;

    pstIveKcfInfo->u3q5Padding = 1.5 * 32;

    pstIveKcfInfo->stKcfBboxCtrl.u32MaxBboxNum = SAMPLE_IVE_KCF_NODE_MAX_NUM;
    pstIveKcfInfo->stKcfBboxCtrl.s32RespThr = 0;


    s32Ret = HI_MPI_IVE_KCF_CreateObjList(&pstIveKcfInfo->stListMem, SAMPLE_IVE_KCF_NODE_MAX_NUM, &pstIveKcfInfo->stObjList);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, FAIL_2,
                           "Error(%#x),HI_MPI_IVE_KCF_CreateObjList failed!\n", s32Ret);

    s32Ret = HI_MPI_IVE_KCF_CreateGaussPeak(pstIveKcfInfo->u3q5Padding, &pstIveKcfInfo->stGaussPeak);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, FAIL_3,
                           "Error(%#x),HI_MPI_IVE_KCF_CreateGaussPeak failed!\n", s32Ret);

    s32Ret = HI_MPI_IVE_KCF_CreateCosWin(&pstIveKcfInfo->stCosWinX, &pstIveKcfInfo->stCosWinY);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, FAIL_3,
                           "Error(%#x),HI_MPI_IVE_KCF_CreateCosWin failed!\n", s32Ret);

    return s32Ret;

FAIL_3:
    IVE_MMZ_FREE(pstIveKcfInfo->stTotalMem.u64PhyAddr, pstIveKcfInfo->stTotalMem.u64VirAddr);
    (HI_VOID) HI_MPI_IVE_KCF_DestroyObjList(&pstIveKcfInfo->stObjList);
    return s32Ret;
FAIL_2:
    IVE_MMZ_FREE(pstIveKcfInfo->stTotalMem.u64PhyAddr, pstIveKcfInfo->stTotalMem.u64VirAddr);
    return s32Ret;
FAIL_1:
    IVE_MMZ_FREE(pstIveKcfInfo->stTotalMem.u64PhyAddr, pstIveKcfInfo->stTotalMem.u64VirAddr);
FAIL_0:
    return s32Ret;

}


HI_S32 kcf_prm_deinit(HI_VOID)
{
    IVE_KCF_PRM_S *pstIveKcfInfo = &s_stIveKcfInfo;
    (HI_VOID) HI_MPI_IVE_KCF_DestroyObjList(&pstIveKcfInfo->stObjList);
    IVE_MMZ_FREE(pstIveKcfInfo->stTotalMem.u64PhyAddr, pstIveKcfInfo->stTotalMem.u64VirAddr);
}


HI_S32 kcf_object_init(IVE_RECT_S24Q8_S *pstTrainRoiInfo,IVE_IMAGE_S *pstSrc)
{
    HI_S32 s32Ret;
    IVE_ROI_INFO_S astTrainRoiInfo[1] = {0};

    astTrainRoiInfo[0].u32RoiId = 0;
    astTrainRoiInfo[0].stRoi = *pstTrainRoiInfo;
    

    HI_U32 u32BboxNum = 0;

    /************Calibration parameters********************/
    printf("====== The Train Bbox Info ======\n");
    printf("X:%d\n", astTrainRoiInfo[0].stRoi.s24q8X / 256 & (~1));
    printf("Y:%d\n", astTrainRoiInfo[0].stRoi.s24q8Y / 256 & (~1));
    printf("H:%d\n", astTrainRoiInfo[0].stRoi.u32Height & (~1));
    printf("W:%d\n", astTrainRoiInfo[0].stRoi.u32Width & (~1));
    printf("=================================\n");
    /******************************************************/

    if(s_stIveKcfInfo.stObjList.u32TrackObjNum != 0)
    {
        s32Ret = HI_MPI_IVE_KCF_GetObjBbox(&s_stIveKcfInfo.stObjList, astBbox,
                                        &u32BboxNum, &s_stIveKcfInfo.stKcfBboxCtrl); /*Obtain target area tracking information*/
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_IVE_KCF_GetObjBbox failed!\n", s32Ret);
            return s32Ret;
        }
        astBbox[0].stRoiInfo.stRoi.s24q8X = pstTrainRoiInfo->s24q8X;
        astBbox[0].stRoiInfo.stRoi.s24q8Y = pstTrainRoiInfo->s24q8Y;
        astBbox[0].stRoiInfo.stRoi.u32Width = pstTrainRoiInfo->u32Width;
        astBbox[0].stRoiInfo.stRoi.u32Height = pstTrainRoiInfo->u32Height;


        astBbox[0].bRoiRefresh = HI_TRUE;
        s32Ret = HI_MPI_IVE_KCF_ObjUpdate(&s_stIveKcfInfo.stObjList, astBbox, u32BboxNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_IVE_KCF_ObjUpdate failed!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        s32Ret = HI_MPI_IVE_KCF_GetTrainObj(s_stIveKcfInfo.u3q5Padding, astTrainRoiInfo, 1,
                                        &s_stIveKcfInfo.stCosWinX, &s_stIveKcfInfo.stCosWinY, &s_stIveKcfInfo.stGaussPeak,
                                        &s_stIveKcfInfo.stObjList);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_IVE_KCF_GetTrainObj failed!\n", s32Ret);
            return s32Ret;
        }

        //printf("#######1 train num: %d \n", s_stIveKcfInfo.stObjList.u32TrainObjNum);
        //printf("#######1 track num: %d \n", s_stIveKcfInfo.stObjList.u32TrackObjNum);

        if (s_stIveKcfInfo.stObjList.u32TrainObjNum != 0)
        {
            s32Ret = HI_MPI_IVE_KCF_Process(&hIveHandle, pstSrc, &s_stIveKcfInfo.stObjList,
                                            &s_stIveKcfInfo.stKcfProCtrl, bInstant);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Error(%#x),HI_MPI_IVE_KCF_Process failed!\n", s32Ret);
                return s32Ret;
            }

            s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
            while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
            {
                usleep(100);
                s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
            }

            SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, HI_FALSE, "Error(%#x),HI_MPI_IVE_Query failed!\n", s32Ret);
        }

        // printf("#######2 train num: %d \n", s_stIveKcfInfo.stObjList.u32TrainObjNum);
        // printf("#######2 track num: %d \n", s_stIveKcfInfo.stObjList.u32TrackObjNum);

        s32Ret = HI_MPI_IVE_KCF_GetObjBbox(&s_stIveKcfInfo.stObjList, astBbox,
                                        &u32BboxNum, &s_stIveKcfInfo.stKcfBboxCtrl); /*Obtain target area tracking information*/
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_IVE_KCF_GetObjBbox failed!\n", s32Ret);
            return s32Ret;
        }

        // memcpy_s(astLastFrameBbox, sizeof(IVE_KCF_BBOX_S) * 1, astGetBbox, sizeof(IVE_KCF_BBOX_S) * 1);
    }
    printf("#######3 train num: %d \n", s_stIveKcfInfo.stObjList.u32TrainObjNum);
    printf("#######3 track num: %d \n", s_stIveKcfInfo.stObjList.u32TrackObjNum);
    printf("#######3 Free num: %d \n", s_stIveKcfInfo.stObjList.u32FreeObjNum);

    bTrackState = HI_TRUE;
    return s32Ret;
}


HI_S32 kcf_track_update(IVE_IMAGE_S *pstSrc, IVE_RECT_S24Q8_S *pstTrackBox, HI_BOOL *pbTracking_State)
{
    HI_S32 s32Ret;
    HI_S32 u32BboxNum = 1;
    
    if(!bTrackState)
    {
        *pbTracking_State = HI_FALSE;
        memset(pstTrackBox, 0, sizeof(IVE_RECT_S24Q8_S));
        return 0;
    }
    // if (s_stIveKcfInfo.stObjList.u32TrackObjNum != 0 )
    if (s_stIveKcfInfo.stObjList.u32TrackObjNum != 0 || s_stIveKcfInfo.stObjList.u32TrainObjNum != 0 )
    {
        
        /********************KCF track and reflash the bbox*****************/
        s32Ret = HI_MPI_IVE_KCF_Process(&hIveHandle, pstSrc, &s_stIveKcfInfo.stObjList,
                                        &s_stIveKcfInfo.stKcfProCtrl, bInstant);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_IVE_KCF_Process failed!\n", s32Ret);
            return s32Ret;
        }

        s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
        while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
        {
            usleep(100);
            s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
        }

        SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, HI_FALSE, "Error(%#x),HI_MPI_IVE_Query failed!\n", s32Ret);
        /*******************************************************************/

        s32Ret = HI_MPI_IVE_KCF_GetObjBbox(&s_stIveKcfInfo.stObjList, astBbox,
                                           &u32BboxNum, &s_stIveKcfInfo.stKcfBboxCtrl); /*Obtain target area tracking information*/
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_IVE_KCF_GetObjBbox failed!\n", s32Ret);
            return s32Ret;
        }
        #if 0
        s32Ret = HI_MPI_IVE_KCF_JudgeObjBboxTrackState(&stTemplRoiInfo, &astBbox[0], pbTracking_State); // currun frame ,last frame
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_IVE_KCF_JudgeObjBboxTrackState failed!\n", s32Ret);
            return s32Ret;
        }
        #endif

        // stTemplRoiInfo.stRoi = astBbox[0].stRoiInfo.stRoi;

        HI_FLOAT S32Rate = 1.0 * (astBbox[0].s32Response) / (s32LastResPones + astBbox[0].s32Response);

        if (S32Rate < 0.4)    /*track fault*/
        {
            //*pbTracking_State = HI_FALSE;
            bTrackState = HI_FALSE;
            //astBbox[0].bTrackOk = HI_FALSE;
        }
        else
        {
            //*pbTracking_State = HI_TRUE;
            bTrackState = HI_TRUE;
            //astBbox[0].bTrackOk = HI_TRUE;
        }

        #if 0

        HI_U32 s32ImgHeight = pstSrc->u32Height; /*Get Img Height*/
        HI_U32 s32ImgWidth = pstSrc->u32Width;   /*Get Img Width*/
        HI_U32 s32Bbox_X = astBbox[0].stRoiInfo.stRoi.s24q8X / 256 & (~1);
        HI_U32 s32Bbox_Y = astBbox[0].stRoiInfo.stRoi.s24q8Y / 256 & (~1);
        HI_U32 s32Bbox_W = astBbox[0].stRoiInfo.stRoi.u32Width & (~1);
        HI_U32 s32Bbox_H = astBbox[0].stRoiInfo.stRoi.u32Height & (~1);

        if ((s32Bbox_X + s32Bbox_W <= 0) || (s32Bbox_Y + s32Bbox_H <= 0) || (s32Bbox_X >= (s32ImgWidth - 1)) || (s32Bbox_Y >= (s32ImgHeight - 1)))
        {
            if (s32Bbox_X + s32Bbox_W <= 0)
                astBbox[0].stRoiInfo.stRoi.s24q8X = (-s32Bbox_W + 1) * 256;
            if (s32Bbox_Y + s32Bbox_H <= 0)
                astBbox[0].stRoiInfo.stRoi.s24q8Y = (-s32Bbox_H + 1) * 256;
            if (s32Bbox_X >= (s32ImgWidth - 1))
                astBbox[0].stRoiInfo.stRoi.s24q8X = (s32ImgWidth - 2) * 256;
            if (s32Bbox_Y >= (s32ImgHeight - 1))
                astBbox[0].stRoiInfo.stRoi.s24q8Y = (s32ImgHeight - 2) * 256;
            //astBbox[0].bRoiRefresh = HI_TRUE;
            astBbox[0].bTrackOk = HI_FALSE;
            *pbTracking_State = HI_FALSE;
        }
        


        /*Updating the tracking chain*/
        s32Ret = HI_MPI_IVE_KCF_ObjUpdate(&s_stIveKcfInfo.stObjList, astBbox, u32BboxNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_IVE_KCF_ObjUpdate failed!\n", s32Ret);
            return s32Ret;
        }
        #endif

        if (bTrackState)
        {
            *pbTracking_State = HI_TRUE;
            *pstTrackBox = astBbox->stRoiInfo.stRoi; ////////////////////////////////////////////////
        }
        else
        {
            *pbTracking_State = HI_FALSE;
            memset(pstTrackBox, 0, sizeof(IVE_RECT_S24Q8_S));

            printf("################################NO  OK!!!!!!!!!\n");
            printf("LastResPones:%d\n", s32LastResPones);
            printf("ResPones:    %d\n", astBbox[0].s32Response);
            printf("S32Rate:%f\n", S32Rate);
            printf("###  x:%d\n",pstTrackBox->s24q8X /256);
            printf("###  y:%d\n",pstTrackBox->s24q8Y /256);
            printf("###  w:%d\n",pstTrackBox->u32Width);
            printf("###  h:%d\n",pstTrackBox->u32Height);
        }
        s32LastResPones = astBbox[0].s32Response; /*reflash Response*/
    }
    else
    {
        bTrackState = HI_FALSE;
        *pbTracking_State = HI_FALSE;
        memset(pstTrackBox, 0, sizeof(IVE_RECT_S24Q8_S));


        printf("###  x:%d\n",pstTrackBox->s24q8X /256);
        printf("###  y:%d\n",pstTrackBox->s24q8Y /256);
        printf("###  w:%d\n",pstTrackBox->u32Width);
        printf("###  h:%d\n",pstTrackBox->u32Height);
    }

    return s32Ret;
}
