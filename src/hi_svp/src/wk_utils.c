

#include "wk_utils.h"



#define WK_SVP_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define WK_SVP_MIN(a, b) (((a) < (b)) ? (a) : (b))



// void get_frame_bdbox(WK_RECT_ARRAY_S *pstRect, HI_U32 s32width, HI_U32 s32heigth, HI_BOOL bfilter)
// {
//     int i,j;
   
//     for (i = 0; i <= pstRect->u32ClsNum; i++)
//     {
//         if(bfilter)
//         {
//             if (i != 1 )
//             {
//                 continue;
//             }
//         }
        
//         for (j = 0; j <= pstRect->au32RoiNum[i]; j++)
//         {
//             pstRect->astRect[i][j].ast_uPoint[0].s32X = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[0].s32X * (HI_FLOAT)s32width) & (~1);
//             pstRect->astRect[i][j].ast_uPoint[0].s32Y = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[0].s32Y * (HI_FLOAT)s32heigth) & (~1);

//             pstRect->astRect[i][j].ast_uPoint[1].s32X = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[1].s32X * (HI_FLOAT)s32width) & (~1);
//             pstRect->astRect[i][j].ast_uPoint[1].s32Y = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[1].s32Y * (HI_FLOAT)s32heigth) & (~1);

//             pstRect->astRect[i][j].ast_uPoint[2].s32X = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[2].s32X * (HI_FLOAT)s32width) & (~1);
//             pstRect->astRect[i][j].ast_uPoint[2].s32Y = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[2].s32Y * (HI_FLOAT)s32heigth) & (~1);

//             pstRect->astRect[i][j].ast_uPoint[3].s32X = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[3].s32X * (HI_FLOAT)s32width) & (~1);
//             pstRect->astRect[i][j].ast_uPoint[3].s32Y = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[3].s32Y * (HI_FLOAT)s32heigth) & (~1);
            
//             //printf("6666666666\n");
//             //printf("####s32XMin:%d \ns32YMin:%d \ns32XMax:%d \ns32YMax:%d \n", pstRect->astRect[i][j].ast_uPoint[0].s32X, pstRect->astRect[i][j].ast_uPoint[0].s32Y, pstRect->astRect[i][j].ast_uPoint[3].s32X, pstRect->astRect[i][j].ast_uPoint[3].s32Y);
           
//         }
        
//     }
//     return;
// }


// void Four_Point_Denormalization(WK_RECT_ARRAY_S *pstRect, HI_U32 s32width, HI_U32 s32heigth)
// {
//     int i,j;
   
//     for (i = 0; i <= pstRect->u32ClsNum; i++)
//     {
//         for (j = 0; j <= pstRect->au32RoiNum[i]; j++)
//         {
//             pstRect->astRect[i][j].ast_uPoint[0].s32X = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[0].s32X * (HI_FLOAT)s32width) & (~1);
//             pstRect->astRect[i][j].ast_uPoint[0].s32Y = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[0].s32Y * (HI_FLOAT)s32heigth) & (~1);

//             pstRect->astRect[i][j].ast_uPoint[1].s32X = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[1].s32X * (HI_FLOAT)s32width) & (~1);
//             pstRect->astRect[i][j].ast_uPoint[1].s32Y = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[1].s32Y * (HI_FLOAT)s32heigth) & (~1);

//             pstRect->astRect[i][j].ast_uPoint[2].s32X = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[2].s32X * (HI_FLOAT)s32width) & (~1);
//             pstRect->astRect[i][j].ast_uPoint[2].s32Y = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[2].s32Y * (HI_FLOAT)s32heigth) & (~1);

//             pstRect->astRect[i][j].ast_uPoint[3].s32X = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[3].s32X * (HI_FLOAT)s32width) & (~1);
//             pstRect->astRect[i][j].ast_uPoint[3].s32Y = (HI_S32)(pstRect->astRect[i][j].ast_fPoint[3].s32Y * (HI_FLOAT)s32heigth) & (~1);        
//         }
        
//     }
//     return;
// }


void NORMALIZATION_TO_S24Q8(WK_FLOAT_RECT_S *pstInputBbox, IVE_RECT_S24Q8_S *pstSlectBbox, HI_S32 img_W, HI_S32 img_H)
{
    pstSlectBbox->s24q8X = (HI_S32)(pstInputBbox->fX * (HI_FLOAT)img_W) * 256 & (~1);
    pstSlectBbox->s24q8Y = (HI_S32)(pstInputBbox->fY * (HI_FLOAT)img_H) * 256 & (~1);
    pstSlectBbox->u32Width = (HI_S32)(pstInputBbox->fWidth * (HI_FLOAT)img_W) & (~1);
    pstSlectBbox->u32Height = (HI_S32)(pstInputBbox->fHeight * (HI_FLOAT)img_H) & (~1);
    // printf("###pstSlectBbox->s24q8X:%d\n",pstSlectBbox->s24q8X);
    // printf("###pstSlectBbox->s24q8Y:%d\n",pstSlectBbox->s24q8Y);
    // printf("###pstSlectBbox->u32Width:%d\n",pstSlectBbox->u32Width);
    // printf("###pstSlectBbox->u32Height:%d\n",pstSlectBbox->u32Height);
    return;
}

void S24Q8_TO_NORMALIZATION(WK_FLOAT_RECT_S *pstResBbox, IVE_RECT_S24Q8_S *pstTrackBbox, HI_S32 img_W, HI_S32 img_H)
{
    pstResBbox->fX = (HI_FLOAT) pstTrackBbox->s24q8X / 256 / img_W;
    pstResBbox->fY = (HI_FLOAT) pstTrackBbox->s24q8Y / 256 / img_H;
    pstResBbox->fWidth = (HI_FLOAT)pstTrackBbox->u32Width / img_W;
    pstResBbox->fHeight = (HI_FLOAT)pstTrackBbox->u32Height / img_H;
    return;
}



void RECT_TO_NORMALIZATION(WK_RECT_ARRAY_S *pastRect_u, WK_RECT_ARRAY_S *pastRect_f, HI_S32 img_W, HI_S32 img_H)
{
    int i;
    pastRect_u->u16Num = pastRect_f->u16Num;
    for(i = 0; i++; i< pastRect_u->u16Num)
    {
        pastRect_f->astRect[i].stRect_f.fX = (float)pastRect_u->astRect[i].stRect_u.uX / img_W;
        pastRect_f->astRect[i].stRect_f.fY = (float)pastRect_u->astRect[i].stRect_u.uY / img_H;
        pastRect_f->astRect[i].stRect_f.fWidth = (float)pastRect_u->astRect[i].stRect_u.uWidth / img_W;
        pastRect_f->astRect[i].stRect_f.fHeight = (float)pastRect_u->astRect[i].stRect_u.uHeight / img_H;
    }
    return;
}

void RECT_TO_DENRMALIZATION(WK_RECT_ARRAY_S *pastRect_f, WK_RECT_ARRAY_S *pastRect_u, HI_S32 img_W, HI_S32 img_H)
{
    int i;
    pastRect_u->u16Num = pastRect_f->u16Num;
    for(i = 0; i< pastRect_f->u16Num; i++)
    {
        pastRect_u->astRect[i].stRect_u.uX = (int)(pastRect_f->astRect[i].stRect_f.fX * img_W) & (~1);
        pastRect_u->astRect[i].stRect_u.uY = (int)(pastRect_f->astRect[i].stRect_f.fY * img_H) & (~1);
        pastRect_u->astRect[i].stRect_u.uWidth = (int)(pastRect_f->astRect[i].stRect_f.fWidth * img_W) & (~1);
        pastRect_u->astRect[i].stRect_u.uHeight = (int)(pastRect_f->astRect[i].stRect_f.fHeight * img_H) & (~1);

        //printf("fH:%f   imgH:%d\n",pastRect_f->astRect[i].stRect_f.fHeight,img_H);

        #if 1
        printf("NUM:%d\n",i);
        printf("X:%d\n",pastRect_u->astRect[i].stRect_u.uX);
        printf("Y:%d\n",pastRect_u->astRect[i].stRect_u.uY);
        printf("W:%d\n",pastRect_u->astRect[i].stRect_u.uWidth);
        printf("H:%d\n",pastRect_u->astRect[i].stRect_u.uHeight);
        #endif
    }

   
    return;
}

void RECT_TO_POINT(WK_RECT_ARRAY_S *pstRect_u, WK_POINT_ARRAY_S *pstPoint_u)
{
    int i;
    for(i = 0; i < pstRect_u->u16Num; i++)
    {
        pstPoint_u->astPoint[i].astPoint_u[0].uX = pstRect_u->astRect[i].stRect_u.uX;
        pstPoint_u->astPoint[i].astPoint_u[0].uY = pstRect_u->astRect[i].stRect_u.uY;
        pstPoint_u->astPoint[i].astPoint_u[1].uX = pstRect_u->astRect[i].stRect_u.uX + pstRect_u->astRect[i].stRect_u.uWidth;
        pstPoint_u->astPoint[i].astPoint_u[1].uY = pstRect_u->astRect[i].stRect_u.uY;
        pstPoint_u->astPoint[i].astPoint_u[2].uX = pstRect_u->astRect[i].stRect_u.uX + pstRect_u->astRect[i].stRect_u.uWidth;
        pstPoint_u->astPoint[i].astPoint_u[2].uY = pstRect_u->astRect[i].stRect_u.uY + pstRect_u->astRect[i].stRect_u.uHeight; 
        pstPoint_u->astPoint[i].astPoint_u[3].uX = pstRect_u->astRect[i].stRect_u.uX;
        pstPoint_u->astPoint[i].astPoint_u[3].uY = pstRect_u->astRect[i].stRect_u.uY + pstRect_u->astRect[i].stRect_u.uHeight;

        #if 0
        int j;
        for( j = 0; j < 4; j++)
        {
            printf("###NUM %d:  X%d = %d   Y%d = %d\n",i, j, pstPoint_u->astPoint[i].astPoint_u[j].uX, j, pstPoint_u->astPoint[i].astPoint_u[j].uY);
        }
        #endif
    }
    return;
}


void Filter_Object(WK_YOLO_RECT_ARRAY_S *raw_pastRect_f, WK_RECT_ARRAY_S *out_pastRect_f, WK_YOLO_CLASS eClass)
{
    int i,j;
    out_pastRect_f->u16Num = 0;
    for (i = 0; i < raw_pastRect_f->u32ClsNum; i++)
    {
        if(i != eClass)
        {
            continue;
        }
        for(j = 0; j < raw_pastRect_f->au32RoiNum[i]; j++)
        {
            out_pastRect_f->astRect[j].stRect_f.fX = raw_pastRect_f->astRect[i][j].stRect.stRect_f.fX;
            out_pastRect_f->astRect[j].stRect_f.fY = raw_pastRect_f->astRect[i][j].stRect.stRect_f.fY;
            out_pastRect_f->astRect[j].stRect_f.fWidth = raw_pastRect_f->astRect[i][j].stRect.stRect_f.fWidth;
            out_pastRect_f->astRect[j].stRect_f.fHeight = raw_pastRect_f->astRect[i][j].stRect.stRect_f.fHeight;
            out_pastRect_f->u16Num += 1;
        }
    }
    return;
}



HI_FLOAT WK_CalcIOU(WK_FLOAT_RECT_S *pstRect1, WK_FLOAT_RECT_S *pstRect2)
{
    HI_FLOAT s32MinX, s32MinY, s32MaxX, s32MaxY;
    HI_FLOAT fArea1, fArea2, fInterArea, fIou;
    HI_FLOAT s32Width, s32Height;

    s32MinX = WK_SVP_MAX(pstRect1->fX , pstRect2->fX);
    s32MinY = WK_SVP_MAX(pstRect1->fY , pstRect2->fY);
    s32MaxX = WK_SVP_MIN(pstRect1->fX + pstRect1->fWidth, pstRect2->fX + pstRect2->fWidth);
    s32MaxY = WK_SVP_MIN(pstRect1->fY + pstRect1->fHeight, pstRect2->fY + pstRect2->fHeight);

    s32Width = s32MaxX - s32MinX + 1;
    s32Height = s32MaxY - s32MinY + 1;

    s32Width = s32Width > 0 ? s32Width : 0;
    s32Height = s32Height > 0 ? s32Height : 0;

    fInterArea = s32Width * s32Height;

    fArea1 = pstRect1->fWidth * pstRect1->fHeight;
    fArea2 = pstRect2->fWidth * pstRect2->fHeight;

    fIou = fInterArea / (fArea1 + fArea2 - fInterArea);

    return fIou;
}





// HI_VOID Rect_To_Point_Array(IVE_RECT_S24Q8_S astBbox[], HI_U32 u32BboxObjNum, WK_POINT_ARRAY_S *pstRect)
// {
//     HI_U32 i;

//     pstRect->u16Num = 0;
//     for (i = 0; i < u32BboxObjNum; i++)
//     {
//         pstRect->astRect[i].astuPoint[0].s32X = astBbox[i].s24q8X / 256 & (~1);
//         pstRect->astRect[i].astuPoint[0].s32Y = astBbox[i].s24q8Y / 256 & (~1);
//         // if ((pstRect->astRect[i].astPoint[0].s32X < 0) || (pstRect->astRect[i].astPoint[0].s32Y < 0))
//         // {
//         //     pstRect->astRect[i].astPoint[0].s32X = 0;
//         //     pstRect->astRect[i].astPoint[0].s32Y = 0;
//         // }
//         pstRect->astRect[i].astuPoint[1].s32X = pstRect->astRect[i].astuPoint[0].s32X + (astBbox[i].u32Width & (~1));
//         pstRect->astRect[i].astuPoint[1].s32Y = pstRect->astRect[i].astuPoint[0].s32Y;
//         pstRect->astRect[i].astuPoint[2].s32X = pstRect->astRect[i].astuPoint[0].s32X + (astBbox[i].u32Width & (~1));
//         pstRect->astRect[i].astuPoint[2].s32Y = pstRect->astRect[i].astuPoint[0].s32Y + (astBbox[i].u32Height & (~1));
//         pstRect->astRect[i].astuPoint[3].s32X = pstRect->astRect[i].astuPoint[0].s32X;
//         pstRect->astRect[i].astuPoint[3].s32Y = pstRect->astRect[i].astuPoint[0].s32Y + (astBbox[i].u32Height & (~1));
//         // printf("###num:%d;x0:%d\n",i,pstRect->astRect[i].astuPoint[0].s32X);
//         // printf("###num:%d;y0:%d\n",i,pstRect->astRect[i].astuPoint[0].s32Y);
//         // printf("###num:%d;x1:%d\n",i,pstRect->astRect[i].astuPoint[1].s32X);
//         // printf("###num:%d;y1:%d\n",i,pstRect->astRect[i].astuPoint[1].s32Y);
//         // printf("###num:%d;x2:%d\n",i,pstRect->astRect[i].astuPoint[2].s32X);
//         // printf("###num:%d;y2:%d\n",i,pstRect->astRect[i].astuPoint[2].s32Y);
//         // printf("###num:%d;x3:%d\n",i,pstRect->astRect[i].astuPoint[3].s32X);
//         // printf("###num:%d;y3:%d\n",i,pstRect->astRect[i].astuPoint[3].s32Y);
//         pstRect->u16Num++;
//     }
// }








HI_S32 WK_Fill_ONE_Rect(VIDEO_FRAME_INFO_S *pstFrmInfo, WK_POINT_ARRAY_S* pstbbox, HI_U32 u32Color)
{
    VGS_HANDLE VgsHandle = -1;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i,j;
    VGS_TASK_ATTR_S stVgsTask;
    VGS_ADD_COVER_S stVgsAddCover;
    static HI_U32 u32Frm = 0;
    u32Frm++;
    
    s32Ret = HI_MPI_VGS_BeginJob(&VgsHandle);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Vgs begin job fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    memcpy(&stVgsTask.stImgIn, pstFrmInfo, sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&stVgsTask.stImgOut, pstFrmInfo, sizeof(VIDEO_FRAME_INFO_S));

    stVgsAddCover.enCoverType = COVER_QUAD_RANGLE;
    stVgsAddCover.u32Color = u32Color;
    stVgsAddCover.stQuadRangle.bSolid = HI_FALSE;
    stVgsAddCover.stQuadRangle.u32Thick = 2;

   
    memcpy(stVgsAddCover.stQuadRangle.stPoint, pstbbox->astPoint[0].astPoint_u, sizeof(pstbbox->astPoint[0].astPoint_u));
    s32Ret = HI_MPI_VGS_AddCoverTask(VgsHandle, &stVgsTask, &stVgsAddCover);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VGS_AddCoverTask fail,Error(%#x)\n", s32Ret);
        HI_MPI_VGS_CancelJob(VgsHandle);
        return s32Ret;
    }

  

    s32Ret = HI_MPI_VGS_EndJob(VgsHandle);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VGS_EndJob fail,Error(%#x)\n", s32Ret);
        HI_MPI_VGS_CancelJob(VgsHandle);
        return s32Ret;
    }

    return s32Ret;

}



void YOLOV3_RECT_TO_DENRMALIZATION(WK_YOLO_RECT_ARRAY_S *pastRect_f, WK_YOLO_RECT_ARRAY_S *pastRect_u, HI_S32 img_W, HI_S32 img_H)
{

    int i,j;
    pastRect_u->u32TotalNum = pastRect_f->u32TotalNum;
    pastRect_u->u32ClsNum = pastRect_f->u32ClsNum;
    memcpy(pastRect_u->au32RoiNum,pastRect_f->au32RoiNum,sizeof(pastRect_f->au32RoiNum));
    if (pastRect_f->u32TotalNum == 0)
    {
        return;
    }
    for(i = 0; i < pastRect_f->u32ClsNum; i++)
    {
        for(j = 0; j < pastRect_f->au32RoiNum[i]; j++ )
        { 
            pastRect_u->astRect[i][j].f32Score = pastRect_f->astRect[i][j].f32Score;
            pastRect_u->astRect[i][j].stRect.stRect_u.uX = (int)(pastRect_f->astRect[i][j].stRect.stRect_f.fX * img_W) & (~1);
            pastRect_u->astRect[i][j].stRect.stRect_u.uY = (int)(pastRect_f->astRect[i][j].stRect.stRect_f.fY * img_H) & (~1);
            pastRect_u->astRect[i][j].stRect.stRect_u.uWidth = (int)(pastRect_f->astRect[i][j].stRect.stRect_f.fWidth * img_W) & (~1);
            pastRect_u->astRect[i][j].stRect.stRect_u.uHeight = (int)(pastRect_f->astRect[i][j].stRect.stRect_f.fHeight * img_H) & (~1);
            // printf("### X:%d\n",pastRect_u->astRect[i][j].stRect.stRect_u.uX);
            // printf("### Y:%d\n",pastRect_u->astRect[i][j].stRect.stRect_u.uY);
            // printf("### W:%d\n",pastRect_u->astRect[i][j].stRect.stRect_u.uWidth);
            // printf("### H:%d\n",pastRect_u->astRect[i][j].stRect.stRect_u.uHeight);
        }

    }  
    return;
}




HI_S32 SAMPLE_COMM_SVP_NNIE_YOLOV3_FillRect(VIDEO_FRAME_INFO_S *pstFrmInfo, WK_YOLO_RECT_ARRAY_S* pstRect_u, HI_U32 u32Color)
{
    VGS_HANDLE VgsHandle = -1;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i,j;
    VGS_TASK_ATTR_S stVgsTask;
    VGS_ADD_COVER_S stVgsAddCover;
    static HI_U32 u32Frm = 0;
    u32Frm++;
    if (0 == pstRect_u->u32TotalNum)
    {
        return s32Ret;
    }
    s32Ret = HI_MPI_VGS_BeginJob(&VgsHandle);
    if (s32Ret != HI_SUCCESS)
    {
        printf("Vgs begin job fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    memcpy(&stVgsTask.stImgIn, pstFrmInfo, sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&stVgsTask.stImgOut, pstFrmInfo, sizeof(VIDEO_FRAME_INFO_S));

    stVgsAddCover.enCoverType = COVER_QUAD_RANGLE;
    stVgsAddCover.u32Color = u32Color;
    stVgsAddCover.stQuadRangle.bSolid = HI_FALSE;
    stVgsAddCover.stQuadRangle.u32Thick = 2;

    for (i = 0; i < pstRect_u->u32ClsNum; i++)
    {
        if(pstRect_u->au32RoiNum[i] == 0)
        {
            continue;
        }

        for (j = 0; j < pstRect_u->au32RoiNum[i]; j++)
        {
            WK_RECT_ARRAY_S astRect_u = {0};
            WK_POINT_ARRAY_S astPoint_u = {0};
            astRect_u.u16Num = 1;
            astRect_u.astRect[0].stRect_u = pstRect_u->astRect[i][j].stRect.stRect_u;
            RECT_TO_POINT(&astRect_u, &astPoint_u);
            memcpy(stVgsAddCover.stQuadRangle.stPoint, astPoint_u.astPoint[0].astPoint_u, sizeof(astPoint_u.astPoint[0].astPoint_u));
            s32Ret = HI_MPI_VGS_AddCoverTask(VgsHandle, &stVgsTask, &stVgsAddCover);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VGS_AddCoverTask fail,Error(%#x)\n", s32Ret);
                // SAMPLE_PRT("HI_MPI_VGS_AddCoverTask fail,Error(%#x)\n", s32Ret);
                HI_MPI_VGS_CancelJob(VgsHandle);
                return s32Ret;
            }

        }

    }

    s32Ret = HI_MPI_VGS_EndJob(VgsHandle);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VGS_EndJob fail,Error(%#x)\n", s32Ret);
        HI_MPI_VGS_CancelJob(VgsHandle);
        return s32Ret;
    }

    return s32Ret;

}