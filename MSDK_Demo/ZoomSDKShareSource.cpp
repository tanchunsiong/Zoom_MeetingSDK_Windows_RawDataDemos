
//SendShareScreenRawData
#include "ZoomSDKShareSource.h"
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

using namespace cv;
using namespace std;

int video_share_play_flag = -1;
int share_height = 480;
int share_width = 640;


void PlayVideoFileToShare(IZoomSDKShareSender* share_sender, string share_source)
{
    char* frameBuffer;
    int frameLen = share_height / 2 * 3 * share_width;
    frameBuffer = (char*)malloc(frameLen);

    // execute in a thread.
    while (video_share_play_flag > 0 && share_sender) {
        Mat frame;
        VideoCapture cap;
        cap.open(share_source);
        if (!cap.isOpened()) {
            cerr << "ERROR! Unable to open camera\n";
            video_share_play_flag = 0;
            break;
        }
        else {
            //--- GRAB AND WRITE LOOP
            std::cout << "Start grabbing" << endl;
            while (video_share_play_flag > 0)
            {
                // wait for a new frame from camera and store it into 'frame'
                cap.read(frame);
                // check if we succeeded
                if (frame.empty()) {
                    cerr << "ERROR! blank frame grabbed\n";
                    break;
                }
                Mat resizedFrame;
                resize(frame, resizedFrame, Size(share_width, share_height), 0, 0, INTER_LINEAR);

                //covert Mat to YUV buffer
                Mat yuv;
                cv::cvtColor(resizedFrame, yuv, COLOR_BGRA2YUV_I420);
                char* p;
                for (int i = 0; i < share_height / 2 * 3; ++i) {
                    p = yuv.ptr<char>(i);
                    for (int j = 0; j < share_width; ++j) {
                        frameBuffer[i * share_width + j] = p[j];
                    }
                }
                ((IZoomSDKShareSender*)share_sender)->sendShareFrame(frameBuffer, share_width, share_height, frameLen);
            }
            cap.release();
        }
    }
    video_share_play_flag = -1;
}





void ZoomSDKShareSource::onStartSend(IZoomSDKShareSender* pSender)
{
    std::cout << "onStartSend" << endl;
    if (pSender && video_share_play_flag != 1) {
        while (video_share_play_flag > -1) {}
        video_share_play_flag = 1;
        thread(PlayVideoFileToShare, pSender, share_source_).detach();
    }
}

void ZoomSDKShareSource::onStopSend()
{
    std::cout << "onStopSend" << endl;
    video_share_play_flag = 0;
}



ZoomSDKShareSource::ZoomSDKShareSource(string video_source)
{
    share_source_ = video_source;
}

