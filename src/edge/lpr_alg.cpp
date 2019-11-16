#include "lpr_alg.h"
#include "rv_anpr_interface.h"
#include "jpg_codec.h"
#include "common.h"
#include <libyuv.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include <opencv2/opencv.hpp>

using namespace std;

#define WIDTH           4096            // Max image width
#define HEIGHT          2160            // Max image height

EyeJpegCodec ejc;
static unsigned char mem1[0x4000];                // 16K
static unsigned char mem2[100*1024*1024];            // 100M

//全局算法句柄
void *g_lprhandle;
//车牌识别初始化
bool vlpr_init()
{
    //打印算法版本信息
    char  szversion[64] ={0};
    RV_GETVERSION((unsigned char *)szversion) ;
    printf ("version:%s\n" ,szversion);
    //初始化算法句柄
    g_lprhandle =   RV_CREATE_LPR(WIDTH,HEIGHT, "test");
    if(g_lprhandle == NULL)
    {
        printf ("algorithm initialize error \n") ;
        return false;
    }
    
    return true;
}
//车牌颜色转化
//蓝 1   3
//黄 2   4
//白 3   2
//黑 4   3
//绿 5   5
//默认蓝色，没有黄绿色
//因为目前几乎没有黑色拍照，所有黑色拍照按照蓝牌处理
int pcolor_transfer(int c)
{
    int r=3;
    switch (c) {
        case 1:
            r=3;
            break;
        case 2:
            r=4;
            break;
        case 3:
            r=2;
            break;
        case 4:
            r=3;
            break;
        case 5:
            r=5;
            break;
    }
    return r;
}
//车牌识别，输入为jpg图像指针和长度，输出识别结果，识别结果结构体需要调用方分配
bool vlpr_analyze(const unsigned char *pImage, int len, PVPR pVPR)
{
    int w=0;int h=0; int c=4;
    int ww=0;int hh=0;
    char *argb_buf /*= (char *)malloc(WIDTH*HEIGHT*4)*/;
    //JPEG转为ARGB
    //bool ret = ejc.JpegUnCompress((char *)pImage, len, (char *)argb_buf,
    //                              WIDTH*HEIGHT*4, w, h, c);
    vector<unsigned char> data;
    for (int i = 0; i < len; ++i){
        data.push_back(pImage[i]);
    }
    cv::Mat image = cv::imdecode(cv::Mat(data), 1);
    if(!image.data)
    {
        printf ("imdecode error \n") ;
        return false;
    }
    printf ("imdecode OK \n") ;
    //做16字节对齐
    ww=image.cols;hh=image.rows;
    w=((ww-1)/16+1)*16;
    h=((hh-1)/16+1)*16;
    printf("%d x %d | %d x %d\n", ww, hh, w, h);
    cv::Mat image_bigger(w,h,image.type(),cv::Scalar(0,0,0));
    cv::Mat roi=image_bigger(cv::Rect(0,0,image.cols,image.rows));
    image.copyTo(roi);
    //做颜色转换 BGR->ARGB
    cv::Mat image_bgra;
    cv::cvtColor(image_bigger, image_bgra, cv::COLOR_BGR2BGRA);
    cv::Mat image_abgr(image_bgra.size(), image_bgra.type());
    int from_to[] = { 0,3, 1,2, 2,1, 3,0 };
    cv::mixChannels(&image_bgra,1,&image_abgr,1,from_to,4);
    //ARGB转为NV12
    uint8_t *y_data=(uint8_t *)malloc((int)(w*h*1.5));
    uint8_t *uv_data=y_data+w*h;
    libyuv::ARGBToNV12((const uint8_t*)image_abgr.data, w*c, y_data, w, uv_data, w, w, h);
    //开始识别车牌
    RV_ANPRRESULT anprresult[8] = { 0 }; //buff 必须大于最多识别的车牌数
    RV_RECRECT     rectroi;
    rectroi.left = 0; rectroi.right = w -1;
    rectroi.top =h /4; rectroi.bottom = h - 32;
    //识别区域 尽量避开图片上叠加的文字
    int nPlateNum = 1;
    int nRet = RV_RECFRAME_SINGLE(g_lprhandle ,y_data, w, h, RV_YUV420SP_UVUV , rectroi, anprresult, &nPlateNum);
    //识别之后释放NV12缓存
    if(y_data)
    {
        free(y_data);
        y_data=NULL;
    }
    if(nRet >= 0)
    {
        if (nPlateNum == 0) {
            return false;
        }
        //车牌结果输出
        std::string plate_gbk=anprresult[0].platenum;
        std::string plate_utf8;
        gbk2utf8(plate_gbk, plate_utf8);
        strcpy(pVPR->license, plate_utf8.c_str());
        //车牌颜色处理
        std::string pcolor_gbk=anprresult[0].platecolor;
        std::string pcolor_utf8;
        gbk2utf8(pcolor_gbk, pcolor_utf8);
        strcpy(pVPR->color, pcolor_utf8.c_str());
        pVPR->nColor = pcolor_transfer(anprresult[0].platecolorindex);
        //车牌类型
        pVPR->nType = anprresult[0].platetype;
        //置信度
        pVPR->nConfidence = anprresult[0].platereliability;
        //车牌坐标
        pVPR->left = anprresult[0].platerect.left;
        pVPR->right = anprresult[0].platerect.right;
        pVPR->top = anprresult[0].platerect.top;
        pVPR->bottom = anprresult[0].platerect.bottom;
        return true;
    }
    else{
        return false;
    }
}
