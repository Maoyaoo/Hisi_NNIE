#include "yolov3_core.h"



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
    pstSoftWareParam->u32BboxNumEachGrid = 3;
    pstSoftWareParam->u32ClassNum = 80;
    pstSoftWareParam->au32GridNumHeight[0] = 13;
    pstSoftWareParam->au32GridNumHeight[1] = 26;
    pstSoftWareParam->au32GridNumHeight[2] = 52;
    pstSoftWareParam->au32GridNumWidth[0] = 13;
    pstSoftWareParam->au32GridNumWidth[1] = 26;
    pstSoftWareParam->au32GridNumWidth[2] = 52;
    pstSoftWareParam->u32NmsThresh = (HI_U32)(0.3f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->u32ConfThresh = (HI_U32)(0.5f*SAMPLE_SVP_NNIE_QUANT_BASE);
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


/******************************************************************************
* function : roi to rect
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_RoiToRect_Yolov3(SVP_BLOB_S *pstDstScore,
    SVP_BLOB_S *pstDstRoi, SVP_BLOB_S *pstClassRoiNum, HI_FLOAT pf32GetResultThresh,
    HI_BOOL bPrint,WK_RECT_ARRAY_S *pstRect,
    HI_U32 u32SrcWidth, HI_U32 u32SrcHeight)
{
    /*print result, this sample has 81 classes:
        class 0:background      class 1:person       class 2:bicycle         class 3:car            class 4:motorbike      class 5:aeroplane
        class 6:bus             class 7:train        class 8:truck           class 9:boat           class 10:traffic light
        class 11:fire hydrant   class 12:stop sign   class 13:parking meter  class 14:bench         class 15:bird
        class 16:cat            class 17:dog         class 18:horse          class 19:sheep         class 20:cow
        class 21:elephant       class 22:bear        class 23:zebra          class 24:giraffe       class 25:backpack
        class 26:umbrella       class 27:handbag     class 28:tie            class 29:suitcase      class 30:frisbee
        class 31:skis           class 32:snowboard   class 33:sports ball    class 34:kite          class 35:baseball bat
        class 36:baseball glove class 37:skateboard  class 38:surfboard      class 39:tennis racket class 40bottle
        class 41:wine glass     class 42:cup         class 43:fork           class 44:knife         class 45:spoon
        class 46:bowl           class 47:banana      class 48:apple          class 49:sandwich      class 50orange
        class 51:broccoli       class 52:carrot      class 53:hot dog        class 54:pizza         class 55:donut
        class 56:cake           class 57:chair       class 58:sofa           class 59:pottedplant   class 60bed
        class 61:diningtable    class 62:toilet      class 63:vmonitor       class 64:laptop        class 65:mouse
        class 66:remote         class 67:keyboard    class 68:cell phone     class 69:microwave     class 70:oven
        class 71:toaster        class 72:sink        class 73:refrigerator   class 74:book          class 75:clock
        class 76:vase           class 77:scissors    class 78:teddy bear     class 79:hair drier    class 80:toothbrush*/
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
    //HI_S32 s32XMin = 0,s32YMin= 0,s32XMax = 0,s32YMax = 0;
    HI_FLOAT s32XMin = 0,s32YMin= 0,s32XMax = 0,s32YMax = 0;

 
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
            printf("==== The %dth class box info====\n", i);
            printf("######Total NUM: %d \n",ps32ClassRoiNum[i]);
            for (j = 0; j < (HI_U32)ps32ClassRoiNum[i]; j++)
            {
                /*Score is descend order*/
                f32Score = (HI_FLOAT)ps32Score[u32ScoreBias + j] / SAMPLE_SVP_NNIE_QUANT_BASE;
                if ((f32Score < pf32GetResultThresh) || (u32RoiNumTmp >= SAMPLE_SVP_NNIE_MAX_ROI_NUM_OF_CLASS))
                {
                    break;
                }
                #if 0
                s32XMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM];
                s32YMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 1];
                s32XMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 2];
                s32YMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 3];
                SAMPLE_SVP_TRACE_INFO("%d %d %d %d %f\n", s32XMin, s32YMin, s32XMax, s32YMax, f32Score);
                #endif
                /**/
                s32XMin = (HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM] / (HI_FLOAT)u32SrcWidth ;
                s32YMin = (HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 1] / (HI_FLOAT)u32SrcHeight;
                s32XMax = (HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 2] / (HI_FLOAT)u32SrcWidth;
                s32YMax = (HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 3] / (HI_FLOAT)u32SrcHeight;

                if (bPrint)  /*print result*/
                {
                    printf("s32XMin:%f \ns32YMin:%f \ns32XMax:%f \ns32YMax:%f \nf32Score:%f\n", s32XMin, s32YMin, s32XMax, s32YMax, f32Score);
                    //SAMPLE_SVP_TRACE_INFO("%f %f %f %f %f\n", s32XMin, s32YMin, s32XMax, s32YMax, f32Score);
                }
                pstRect->astRect[i][u32RoiNumTmp].ast_fPoint[0].s32X = s32XMin;
                pstRect->astRect[i][u32RoiNumTmp].ast_fPoint[0].s32Y = s32YMin;

                pstRect->astRect[i][u32RoiNumTmp].ast_fPoint[1].s32X = s32XMax;
                pstRect->astRect[i][u32RoiNumTmp].ast_fPoint[1].s32Y = s32YMin;

                pstRect->astRect[i][u32RoiNumTmp].ast_fPoint[2].s32X = s32XMax;
                pstRect->astRect[i][u32RoiNumTmp].ast_fPoint[2].s32Y = s32YMax;
                
                pstRect->astRect[i][u32RoiNumTmp].ast_fPoint[3].s32X = s32XMin;
                pstRect->astRect[i][u32RoiNumTmp].ast_fPoint[3].s32Y = s32YMax;
                
                pstRect->astRect[i][u32RoiNumTmp].f32Score = f32Score;

                u32RoiNumTmp++;
            }

        }

        pstRect->au32RoiNum[i] = u32RoiNumTmp;
        pstRect->u32TotalNum += u32RoiNumTmp;
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
         WK_RECT_ARRAY_S *pstRect)
{
    HI_U32 u32BaseWidth = 0;
    HI_U32 u32BaseHeight = 0;
    u32BaseWidth = pstExtFrmInfo->stVFrame.u32Width;
    u32BaseHeight = pstExtFrmInfo->stVFrame.u32Height;
    //printf("###u32W:%d\n",u32BaseWidth);
    //printf("###u32H:%d\n",u32BaseHeight);

    HI_S32 s32Ret = HI_FAILURE;
    HI_FLOAT f32GetResultThresh = 0.2f;
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;
    /*SP420*/
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
    
    /*Software process*/
    /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_Yolov3_GetResult
     function input datas are correct*/
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_GetResult(pstNnieParam,pstSoftWareParam);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),Error,SAMPLE_SVP_NNIE_Yolov3_GetResult failed!\n", s32Ret);
        return s32Ret;
    }

    #if 0
    SAMPLE_SVP_NNIE_Detection_Result_to_rect(&pstSoftWareParam->stDstScore,
												&pstSoftWareParam->stDstRoi,
												&pstSoftWareParam->stClassRoiNum,
												f32PrintResultThresh,
												pstRect);
    #endif


    #if 0
    s32Ret = yolov3_print_result(pstSoftWareParam);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),Error,yolov3_print_result failed!\n", s32Ret);
        return s32Ret;
    }
    #endif

    s32Ret = SAMPLE_SVP_NNIE_RoiToRect_Yolov3(&(pstSoftWareParam->stDstScore),
                                            &(pstSoftWareParam->stDstRoi), 
                                            &(pstSoftWareParam->stClassRoiNum),
                                            f32GetResultThresh,
                                            HI_TRUE,
                                            pstRect,
                                            u32BaseWidth,
                                            u32BaseHeight);

    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error(%#x),SAMPLE_SVP_NNIE_RoiToRect_Yolov3 failed!\n",s32Ret);

    return s32Ret;
}

#if 0
/*Get result*/
int yolov3_print_result(SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S *pstSoftWareParam)
{
    HI_S32 s32Ret;
    HI_FLOAT f32PrintResultThresh = 0.0f;
    f32PrintResultThresh = 0.8f;
    /*print result, this sample has 81 classes:
        class 0:background      class 1:person       class 2:bicycle         class 3:car            class 4:motorbike      class 5:aeroplane
        class 6:bus             class 7:train        class 8:truck           class 9:boat           class 10:traffic light
        class 11:fire hydrant   class 12:stop sign   class 13:parking meter  class 14:bench         class 15:bird
        class 16:cat            class 17:dog         class 18:horse          class 19:sheep         class 20:cow
        class 21:elephant       class 22:bear        class 23:zebra          class 24:giraffe       class 25:backpack
        class 26:umbrella       class 27:handbag     class 28:tie            class 29:suitcase      class 30:frisbee
        class 31:skis           class 32:snowboard   class 33:sports ball    class 34:kite          class 35:baseball bat
        class 36:baseball glove class 37:skateboard  class 38:surfboard      class 39:tennis racket class 40bottle
        class 41:wine glass     class 42:cup         class 43:fork           class 44:knife         class 45:spoon
        class 46:bowl           class 47:banana      class 48:apple          class 49:sandwich      class 50orange
        class 51:broccoli       class 52:carrot      class 53:hot dog        class 54:pizza         class 55:donut
        class 56:cake           class 57:chair       class 58:sofa           class 59:pottedplant   class 60bed
        class 61:diningtable    class 62:toilet      class 63:vmonitor       class 64:laptop        class 65:mouse
        class 66:remote         class 67:keyboard    class 68:cell phone     class 69:microwave     class 70:oven
        class 71:toaster        class 72:sink        class 73:refrigerator   class 74:book          class 75:clock
        class 76:vase           class 77:scissors    class 78:teddy bear     class 79:hair drier    class 80:toothbrush*/

    printf("=========== return Yolov3 result ===========\n");
    (void)SAMPLE_SVP_NNIE_Detection_PrintResult(&pstSoftWareParam->stDstScore,
        &pstSoftWareParam->stDstRoi, &pstSoftWareParam->stClassRoiNum,f32PrintResultThresh);

    return s32Ret;
}
#endif