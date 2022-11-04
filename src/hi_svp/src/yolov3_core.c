#include "yolov3_core.h"


char *cls_names[] = {"background", "person", "bicycle", "car", "motorbike", "aeroplane", "bus", "train", "truck",
                        "boat",

                        "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog",
                        "horse", "sheep",

                        "cow", "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie",
                        "suitcase",

                        "frisbee", "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove",
                        "skateboard", "surfboard", "tennis racket",

                        "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "donut", "apple", "sandwich",

                        "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "sofa",
                        "pottedplant",

                        "bed", "diningtable", "toilet", "vmonitor", "laptop", "mouse", "remote", "keyboard",
                        "cell phone", "microwave",

                        "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
                        "hair drier", "toothbrush"};



/******************************************************************************
* function : Yolov3 software para init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov3_SoftwareInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ClassNum = 0;
    HI_U32 u32TotalSize = 0;
    HI_U32 u32DstRoiSize = 0;
    HI_U32 u32DstScoreSize = 0;
    HI_U32 u32ClassRoiNumSize = 0;
    HI_U32 u32TmpBufTotalSize = 0;
    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;

    pstSoftWareParam->u32OriImHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height;
    pstSoftWareParam->u32OriImWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width;
    pstSoftWareParam->u32BboxNumEachGrid = 3;       //一个grid产生3个bbox
    pstSoftWareParam->u32ClassNum = 80;             //检测网路的识别类别，不同检测网络类别不一样
    //三种grid的划分方法，以检测小物体
    pstSoftWareParam->au32GridNumHeight[0] = 13;
    pstSoftWareParam->au32GridNumHeight[1] = 26;
    pstSoftWareParam->au32GridNumHeight[2] = 52;
    pstSoftWareParam->au32GridNumWidth[0] = 13;
    pstSoftWareParam->au32GridNumWidth[1] = 26;
    pstSoftWareParam->au32GridNumWidth[2] = 52;
    pstSoftWareParam->u32NmsThresh = (HI_U32)(0.3f*SAMPLE_SVP_NNIE_QUANT_BASE);   // NMS阈值
    pstSoftWareParam->u32ConfThresh = (HI_U32)(0.5f*SAMPLE_SVP_NNIE_QUANT_BASE);  //物体置信度阈值
    pstSoftWareParam->u32MaxRoiNum = 10;
    pstSoftWareParam->af32Bias[0][0] = 116;
    pstSoftWareParam->af32Bias[0][1] = 90;
    pstSoftWareParam->af32Bias[0][2] = 156;
    pstSoftWareParam->af32Bias[0][3] = 198;
    pstSoftWareParam->af32Bias[0][4] = 373;
    pstSoftWareParam->af32Bias[0][5] = 326;
    pstSoftWareParam->af32Bias[1][0] = 30;
    pstSoftWareParam->af32Bias[1][1] = 61;
    pstSoftWareParam->af32Bias[1][2] = 62;
    pstSoftWareParam->af32Bias[1][3] = 45;
    pstSoftWareParam->af32Bias[1][4] = 59;
    pstSoftWareParam->af32Bias[1][5] = 119;
    pstSoftWareParam->af32Bias[2][0] = 10;
    pstSoftWareParam->af32Bias[2][1] = 13;
    pstSoftWareParam->af32Bias[2][2] = 16;
    pstSoftWareParam->af32Bias[2][3] = 30;
    pstSoftWareParam->af32Bias[2][4] = 33;
    pstSoftWareParam->af32Bias[2][5] = 23;

    /*Malloc assist buffer memory*/
    u32ClassNum = pstSoftWareParam->u32ClassNum+1;

    SAMPLE_SVP_CHECK_EXPR_RET(SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM != pstNnieParam->pstModel->astSeg[0].u16DstNum,
        HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,pstNnieParam->pstModel->astSeg[0].u16DstNum(%d) should be %d!\n",
        pstNnieParam->pstModel->astSeg[0].u16DstNum,SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM);
    u32TmpBufTotalSize = SAMPLE_SVP_NNIE_Yolov3_GetResultTmpBuf(pstNnieParam,pstSoftWareParam);
    u32DstRoiSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    u32DstScoreSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32));
    u32ClassRoiNumSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    u32TotalSize = u32TotalSize+u32DstRoiSize+u32DstScoreSize+u32ClassRoiNumSize+u32TmpBufTotalSize;
    s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_YOLOV3_INIT",NULL,(HI_U64*)&u64PhyAddr,
        (void**)&pu8VirAddr,u32TotalSize);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,Malloc memory failed!\n");
    memset(pu8VirAddr,0, u32TotalSize);
    SAMPLE_COMM_SVP_FlushCache(u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);

   /*set each tmp buffer addr*/
    pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = u64PhyAddr;
    pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = (HI_U64)((HI_UL)pu8VirAddr);

    /*set result blob*/
    pstSoftWareParam->stDstRoi.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstRoi.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize;
    pstSoftWareParam->stDstRoi.u64VirAddr = (HI_U64)((HI_UL)pu8VirAddr+u32TmpBufTotalSize);
    pstSoftWareParam->stDstRoi.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
        pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    pstSoftWareParam->stDstRoi.u32Num = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Width = u32ClassNum*
        pstSoftWareParam->u32MaxRoiNum*SAMPLE_SVP_NNIE_COORDI_NUM;

    pstSoftWareParam->stDstScore.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstScore.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+u32DstRoiSize;
    pstSoftWareParam->stDstScore.u64VirAddr = (HI_U64)((HI_UL)pu8VirAddr+u32TmpBufTotalSize+u32DstRoiSize);
    pstSoftWareParam->stDstScore.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
        pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32));
    pstSoftWareParam->stDstScore.u32Num = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Width = u32ClassNum*pstSoftWareParam->u32MaxRoiNum;

    pstSoftWareParam->stClassRoiNum.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stClassRoiNum.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+
        u32DstRoiSize+u32DstScoreSize;
    pstSoftWareParam->stClassRoiNum.u64VirAddr = (HI_U64)((HI_UL)pu8VirAddr+u32TmpBufTotalSize+
        u32DstRoiSize+u32DstScoreSize);
    pstSoftWareParam->stClassRoiNum.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    pstSoftWareParam->stClassRoiNum.u32Num = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Width = u32ClassNum;

    return s32Ret;
}


/******************************************************************************
* function : Yolov3 software deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit(SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_CHECK_EXPR_RET(NULL== pstSoftWareParam,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error, pstSoftWareParam can't be NULL!\n");
    if(0!=pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr && 0!=pstSoftWareParam->stGetResultTmpBuf.u64VirAddr)
    {
        SAMPLE_SVP_MMZ_FREE(pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr,
            pstSoftWareParam->stGetResultTmpBuf.u64VirAddr);
        pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = 0;
        pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = 0;
        pstSoftWareParam->stDstRoi.u64PhyAddr = 0;
        pstSoftWareParam->stDstRoi.u64VirAddr = 0;
        pstSoftWareParam->stDstScore.u64PhyAddr = 0;
        pstSoftWareParam->stDstScore.u64VirAddr = 0;
        pstSoftWareParam->stClassRoiNum.u64PhyAddr = 0;
        pstSoftWareParam->stClassRoiNum.u64VirAddr = 0;
    }
    return s32Ret;
}


/******************************************************************************
* function : Yolov3 Deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov3_Deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam,SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*hardware deinit*/
    if(pstNnieParam!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(pstNnieParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error,SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
    }
    /*software deinit*/
    if(pstSoftWareParam!=NULL)
    {
        s32Ret = SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit(pstSoftWareParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error,SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit failed!\n");
    }
    /*model deinit*/
    if(pstNnieModel!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_UnloadModel(pstNnieModel);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error,SAMPLE_COMM_SVP_NNIE_UnloadModel failed!\n");
    }
    return s32Ret;
}



/******************************************************************************
* function : Yolov3 init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov3_ParamInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_COMM_SVP_NNIE_ParamInit(pstCfg,pstNnieParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SAMPLE_COMM_SVP_NNIE_ParamInit failed!\n",s32Ret);

    /*init software para*/
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_SoftwareInit(pstCfg,pstNnieParam,
        pstSoftWareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SAMPLE_SVP_NNIE_Yolov3_SoftwareInit failed!\n",s32Ret);

    return s32Ret;
INIT_FAIL_0:
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_Deinit(pstNnieParam,pstSoftWareParam,NULL);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),SAMPLE_SVP_NNIE_Yolov3_Deinit failed!\n",s32Ret);
    return HI_FAILURE;
}




/******************************************************************************
* function : NNIE Forward
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Forward(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx,
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S* pstProcSegIdx,HI_BOOL bInstant)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0, j = 0;
    HI_BOOL bFinish = HI_FALSE;
    SVP_NNIE_HANDLE hSvpNnieHandle = 0;
    HI_U32 u32TotalStepNum = 0;
    SAMPLE_SVP_NIE_PERF_STAT_DEF_VAR()

    SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_CLREAR()

    SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64PhyAddr,
        SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_VOID,pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64VirAddr),
        pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u32Size);

    SAMPLE_SVP_NNIE_PERF_STAT_BEGIN()
    for(i = 0; i < pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32DstNum; i++)
    {
        if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].enType)
        {
            for(j = 0; j < pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num; j++)
            {
                u32TotalStepNum += *(SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_U32,pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stSeq.u64VirAddrStep)+j);
            }
            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_VOID,pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr),
                u32TotalStepNum*pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);

        }
        else
        {
            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_VOID,pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr),
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Chn*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Height*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);
        }
    }
    SAMPLE_SVP_NNIE_PERF_STAT_END()
    SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_PRE_DST_FLUSH_TIME()

    /*set input blob according to node name*/
    if(pstInputDataIdx->u32SegIdx != pstProcSegIdx->u32SegIdx)
    {
        for(i = 0; i < pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].u16SrcNum; i++)
        {
            for(j = 0; j < pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum; j++)
            {
                if(0 == strncmp(pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].astDstNode[j].szName,
                    pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].astSrcNode[i].szName,
                    SVP_NNIE_NODE_NAME_LEN))
                {
                    pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc[i] =
                        pstNnieParam->astSegData[pstInputDataIdx->u32SegIdx].astDst[j];
                    break;
                }
            }
            SAMPLE_SVP_CHECK_EXPR_RET((j == pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum),
                HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,can't find %d-th seg's %d-th src blob!\n",
                pstProcSegIdx->u32SegIdx,i);
        }
    }

    /*NNIE_Forward*/
    SAMPLE_SVP_NNIE_PERF_STAT_BEGIN()
    s32Ret = HI_MPI_SVP_NNIE_Forward(&hSvpNnieHandle,
        pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc,
        pstNnieParam->pstModel, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst,
        &pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx], bInstant);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,HI_MPI_SVP_NNIE_Forward failed!\n");

    if(bInstant)
    {
        /*Wait NNIE finish*/
        while(HI_ERR_SVP_NNIE_QUERY_TIMEOUT == (s32Ret = HI_MPI_SVP_NNIE_Query(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].enNnieId,
            hSvpNnieHandle, &bFinish, HI_TRUE)))
        {
            usleep(100);
            SAMPLE_SVP_TRACE(SAMPLE_SVP_ERR_LEVEL_INFO,
                "HI_MPI_SVP_NNIE_Query Query timeout!\n");
        }
    }
    SAMPLE_SVP_NNIE_PERF_STAT_END()
    SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_OP_TIME()
    u32TotalStepNum = 0;

    SAMPLE_SVP_NNIE_PERF_STAT_BEGIN()
    for(i = 0; i < pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32DstNum; i++)
    {
        if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].enType)
        {
            for(j = 0; j < pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num; j++)
            {
                u32TotalStepNum += *(SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_U32,pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stSeq.u64VirAddrStep)+j);
            }
            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_VOID,pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr),
                u32TotalStepNum*pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);

        }
        else
        {
            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_VOID,pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr),
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Chn*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Height*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);
        }
    }
    SAMPLE_SVP_NNIE_PERF_STAT_END()
    SAMPLE_SVP_NNIE_PERF_STAT_OP_FORWARD_AFTER_DST_FLUSH_TIME()

    return s32Ret;
}



HI_S32 SAMPLE_SVP_NNIE_RoiToRect_Yolov3_Test(SVP_BLOB_S *pstDstScore,
    SVP_BLOB_S *pstDstRoi, SVP_BLOB_S *pstClassRoiNum, HI_FLOAT pf32GetResultThresh,
    HI_BOOL bPrint,WK_YOLO_RECT_ARRAY_S *pstRect,
    HI_U32 u32SrcWidth, HI_U32 u32SrcHeight)
{
        HI_U32 i = 0, j = 0;
    HI_U32 u32RoiNumBias = 0;
    HI_U32 u32ScoreBias = 0;
    HI_U32 u32BboxBias = 0;
    HI_FLOAT f32Score = 0.0f;
    HI_S32* ps32Score = SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_S32,pstDstScore->u64VirAddr);
    HI_S32* ps32Roi = SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_S32,pstDstRoi->u64VirAddr);
    HI_S32* ps32ClassRoiNum = SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_S32,pstClassRoiNum->u64VirAddr);
    HI_U32 u32ClassNum = pstClassRoiNum->unShape.stWhc.u32Width;
    HI_U32 u32RoiNumTmp = 0;
    HI_S32 s32XMin = 0,s32YMin= 0,s32XMax = 0,s32YMax = 0;
    // HI_FLOAT s32XMin = 0,s32YMin= 0,s32XMax = 0,s32YMax = 0;

    SAMPLE_SVP_CHECK_EXPR_RET(u32ClassNum > WK_YOLOV3_MAX_CLASS_NUM ,HI_ERR_SVP_NNIE_ILLEGAL_PARAM,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),u32ClassNum(%u) must be less than or equal %u to!\n",HI_ERR_SVP_NNIE_ILLEGAL_PARAM,u32ClassNum, WK_YOLOV3_MAX_CLASS_NUM);
 
    pstRect->u32TotalNum = 0;            /*init tolalNum*/
    pstRect->u32ClsNum = u32ClassNum;    /*get class Num*/
    pstRect->au32RoiNum[0] = 0;          /*init ROINum for etch class*/
    u32RoiNumBias += ps32ClassRoiNum[0];

    for (i = 1; i < u32ClassNum; i++)
    {
        u32ScoreBias = u32RoiNumBias;
        u32BboxBias = u32RoiNumBias * SAMPLE_SVP_NNIE_COORDI_NUM;
        u32RoiNumTmp = 0;
        /*if the confidence score greater than result thresh, the result will be drawed*/
        if(((HI_FLOAT)ps32Score[u32ScoreBias] / SAMPLE_SVP_NNIE_QUANT_BASE >=
            pf32GetResultThresh)  &&  (ps32ClassRoiNum[i] != 0))
        { 
            printf("==== The %d th class box info====\n", i);
            printf("######Total NUM: %d \n",ps32ClassRoiNum[i]);
            for (j = 0; j < (HI_U32)ps32ClassRoiNum[i]; j++)
            {
                /*Score is descend order*/
                f32Score = (HI_FLOAT)ps32Score[u32ScoreBias + j] / SAMPLE_SVP_NNIE_QUANT_BASE;
                if ((f32Score < pf32GetResultThresh) || (u32RoiNumTmp >= SAMPLE_SVP_NNIE_MAX_ROI_NUM_OF_CLASS))
                {
                    break;
                }
           
                s32XMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM];
                s32YMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 1];
                s32XMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 2];
                s32YMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 3];
             
                // s32XMin = (HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM] / (HI_FLOAT)u32SrcWidth ;
                // s32YMin = (HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 1] / (HI_FLOAT)u32SrcHeight;
                // s32XMax = (HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 2] / (HI_FLOAT)u32SrcWidth;
                // s32YMax = (HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 3] / (HI_FLOAT)u32SrcHeight;

                pstRect->astRect[i][u32RoiNumTmp].stRect.stRect_f.fX = (HI_FLOAT)s32XMin / (HI_FLOAT)u32SrcWidth;
                pstRect->astRect[i][u32RoiNumTmp].stRect.stRect_f.fY = (HI_FLOAT)s32YMin / (HI_FLOAT)u32SrcHeight;
                pstRect->astRect[i][u32RoiNumTmp].stRect.stRect_f.fWidth = (HI_FLOAT)(s32XMax - s32XMin) / (HI_FLOAT)u32SrcWidth;
                pstRect->astRect[i][u32RoiNumTmp].stRect.stRect_f.fHeight = (HI_FLOAT)(s32YMax - s32YMin) / (HI_FLOAT)u32SrcHeight;
                pstRect->astRect[i][u32RoiNumTmp].f32Score = f32Score;
                

                /*print result*/
                if (bPrint)  
                { 
                    printf("====================\n");
                    /*打印正常坐标Xmin，Ymin，XMax，YMax*/
                    printf("s32XMin:%d \ns32YMin:%d \ns32XMax:%d \ns32YMax:%d \nf32Score:%f\n", s32XMin, s32YMin, s32XMax, s32YMax, f32Score);
                    
                    /*打印归一化X，Y，W，H*/
                    // printf("fx:%f\n",pstRect->astRect[i][u32RoiNumTmp].stRect.stRect_f.fX);
                    // printf("fy:%f\n",pstRect->astRect[i][u32RoiNumTmp].stRect.stRect_f.fY);
                    // printf("fw:%f\n",pstRect->astRect[i][u32RoiNumTmp].stRect.stRect_f.fWidth);
                    // printf("fh:%f\n",pstRect->astRect[i][u32RoiNumTmp].stRect.stRect_f.fHeight);
                    printf("====================\n");
                    //SAMPLE_SVP_TRACE_INFO("%f %f %f %f %f\n", s32XMin, s32YMin, s32XMax, s32YMax, f32Score);
                }
                u32RoiNumTmp++;

            }

        }
        pstRect->au32RoiNum[i] = u32RoiNumTmp;
        pstRect->u32TotalNum += u32RoiNumTmp;
        u32RoiNumBias += ps32ClassRoiNum[i];
    }
    
    return HI_SUCCESS;


}




/******************************************************************************
* function : roi to rect
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_RoiToRect_Yolov3(SVP_BLOB_S *pstDstScore,
    SVP_BLOB_S *pstDstRoi, SVP_BLOB_S *pstClassRoiNum, HI_FLOAT pf32GetResultThresh,
    HI_BOOL bPrint,WK_YOLO_RECT_ARRAY_S *pstRect,
    HI_U32 u32SrcWidth, HI_U32 u32SrcHeight)
{
    HI_U32 i = 0, j = 0;
    HI_U32 u32RoiNumBias = 0;
    HI_U32 u32ScoreBias = 0;
    HI_U32 u32BboxBias = 0;
    HI_FLOAT f32Score = 0.0f;
    HI_S32* ps32Score = SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_S32,pstDstScore->u64VirAddr);
    HI_S32* ps32Roi = SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_S32,pstDstRoi->u64VirAddr);
    HI_S32* ps32ClassRoiNum = SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_S32,pstClassRoiNum->u64VirAddr);
    HI_U32 u32ClassNum = pstClassRoiNum->unShape.stWhc.u32Width;
    HI_S32 s32XMin = 0,s32YMin= 0,s32XMax = 0,s32YMax = 0;
    // HI_FLOAT s32XMin = 0,s32YMin= 0,s32XMax = 0,s32YMax = 0;

    // SAMPLE_SVP_CHECK_EXPR_RET(u32ClassNum > WK_YOLOV3_MAX_CLASS_NUM ,HI_ERR_SVP_NNIE_ILLEGAL_PARAM,SAMPLE_SVP_ERR_LEVEL_ERROR,
    //     "Error(%#x),u32ClassNum(%u) must be less than or equal %u to!\n",HI_ERR_SVP_NNIE_ILLEGAL_PARAM,u32ClassNum, WK_YOLOV3_MAX_CLASS_NUM);
 
    memset(pstRect,0x00,sizeof(WK_YOLO_RECT_ARRAY_S));  //初始化结果返回变量
    // pstRect->u32TotalNum = 0;            /*init tolalNum*/
    pstRect->u32ClsNum = u32ClassNum;    /*get class Num*/
    // pstRect->au32RoiNum[0] = 0;          /*init ROINum for etch class*/
    u32RoiNumBias += ps32ClassRoiNum[0];

    for (i = 1; i < u32ClassNum; i++)
    {
        u32ScoreBias = u32RoiNumBias;
        u32BboxBias = u32RoiNumBias * SAMPLE_SVP_NNIE_COORDI_NUM;

        /*if the confidence score greater than result thresh, the result will be drawed*/
        if(((HI_FLOAT)ps32Score[u32ScoreBias] / SAMPLE_SVP_NNIE_QUANT_BASE >= pf32GetResultThresh)  &&  (ps32ClassRoiNum[i] != 0))
        { 
            printf("==== Class is:  %s ====\n", cls_names[i]);
            printf("==== Total NUM: %d ====\n",ps32ClassRoiNum[i]);
            printf("\n\n");
        }
        for (j = 0; j < (HI_U32)ps32ClassRoiNum[i]; j++)
        {
            /*Score is descend order*/
            f32Score = (HI_FLOAT)ps32Score[u32ScoreBias + j] / SAMPLE_SVP_NNIE_QUANT_BASE;
            if ((f32Score < pf32GetResultThresh))
            {
                break;
            }
        
            s32XMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM];
            s32YMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 1];
            s32XMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 2];
            s32YMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 3];
            
            // s32XMin = (HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM] / (HI_FLOAT)u32SrcWidth ;
            // s32YMin = (HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 1] / (HI_FLOAT)u32SrcHeight;
            // s32XMax = (HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 2] / (HI_FLOAT)u32SrcWidth;
            // s32YMax = (HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 3] / (HI_FLOAT)u32SrcHeight;

            pstRect->astRect[i][j].stRect.stRect_f.fX = (HI_FLOAT)s32XMin / (HI_FLOAT)u32SrcWidth;
            pstRect->astRect[i][j].stRect.stRect_f.fY = (HI_FLOAT)s32YMin / (HI_FLOAT)u32SrcHeight;
            pstRect->astRect[i][j].stRect.stRect_f.fWidth = (HI_FLOAT)(s32XMax - s32XMin) / (HI_FLOAT)u32SrcWidth;
            pstRect->astRect[i][j].stRect.stRect_f.fHeight = (HI_FLOAT)(s32YMax - s32YMin) / (HI_FLOAT)u32SrcHeight;
            pstRect->astRect[i][j].f32Score = f32Score;    

            /*print result*/
            if (bPrint)  
            { 
                printf("====================\n");
                /*打印正常坐标Xmin，Ymin，XMax，YMax*/
                printf("s32XMin:%d \ns32YMin:%d \ns32XMax:%d \ns32YMax:%d \nf32Score:%f\n", s32XMin, s32YMin, s32XMax, s32YMax, f32Score);
                
                /*打印归一化X，Y，W，H*/
                // printf("fx:%f\n",pstRect->astRect[i][u32RoiNumTmp].stRect.stRect_f.fX);
                // printf("fy:%f\n",pstRect->astRect[i][u32RoiNumTmp].stRect.stRect_f.fY);
                // printf("fw:%f\n",pstRect->astRect[i][u32RoiNumTmp].stRect.stRect_f.fWidth);
                // printf("fh:%f\n",pstRect->astRect[i][u32RoiNumTmp].stRect.stRect_f.fHeight);
                printf("====================\n");
                //SAMPLE_SVP_TRACE_INFO("%f %f %f %f %f\n", s32XMin, s32YMin, s32XMax, s32YMax, f32Score);
            }
        }
        pstRect->au32RoiNum[i] = (HI_U32)ps32ClassRoiNum[i];
        pstRect->u32TotalNum += (HI_U32)ps32ClassRoiNum[i];
        u32RoiNumBias += ps32ClassRoiNum[i];
    }
    return HI_SUCCESS;
}


int yolov3_param_deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam,SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_Deinit(pstNnieParam,pstSoftWareParam,pstNnieModel);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SAMPLE_SVP_NNIE_Yolov3_Deinit failed!\n",s32Ret);

    return s32Ret;
}




int _yolov3_load_model(const char *model_path, SAMPLE_SVP_NNIE_MODEL_S *s_stModel)
{
    /*Yolov3 Load model*/
    printf("==========Yolov3 Load model==========\n");
    HI_S32 s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(model_path, s_stModel);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_SVP_TRACE_INFO("Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");
        return -1;
    }
    return 0;
}


/*Yolov3 parameter initialization*/
int yolov3_param_init(SAMPLE_SVP_NNIE_MODEL_S *pstModel, SAMPLE_SVP_NNIE_CFG_S *pstCfg,
                     SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S *pstSoftwareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /*Yolov3 param init*/
    pstCfg->pszPic= NULL;
    pstCfg->u32MaxInputNum = 1; //max input image num in each batch
    pstCfg->u32MaxRoiNum = 0;
    pstCfg->aenNnieCoreId[0] = SVP_NNIE_ID_0;//set NNIE core

    //初始化软硬件参数，硬件参数基本不变，软件参数需要修改
    /*Yolov3 parameter initialization*/
    /*Yolov3 software parameters are set in SAMPLE_SVP_NNIE_Yolov3_SoftwareInit,
      if user has changed net struct, please make sure the parameter settings in
      SAMPLE_SVP_NNIE_Yolov3_SoftwareInit function are correct*/
    
    printf("==========Yolov3 parameter initialization==========\n");
    pstNnieParam->pstModel = &pstModel->stModel;
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_ParamInit(pstCfg,pstNnieParam,pstSoftwareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV3_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,SAMPLE_SVP_NNIE_Yolov3_ParamInit failed!\n");

    return s32Ret;

    YOLOV3_FAIL_0:
        s32Ret = SAMPLE_SVP_NNIE_Yolov3_Deinit(pstNnieParam,pstSoftwareParam,pstModel);
        SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),SAMPLE_SVP_NNIE_Yolov3_Deinit failed!\n",s32Ret);
        SAMPLE_COMM_SVP_CheckSysExit();
        return s32Ret;
}


/*Yolov3 inference*/
int yolov3_inference(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S *pstSoftWareParam, VIDEO_FRAME_INFO_S *pstExtFrmInfo,
         WK_YOLO_RECT_ARRAY_S *pstRect)
{
    /*****************获取推理图像大小***********************/
    HI_U32 u32BaseWidth = 0;
    HI_U32 u32BaseHeight = 0;
    u32BaseWidth = pstExtFrmInfo->stVFrame.u32Width;
    u32BaseHeight = pstExtFrmInfo->stVFrame.u32Height;
    // printf("###u32W:%d\n",u32BaseWidth);
    // printf("###u32H:%d\n",u32BaseHeight);
    /*****************************************************/

    HI_S32 s32Ret = HI_FAILURE;
    HI_FLOAT f32GetResultThresh = 0.2f;
    // HI_FLOAT f32GetResultThresh = 0.8f;
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};   //数据填充网络索引
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};    //推理过程中因为不支持层索引

    //Fill SRC data
    //初始输入图片的节点，０段的第０层
    stInputDataIdx.u32SegIdx = 0;   //段索引
    stInputDataIdx.u32NodeIdx = 0;  //层节点索引
    /*SP420*/
    //储存在pstExtFrmInfo里的vpss图片帧，放入硬件配置的输入地址里
    pstNnieParam->astSegData[stInputDataIdx.u32SegIdx].astSrc[stInputDataIdx.u32NodeIdx].u64VirAddr = pstExtFrmInfo->stVFrame.u64VirAddr[0];
    pstNnieParam->astSegData[stInputDataIdx.u32SegIdx].astSrc[stInputDataIdx.u32NodeIdx].u64PhyAddr = pstExtFrmInfo->stVFrame.u64PhyAddr[0];
    pstNnieParam->astSegData[stInputDataIdx.u32SegIdx].astSrc[stInputDataIdx.u32NodeIdx].u32Stride  = pstExtFrmInfo->stVFrame.u32Stride[0];

    /*NNIE process(process the 0-th segment)*/
    stProcSegIdx.u32SegIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_Forward(pstNnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),Error,SAMPLE_SVP_NNIE_Forward failed!\n", s32Ret);
        return s32Ret;
    }
    
    /*Software process*///不支持层处理
    /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_Yolov3_GetResult
     function input datas are correct*/
    //这星把硬件配置pstNnieParam输出的结果，通过自定义的NMS层计算出ROI信息，并将其控取到软件配置中pstSoftWareParam
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_GetResult(pstNnieParam,pstSoftWareParam);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),Error,SAMPLE_SVP_NNIE_Yolov3_GetResult failed!\n", s32Ret);
        return s32Ret;
    }

    #if 0
    s32Ret = SAMPLE_SVP_NNIE_RoiToRect_Yolov3_Test(&(pstSoftWareParam->stDstScore),
                                            &(pstSoftWareParam->stDstRoi), 
                                            &(pstSoftWareParam->stClassRoiNum),
                                            f32GetResultThresh,
                                            HI_TRUE,
                                            pstRect,
                                            u32BaseWidth,
                                            u32BaseHeight);

    #endif

    #if 1
    s32Ret = SAMPLE_SVP_NNIE_RoiToRect_Yolov3(&(pstSoftWareParam->stDstScore),
                                            &(pstSoftWareParam->stDstRoi), 
                                            &(pstSoftWareParam->stClassRoiNum),
                                            f32GetResultThresh,
                                            HI_TRUE,
                                            pstRect,
                                            u32BaseWidth,
                                            u32BaseHeight);
    #endif
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error(%#x),SAMPLE_SVP_NNIE_RoiToRect_Yolov3 failed!\n",s32Ret);

    return s32Ret;
}
