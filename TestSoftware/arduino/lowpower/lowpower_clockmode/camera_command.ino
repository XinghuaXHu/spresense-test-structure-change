
#include <stdio.h>
#include <SDHCI.h>
#include <Camera.h>
#include <arch/board/board.h>

static void VideoCB(CamImage img)
{
    uint8_t *buf;
    size_t img_size;
    size_t width;
    size_t height;
    CAM_IMAGE_PIX_FMT format;
    int16_t x, y;
    int      w, h, i;

    img_size = img.getImgSize();

    if( img_size == 0 ){
        Serial.println("Error - Empty CamImage!");
        return;
    }
    else{
        width  = img.getWidth();
        height = img.getHeight();
        format = img.getPixFormat();
        img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
        buf    = img.getImgBuff();
        display_lcd((uint16_t*)buf, 0, 0, width, height, img_size);

    }
}

CamErr set_video_param(int width, int height, int freq)
{
    CamErr ret;
    CAM_VIDEO_FPS fps;

    Serial.println("Prepare camera");
    switch(freq)
    {
        case 5: fps = CAM_VIDEO_FPS_5;
                break;
        case 6: fps = CAM_VIDEO_FPS_6;
                break;
        case 7: fps = CAM_VIDEO_FPS_7_5;
                break;
        case 15: fps = CAM_VIDEO_FPS_15;
                break;
        case 30: fps = CAM_VIDEO_FPS_30;
                break;
        case 60: fps = CAM_VIDEO_FPS_60;
                break;
        case 120: fps = CAM_VIDEO_FPS_120;
                break;
        default:  fps = CAM_VIDEO_FPS_NONE;
                Serial.println("Error invarid FPS");
                break;
    }

    ret = theCamera.begin(1, fps, width, height, CAM_IMAGE_PIX_FMT_YUV422);

    if( ret != CAM_ERR_SUCCESS ){
        Serial.print("Error begin, Err = ");
        Serial.println(ret);
    }
    else{
        Serial.println("Success begin");
    }

    return ret;
}

CamErr start_video(void)
{
    CamErr ret = CAM_ERR_SUCCESS;

    Serial.println("Start streaming");

    ret = theCamera.startStreaming(true, VideoCB);

    if( ret != CAM_ERR_SUCCESS ){
        Serial.print("Error startStreaming, Err = ");
        Serial.println(ret);
    }
    else{
        Serial.println("Success startStreaming");
    }

    return ret;
}

CamErr stop_video(void)
{
    CamErr ret = CAM_ERR_SUCCESS;

    Serial.println("Stop streaming");

    ret = theCamera.startStreaming(false, VideoCB);

    if( ret != CAM_ERR_SUCCESS ){
        Serial.print("Error stopStreaming, Err = ");
        Serial.println(ret);
    }
    else{
        Serial.println("Success stopStreaming");
    }

    return ret;
}



#define VIDEO_CAPTURE_REPEAT_TIMES 10

void video_capture_test(void)
{
    int i;

    Serial.println("Start video capture test");
    Serial.print("Repeat start/stop video streaming ");
    Serial.print(VIDEO_CAPTURE_REPEAT_TIMES);
    Serial.println(" times.");

    set_video_param(320, 240, 60);

    for(i=0;i<VIDEO_CAPTURE_REPEAT_TIMES;i++){
        start_video();
        Serial.println(" Wait 5sec.");
        delay(500);
        stop_video();
        Serial.println(" Wait 5sec.");
        delay(5000);
    }

    Serial.println("Start video capture test");
    Serial.print("Repeat start/stop video streaming ");
    Serial.print(VIDEO_CAPTURE_REPEAT_TIMES*10);
    Serial.println(" times with NO_WAIT");

    for(i=0;i<VIDEO_CAPTURE_REPEAT_TIMES*10;i++){
        start_video();
        stop_video();
    }
}
