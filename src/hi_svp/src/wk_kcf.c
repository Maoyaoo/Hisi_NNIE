#include "kcf_core.h"
#include "wk_utils.h"
#include "wk_kcf.h"

static void SAMPLE_IVE_FillImage(VIDEO_FRAME_INFO_S *pstFrameInfo, IVE_IMAGE_S *pstImage) //(定义视频图像帧信息结构体,定义二维广义图像信息)
{
    pstImage->au64PhyAddr[0] = pstFrameInfo->stVFrame.u64PhyAddr[0]; //图像数据物理地址
    pstImage->au64PhyAddr[1] = pstFrameInfo->stVFrame.u64PhyAddr[1];
    pstImage->au32Stride[0] = pstFrameInfo->stVFrame.u32Stride[0]; //图像数据跨距
    pstImage->au32Stride[1] = pstFrameInfo->stVFrame.u32Stride[1];
    pstImage->u32Width = pstFrameInfo->stVFrame.u32Width;   //图像的宽
    pstImage->u32Height = pstFrameInfo->stVFrame.u32Height; //图像的高

    if (pstFrameInfo->stVFrame.enPixelFormat == PIXEL_FORMAT_YVU_SEMIPLANAR_420)
    {
        pstImage->enType = IVE_IMAGE_TYPE_YUV420SP;
    }
    else
    {
    }

    return;
}

HI_S32 wk_kcf_prm_init(void)
{
    HI_S32 s32Ret = HI_SUCCESS;
    printf("============ KCF INIT!!! ============\n");
    s32Ret = kcf_prm_init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),kcf_init failed!\n", s32Ret);
        return s32Ret;
    }
    return s32Ret;
}

HI_S32 wk_kcf_prm_deinit()
{
    HI_S32 s32Ret = HI_SUCCESS;
    printf("============ KCF DEINIT!!! ============\n");
    s32Ret = kcf_prm_deinit();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),kcf_init failed!\n", s32Ret);
        return s32Ret;
    }
    return s32Ret;
}

int printNum;

HI_S32 wk_kcf_proc(WK_KCF_INPUT_PRM *pstKCF_Prm)
{
    IVE_IMAGE_S stSrc;
    IVE_RECT_S24Q8_S stSlectBboxInfo = {0};
    IVE_RECT_S24Q8_S stTrackBox = {0};
    //HI_BOOL bTracking_State;
    SAMPLE_IVE_FillImage(&pstKCF_Prm->stKcfFrmInfo, &stSrc);
    NORMALIZATION_TO_S24Q8(&pstKCF_Prm->stInputBboxInfo, &stSlectBboxInfo, stSrc.u32Width, stSrc.u32Height);

    HI_S32 s32Ret = HI_SUCCESS;
    if (pstKCF_Prm->bFlashSelectBbox)
    {
        s32Ret = kcf_object_init(&stSlectBboxInfo, &stSrc);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),kcf_object_init failed!\n", s32Ret);
            return s32Ret;
        }
        pstKCF_Prm->stKcfRes.stResBbox = pstKCF_Prm->stInputBboxInfo;
        pstKCF_Prm->stKcfRes.bTrackState = HI_TRUE;
        pstKCF_Prm->bFlashSelectBbox = HI_FALSE;
    }
    else
    {
        s32Ret = kcf_track_update(&stSrc, &stTrackBox, &pstKCF_Prm->stKcfRes.bTrackState);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),kcf_track_update failed!\n", s32Ret);
            return s32Ret;
        }
        S24Q8_TO_NORMALIZATION(&pstKCF_Prm->stKcfRes.stResBbox, &stTrackBox, stSrc.u32Width, stSrc.u32Height);
    }
    
    printNum++;
    if (printNum % 1000 == 0)
    {
        HI_U32 s32Bbox_X = stTrackBox.s24q8X / 256 & (~1);
        HI_U32 s32Bbox_Y = stTrackBox.s24q8Y / 256 & (~1);
        HI_U32 s32Bbox_W = stTrackBox.u32Width & (~1);
        HI_U32 s32Bbox_H = stTrackBox.u32Height & (~1);  
        printf("=========KCF RES==========\n");
        // printf("fX:%f\n",pstKCF_Prm->stKcfRes.stResBbox.fX);
        // printf("fY:%f\n",pstKCF_Prm->stKcfRes.stResBbox.fY);
        // printf("fW:%f\n",pstKCF_Prm->stKcfRes.stResBbox.fWidth);
        // printf("fH:%f\n",pstKCF_Prm->stKcfRes.stResBbox.fHeight);

        // printf("+++++++++nor+++++++++\n");
        printf("s32Bbox_X = %d \n", s32Bbox_X);
        printf("s32Bbox_Y = %d \n", s32Bbox_Y);
        printf("s32Bbox_W = %d \n", s32Bbox_W);
        printf("s32Bbox_H = %d \n", s32Bbox_H);
        printf("============================\n");
        printf("       \n");
    }
    return s32Ret;
}
