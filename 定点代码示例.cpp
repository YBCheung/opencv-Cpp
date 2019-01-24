#include <iostream>

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <ctype.h>

#include <unistd.h>

#define WIDE    160.0
#define HEIGHT  120.0
#define     min_threa   500
//中断引脚
#define REST   23
//亮灯引脚
#define Led_pin     28
#define shout_down   22//关机引脚

using namespace std;
using namespace cv;

u_int8_t old_x=0,old_y=0;
Mat g_srcImage;
Mat g_grayImage;
int g_nThresh = 80;
int g_nThresh_max = 255;
RNG g_rng(12345);
Mat g_cannyMat_output;
vector <vector <Point> > contours;
vector <Vec4i> g_vHierarchy;

bool flag_have=0;

int  set_gpio(void);
void REST_CallBack(void);
void Shout_down_Callback(void);

int fd;


int main()
{
        double t,fps;
        VideoCapture capture(0);
        //访问二值图像每个点的值
        double maxArea = 0;
        Point center;
        // 寻找最大连通域
        vector<cv::Point> maxContour;
        u_int8_t buf[8]={0xff,0x03,0x00,0x00,0x00,0x00,0x00,0xfe};
        // 寻找最大连通域
        while(1)
       {
            maxArea=0;
            capture>>g_srcImage;
            t = (double)cv::getTickCount();
            resize(g_srcImage,g_srcImage,Size(0,0),WIDE/640.0,HEIGHT/480.0,0);
            flag_have=false;

            // 转成灰度并模糊化降噪
            cvtColor( g_srcImage, g_grayImage, COLOR_BGR2GRAY );
            cv::threshold(g_grayImage,g_grayImage,150,255,2);//二值化
            blur( g_grayImage, g_grayImage, Size(3,3) );
            Canny( g_grayImage, g_cannyMat_output, g_nThresh, g_nThresh*2, 3 );
            imshow("Cany",g_cannyMat_output);

            // 寻找轮廓
            // 查找轮廓，对应连通域
            cv::findContours(g_cannyMat_output,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);

            if(flag_have==false)
            {
                center.x=old_x;
                center.y=old_y;
            }

           for(size_t i = 0; i < contours.size(); i++)
            {
              double area = (contourArea(contours[i]));
              //  printf("%.2f ",area);
                if (area > maxArea )
                {
                    maxArea = area;
                    maxContour = contours[i];
                    flag_have=true;
                }
            }

            if(flag_have==true)
            {
                // 将轮廓转为矩形框
                cv::Rect maxRect = cv::boundingRect(maxContour);
               //  printf("\nmax=%.2f,%.\n",maxArea);
                // 显示连通域
                cv::Mat result2,result1;
                g_grayImage.copyTo(result2);
                g_grayImage.copyTo(result1);

                for (size_t i = 0; i < contours.size(); i++)
                {
                    if((fabs)(contourArea(contours[i]))==maxArea)
                       // printf("asdasfasf\n");
                    {

                             cv::Rect r = cv::boundingRect(contours[i]);
                             cv::rectangle(result1, r, cv::Scalar(255));

                              cv::imshow("a", result1) ;

                              center.x=r.x+r.width/2;
                              center.y=HEIGHT-(r.y+r.height/2);
                              old_x=center.x;
                              old_y=center.y;
                    }
                }
                cv::rectangle(result2, maxRect, cv::Scalar(0,0,255));
                cv::imshow("b", result2) ;
            }
            printf("%d %d\n",center.x,center.y);

            t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
            fps = 1.0 / t;
            waitKey(1);
           // printf("fps：%.2f\n",fps);
      }
}

