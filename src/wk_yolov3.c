#include "yolov3_core.h"
#include "wk_yolov3.h"


/*yolov3 para*/
static SAMPLE_SVP_NNIE_MODEL_S s_stYolov3Model = {0};
static SAMPLE_SVP_NNIE_PARAM_S s_stYolov3NnieParam = {0};
static SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S s_stYolov3SoftwareParam = {0};


SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};






int yolov3_load_model(const char *model_path)
{
    HI_S32 s32Ret = HI_SUCCESS;
    memset(&s_stYolov3Model,0,sizeof(s_stYolov3Model));
    memset(&s_stYolov3NnieParam,0,sizeof(s_stYolov3NnieParam));
    memset(&s_stYolov3SoftwareParam,0,sizeof(s_stYolov3SoftwareParam));

    /*Yolov3 Load model*/
    s32Ret = _yolov3_load_model(model_path, &s_stYolov3Model);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),_yolov3_load_model failed!\n", s32Ret);
        return s32Ret;
    }

     /*Init Param*/
    s32Ret = yolov3_param_init(&s_stYolov3Model, &stNnieCfg, &s_stYolov3NnieParam, &s_stYolov3SoftwareParam);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),yolov3_param_init failed!\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}


int yolov3_run(VIDEO_FRAME_INFO_S *pstExtFrmInfo,WK_RECT_ARRAY_S *pstRect)
{
    HI_S32 s32Ret = HI_SUCCESS;
 

    s32Ret = yolov3_inference(&s_stYolov3NnieParam,&s_stYolov3SoftwareParam,pstExtFrmInfo,pstRect);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV3_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,yolov3_inference failed!\n");

    // s32Ret = yolov3_return_result(&s_stYolov3SoftwareParam);
    // SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV3_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
    //     "Error,yolov3_return_result failed!\n");

    return s32Ret;
    YOLOV3_FAIL_0:
        yolov3_clear();
        return s32Ret;
}


int yolov3_clear(void)
{
    HI_S32 s32Ret = HI_SUCCESS;    
    s32Ret = yolov3_param_deinit(&s_stYolov3NnieParam,&s_stYolov3SoftwareParam,&s_stYolov3Model);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),yolov3_param_init failed!\n", s32Ret);
        return s32Ret;
    }
    return s32Ret;
}


void get_frame_bdbox(WK_RECT_ARRAY_S *pstRect, HI_U32 s32width, HI_U32 s32heigth, HI_BOOL bfilter)
{
    int i,j;
   
    for (i = 0; i <= pstRect->u32ClsNum; i++)
    {
        if(bfilter)
        {
            if (i != 1 )
            {
                continue;
            }
        }
        
        for (j = 0; j <= pstRect->au32RoiNum[i]; j++)
        {
            pstRect->astRect[i][j].ast_uPoint[0].s32X = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[0].s32X * (HI_FLOAT)s32width) & (~1);
            pstRect->astRect[i][j].ast_uPoint[0].s32Y = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[0].s32Y * (HI_FLOAT)s32heigth) & (~1);

            pstRect->astRect[i][j].ast_uPoint[1].s32X = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[1].s32X * (HI_FLOAT)s32width) & (~1);
            pstRect->astRect[i][j].ast_uPoint[1].s32Y = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[1].s32Y * (HI_FLOAT)s32heigth) & (~1);

            pstRect->astRect[i][j].ast_uPoint[2].s32X = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[2].s32X * (HI_FLOAT)s32width) & (~1);
            pstRect->astRect[i][j].ast_uPoint[2].s32Y = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[2].s32Y * (HI_FLOAT)s32heigth) & (~1);

            pstRect->astRect[i][j].ast_uPoint[3].s32X = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[3].s32X * (HI_FLOAT)s32width) & (~1);
            pstRect->astRect[i][j].ast_uPoint[3].s32Y = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[3].s32Y * (HI_FLOAT)s32heigth) & (~1);
            
            //printf("6666666666\n");
            //printf("####s32XMin:%d \ns32YMin:%d \ns32XMax:%d \ns32YMax:%d \n", pstRect->astRect[i][j].ast_uPoint[0].s32X, pstRect->astRect[i][j].ast_uPoint[0].s32Y, pstRect->astRect[i][j].ast_uPoint[3].s32X, pstRect->astRect[i][j].ast_uPoint[3].s32Y);
           
        }
        
    }
    return;
}

HI_S32 SAMPLE_COMM_SVP_NNIE_YOLOV3_FillRect(VIDEO_FRAME_INFO_S *pstFrmInfo, WK_RECT_ARRAY_S* pstRect, HI_U32 u32Color)
{
    VGS_HANDLE VgsHandle = -1;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i,j;
    VGS_TASK_ATTR_S stVgsTask;
    VGS_ADD_COVER_S stVgsAddCover;
    static HI_U32 u32Frm = 0;
    u32Frm++;
    if (0 == pstRect->u32TotalNum)
    {
        return s32Ret;
    }
    s32Ret = HI_MPI_VGS_BeginJob(&VgsHandle);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("Vgs begin job fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    memcpy(&stVgsTask.stImgIn, pstFrmInfo, sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&stVgsTask.stImgOut, pstFrmInfo, sizeof(VIDEO_FRAME_INFO_S));

    stVgsAddCover.enCoverType = COVER_QUAD_RANGLE;
    stVgsAddCover.u32Color = u32Color;
    stVgsAddCover.stQuadRangle.bSolid = HI_FALSE;
    stVgsAddCover.stQuadRangle.u32Thick = 2;

    for (i = 0; i < pstRect->u32ClsNum; i++)
    {
        for (j = 0; j < pstRect->au32RoiNum[i]; j++)
        {
            memcpy(stVgsAddCover.stQuadRangle.stPoint, pstRect->astRect[i][j].ast_uPoint, sizeof(pstRect->astRect[i][j].ast_uPoint));
            s32Ret = HI_MPI_VGS_AddCoverTask(VgsHandle, &stVgsTask, &stVgsAddCover);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VGS_AddCoverTask fail,Error(%#x)\n", s32Ret);
                HI_MPI_VGS_CancelJob(VgsHandle);
                return s32Ret;
            }

        }

    }

    s32Ret = HI_MPI_VGS_EndJob(VgsHandle);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VGS_EndJob fail,Error(%#x)\n", s32Ret);
        HI_MPI_VGS_CancelJob(VgsHandle);
        return s32Ret;
    }

    return s32Ret;

}