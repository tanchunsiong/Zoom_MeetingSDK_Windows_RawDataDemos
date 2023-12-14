//SendVideoRawData

#include "ZoomSDKVideoSource.h"
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

using namespace cv;
using namespace std;

int video_play_flag = -1;
int height = 480;
int width = 640 ;





void PlayVideoFileToVirtualCamera(IZoomSDKVideoSender* video_sender, string video_source)
{
    char* frameBuffer;
    int frameLen = height / 2 * 3 * width;
    frameBuffer = (char*)malloc(frameLen);

    // execute in a thread.
    while (video_play_flag > 0 && video_sender) {
        Mat frame;
        VideoCapture cap;
        cap.open(video_source);
        if (!cap.isOpened()) {
            cerr << "ERROR! Unable to open camera\n";
            video_play_flag = 0;
            return;
        }
     
            //--- GRAB AND WRITE LOOP
            std::cout << "Start grabbing" << endl;
            while (cap.read(frame))
            {
                // wait for a new frame from camera and store it into 'frame'
               
                // check if we succeeded
                if (frame.empty()) {
                    cerr << "ERROR! blank frame grabbed\n";
                    break;
                }
               
                //covert Mat to YUV buffer
                Mat yuv;
                cv::cvtColor(frame, yuv, COLOR_BGRA2YUV_I420);

                char* frameBuffer = (char*)yuv.data;
                int width = yuv.cols;
                int height = yuv.rows;
                int frameLen = yuv.total() * yuv.elemSize();

             

                SDKError err = ((IZoomSDKVideoSender*)video_sender)->sendVideoFrame(frameBuffer, width, height, frameLen, 0);
                if (err != SDKERR_SUCCESS) {
                    std::cout << "sendVideoFrame failed: Error " << err << endl;
                }
            }
            cap.release();
        
    }
    video_play_flag = -1;
}

void ZoomSDKVideoSource::onInitialize(IZoomSDKVideoSender* sender, IList<VideoSourceCapability>* support_cap_list, VideoSourceCapability& suggest_cap)
{
    std::cout << "ZoomSDKVideoSource::onInitialize" << endl;
    video_sender_ = sender;
}

void ZoomSDKVideoSource::onPropertyChange(IList<VideoSourceCapability>* support_cap_list, VideoSourceCapability suggest_cap)
{
    std::cout << "onPropertyChange" << endl;
    std::cout << "suggest frame: " << suggest_cap.frame << endl;
    std::cout << "suggest size: " << suggest_cap.width << "x" << suggest_cap.height << endl;
    width = suggest_cap.width;
    height = suggest_cap.height;
    std::cout << "calculated frameLen: " << height / 2 * 3 * width << endl;
}

void ZoomSDKVideoSource::onStartSend()
{
    std::cout << "onStartSend" << endl;
    if (video_sender_ && video_play_flag != 1) {
        while (video_play_flag > -1) {}
        video_play_flag = 1;
        thread(PlayVideoFileToVirtualCamera, video_sender_, video_source_).detach();
    }
    else {
        std::cout << "video_sender_ is null" << endl;
    }
}

void ZoomSDKVideoSource::onStopSend()
{
    std::cout << "onStopSend" << endl;
    video_play_flag = 0;
}

void ZoomSDKVideoSource::onUninitialized()
{
    std::cout << "onUninitialized" << endl;
    video_sender_ = nullptr;
}

ZoomSDKVideoSource::ZoomSDKVideoSource(string video_source)
{
    video_source_ = video_source;
}

