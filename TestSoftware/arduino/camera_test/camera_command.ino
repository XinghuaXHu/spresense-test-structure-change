
#include <stdio.h>
#include <SDHCI.h>
#include <Camera.h>

#include <arch/board/board.h>
#include "msc.h"

#define FILENAME_LEN (64)

#define MAX_ARG_NUM 5
#define MAX_BUF_LEN 20
  
char buf[MAX_BUF_LEN];
SDClass theSD;

uint32_t still_capture_cnt = 0;
uint32_t video_capture_cnt = 0;
File myFile;
String args[MAX_ARG_NUM];
String video_capture_folder;
bool video_capture_start = false;

//Camera command shell
static int parse_args(String string)
{
    int from = 0;
    int pos;
    int index = 0;

    while( (pos = string.indexOf(" ", from)) != -1 ){
        if( index < MAX_ARG_NUM ){
            args[index++] = string.substring(from, pos);
            from = pos+1;
        }
        else{
            return -1;
        }
    }

    args[index++] = string.substring(from);

    return index;
}


static void VideoCB(CamImage img)
{
    uint8_t *buf;
    size_t img_size;
    size_t width;
    size_t height;
    CAM_IMAGE_PIX_FMT format;
    int16_t x, y;
    int      w, h, i;

#ifdef _SAVE_CAPTURE_DATA
    char video_filename[FILENAME_LEN] = {0};
    char foldername[16] = {0};
#endif
    
    
    img_size = img.getImgSize();

    if( img_size == 0 ){
        Serial.println("Error - Empty CamImage!");
        return;
    }
    else{
        //buf    = img.getImgBuff();
        width  = img.getWidth();
        height = img.getHeight();
        format = img.getPixFormat();
#if 0
        Serial.print("Image size = ");
        Serial.println(img_size);

        Serial.print("Width = ");
        Serial.println(width);

        Serial.print("Height = ");
        Serial.println(height);

        Serial.print("Format = ");

        switch(format)
        {
            case CAM_IMAGE_PIX_FMT_RGB565:
                Serial.println("RGB565");
                break;
            case CAM_IMAGE_PIX_FMT_YUV422:
                Serial.println("YUV422");
                break;
            case CAM_IMAGE_PIX_FMT_JPG:
                Serial.println("JPG");
                break;
            case CAM_IMAGE_PIX_FMT_NONE:
                Serial.println("None");
                break;
            default:
                Serial.println("Error - format");
        }
#endif


        if (true == video_capture_start){

            sprintf(foldername, "%s_%d",video_capture_folder.c_str(),++video_capture_cnt);
            Serial.print("foldername = ");
            Serial.println(foldername);
            
            /* Save to SD card */
            if( (width == 320)&&(height == 240) ){
                sprintf(video_filename, "%s/qvga%04d.yuv", foldername, video_capture_cnt);
            }
            else if( (width == 640)&&(height == 480) ){
                sprintf(video_filename, "%s/vga%04d.yuv", foldername, video_capture_cnt);
            }
            else if( (width == 1280)&&(height == 720) ){
                sprintf(video_filename, "%s/hd%04d.yuv", foldername, video_capture_cnt);
            }
            else{
                sprintf(video_filename, "%s/VIDEO%04d.yuv", foldername, video_capture_cnt);
            }
            Serial.print("video_filename = ");
            Serial.println(video_filename);
            
            //theSD.mkdir(foldername);
            
            img.convertPixFormat(CAM_IMAGE_PIX_FMT_YUV422);
            buf    = img.getImgBuff();  
            myFile = theSD.open(video_filename, FILE_WRITE);
            if (!myFile){
                Serial.println("File open error");
                return 0;
            }
            myFile.write(buf, img_size);
            myFile.close();

            if(0 == video_capture_cnt%3){
                video_capture_start = false;
              }  
          }
 
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

CamErr set_still_param(int width, int height, int format)
{
    CamErr ret;
    CAM_IMAGE_PIX_FMT fmt;

    Serial.println(width);
    Serial.println(height);

    switch(format)
    {
        case 1: fmt = CAM_IMAGE_PIX_FMT_RGB565;
                break;
        case 2: fmt = CAM_IMAGE_PIX_FMT_YUV422;
                break;
        case 3: fmt = CAM_IMAGE_PIX_FMT_JPG;
                break;
        default:  fmt = CAM_IMAGE_PIX_FMT_NONE;
                Serial.println("Error invarid format");
                break;
    }

    ret = theCamera.setStillPictureImageFormat(
        width,                    /**< [en] Image width of Still picture.(px)   <BR> [ja] 静止画写真の横サイズ (単位ピクセル) */
        height,                    /**< [en] Image height of Still picture.(px)  <BR> [ja] 静止画写真の縦サイズ (単位ピクセル) */
        fmt                  /**< [en] Image pixcel format. (Default JPEG) <BR> [ja] 静止画ピクセルフォーマット (デフォルト JPEG) */
    );

    if( ret != CAM_ERR_SUCCESS ){
        Serial.print("Error setStillPictiureImageFormat, Err = ");
        Serial.println(ret);
    }
    else{
        Serial.println("Success setStillPictiureImageFormat");
    }

    return ret;
}

CamErr set_awb(bool enable)
{
    CamErr ret;

    ret = theCamera.setAutoWhiteBalance(enable);

    if( ret != CAM_ERR_SUCCESS ){
        Serial.print("Error setAutoWhiteBalance, Err = ");
        Serial.println(ret);
    }
    else{
        Serial.println("Success setAutoWhiteBalance");
    }

    return ret;
}

CamErr set_awb_mode(int mode )
{
    CamErr ret;
    CAM_WHITE_BALANCE awb_mode;

    switch(mode)
    {
        case 1: awb_mode = CAM_WHITE_BALANCE_INCANDESCENT;
              break;
        case 2: awb_mode = CAM_WHITE_BALANCE_FLUORESCENT;
              break;
        case 3: awb_mode = CAM_WHITE_BALANCE_DAYLIGHT;
              break;
        case 4: awb_mode = CAM_WHITE_BALANCE_FLASH;
              break;
        case 5: awb_mode = CAM_WHITE_BALANCE_CLOUDY;
              break;
        case 6: awb_mode = CAM_WHITE_BALANCE_SHADE;
              break;
        default:
                awb_mode = CAM_WHITE_BALANCE_INCANDESCENT;
              break;
    }

    ret = theCamera.setAutoWhiteBalanceMode(awb_mode);

    if( ret != CAM_ERR_SUCCESS ){
        Serial.print("Error setAutoWhiteBalanceMode, Err = ");
        Serial.println(ret);
    }
    else{
        Serial.println("Success setAutoWhiteBalanceMode");
    }

    return ret;
}

CamErr set_iso_sensitivity(int sensitivity )
{
    CamErr ret;
    int tmp;
    int iso_sensitivity;

    switch(sensitivity)
    {
        case 25: iso_sensitivity = CAM_ISO_SENSITIVITY_25;
                  break;
        case 32: iso_sensitivity = CAM_ISO_SENSITIVITY_32;
                  break;
        case 40: iso_sensitivity = CAM_ISO_SENSITIVITY_40;
                  break;
        case 50: iso_sensitivity = CAM_ISO_SENSITIVITY_50;
                  break;
        case 64: iso_sensitivity = CAM_ISO_SENSITIVITY_64;
                  break;
        case 80: iso_sensitivity = CAM_ISO_SENSITIVITY_80;
                  break;
        case 100: iso_sensitivity = CAM_ISO_SENSITIVITY_100;
                  break;
        case 125: iso_sensitivity = CAM_ISO_SENSITIVITY_125;
                  break;
        case 160: iso_sensitivity = CAM_ISO_SENSITIVITY_160;
                  break;
        case 200: iso_sensitivity = CAM_ISO_SENSITIVITY_200;
                  break;
        case 250: iso_sensitivity = CAM_ISO_SENSITIVITY_250;
                  break;
        case 320: iso_sensitivity = CAM_ISO_SENSITIVITY_320;
                  break;
        case 400: iso_sensitivity = CAM_ISO_SENSITIVITY_400;
                  break;
        case 500: iso_sensitivity = CAM_ISO_SENSITIVITY_500;
                  break;
        case 640: iso_sensitivity = CAM_ISO_SENSITIVITY_640;
                  break;
        case 800: iso_sensitivity = CAM_ISO_SENSITIVITY_800;
                  break;
        case 1000: iso_sensitivity = CAM_ISO_SENSITIVITY_1000;
                  break;
        case 1250: iso_sensitivity = CAM_ISO_SENSITIVITY_1250;
                  break;
        case 1600: iso_sensitivity = CAM_ISO_SENSITIVITY_1600;
                  break;
        default: iso_sensitivity = 0;
                  Serial.println("Error- Invalid parameter");
                  return CAM_ERR_INVALID_PARAM;
    }

    Serial.print("Test ISO sensitivity, ISO=");
    Serial.println(sensitivity);

    ret = theCamera.setISOSensitivity(iso_sensitivity);

    if( ret != CAM_ERR_SUCCESS ){
        Serial.print("Error setISOSensitivity, Err = ");
        Serial.println(ret);
    }
    else{
        Serial.println("Success setISOSensitivity");
    }

    return ret;
}

#if 0
CamErr set_ae(bool enable)
{
    CamErr ret;

    ret = theCamera.setAutoExposure(enable);

    if( ret != CAM_ERR_SUCCESS ){
        Serial.print("Error setAutoExposure, Err = ");
        Serial.println(ret);
    }
    else{
        Serial.println("Success setAutoExposure");
    }

    return ret;
}
#endif

CamErr set_aiso(bool enable)
{
    CamErr ret;

    ret = theCamera.setAutoISOSensitive(enable);

    if( ret != CAM_ERR_SUCCESS ){
        Serial.print("Error setAutoISOSensitive, Err = ");
        Serial.println(ret);
    }
    else{
        Serial.println("Success setAutoISOSensitive");
    }

    return ret;
}

#if 0
CamErr set_scene_mode(int mode )
{
    CamErr ret;
    CAM_SCENE_MODE scene_mode;

    Serial.print("mode=");
    Serial.println(mode);

    switch(mode)
    {
        case 1: scene_mode = CAM_SCENE_MODE_NONE;
              break;
        case 2: scene_mode = CAM_SCENE_MODE_BACKLIGHT;
              break;
        case 3: scene_mode = CAM_SCENE_MODE_BEACH_SNOW;
              break;
        case 4: scene_mode = CAM_SCENE_MODE_CANDLE_LIGHT;
              break;
        case 5: scene_mode = CAM_SCENE_MODE_DAWN_DUSK;
              break;
        case 6: scene_mode = CAM_SCENE_MODE_FALL_COLORS;
              break;
        case 7: scene_mode = CAM_SCENE_MODE_FIREWORKS;
              break;
        case 8: scene_mode = CAM_SCENE_MODE_LANDSCAPE;
              break;
        case 9: scene_mode = CAM_SCENE_MODE_NIGHT;
              break;
        case 10: scene_mode = CAM_SCENE_MODE_PARTY_INDOOR;
              break;
        case 11: scene_mode = CAM_SCENE_MODE_PORTRAIT;
              break;
        case 12: scene_mode = CAM_SCENE_MODE_SPORTS;
              break;
        case 13: scene_mode = CAM_SCENE_MODE_SUNSET;
              break;
        default:
                scene_mode = CAM_SCENE_MODE_NONE;
              break;
    }

    ret = theCamera.setSceneMode(scene_mode);

    if( ret != CAM_ERR_SUCCESS ){
        Serial.print("Error setSceneMode, Err = ");
        Serial.println(ret);
    }
    else{
        Serial.println("Success setSceneMode");
    }

    return ret;
}
#endif

CamErr set_color_mode(int mode )
{
    CamErr ret;
    CAM_COLOR_FX color_mode;

    Serial.print("mode=");
    Serial.println(mode);

    switch(mode)
    {
        case 1: color_mode = CAM_COLOR_FX_NONE;
              break;
        case 2: color_mode = CAM_COLOR_FX_BW;
              break;
        case 3: color_mode = CAM_COLOR_FX_SEPIA;
              break;
        case 4: color_mode = CAM_COLOR_FX_NEGATIVE;
              break;
        case 5: color_mode = CAM_COLOR_FX_EMBOSS;
              break;
        case 6: color_mode = CAM_COLOR_FX_SKETCH;
              break;
        case 7: color_mode = CAM_COLOR_FX_SKY_BLUE;
              break;
        case 8: color_mode = CAM_COLOR_FX_GRASS_GREEN;
              break;
        case 9: color_mode = CAM_COLOR_FX_SKIN_WHITEN;
              break;
        case 10: color_mode = CAM_COLOR_FX_VIVID;
              break;
        case 11: color_mode = CAM_COLOR_FX_AQUA;
              break;
        case 12: color_mode = CAM_COLOR_FX_ART_FREEZE;
              break;
        case 13: color_mode = CAM_COLOR_FX_SILHOUETTE;
              break;
        case 14: color_mode = CAM_COLOR_FX_SOLARIZATION;
              break;
        case 15: color_mode = CAM_COLOR_FX_ANTIQUE;
              break;
        case 16: color_mode = CAM_COLOR_FX_SET_CBCR;
              break;
        case 17: color_mode = CAM_COLOR_FX_PASTEL;
              break;
        default:
                 color_mode = CAM_COLOR_FX_NONE;
              break;
    }

    ret = theCamera.setColorEffect(color_mode);

    if( ret != CAM_ERR_SUCCESS ){
        Serial.print("Error setColorEffect, Err = ");
        Serial.println(ret);
    }
    else{
        Serial.println("Success setColorEffect");
    }

    return ret;
}

int capture_test(String folder)
{
    CamImage img;
    uint8_t *buf;
    size_t img_size;
    size_t width;
    size_t height;
    uint8_t r, g, b;
    int16_t x, y;
    int      w, h, i;

    char still_filename[FILENAME_LEN] = {0};
    CamErr ret;

    Serial.println("Start - TakePicture!");
    img = theCamera.takePicture();
    Serial.println("Finish - TakePicture!");

    img_size = img.getImgSize();

    if( img_size == 0 ){
        Serial.println("Error - TakePicture!");
        return 0;
    }
    else{
#if 0
        ret = img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
        if( ret != CAM_ERR_SUCCESS ){
            Serial.println("Error Convert");
            return 0;
        }
        else{
            Serial.println("Sucess Convert");
        }
#endif
        buf    = img.getImgBuff();
        width  = img.getWidth();
        height = img.getHeight();

        Serial.print("Image size = ");
        Serial.println(img_size);

        Serial.print("Width = ");
        Serial.println(width);

        Serial.print("Height = ");
        Serial.println(height);

        Serial.print("Format = ");
        switch(img.getPixFormat())
        {
            case CAM_IMAGE_PIX_FMT_RGB565:
                Serial.println("RGB565");
                break;
            case CAM_IMAGE_PIX_FMT_YUV422:
                Serial.println("YUV422");
                break;
            case CAM_IMAGE_PIX_FMT_JPG:
                Serial.println("JPG");
                break;
            case CAM_IMAGE_PIX_FMT_NONE:
                Serial.println("None");
                break;
            default:
                Serial.println("Error - format");
        }

        /* Save to SD card */
        if( (width == 320)&&(height == 240) ){
            sprintf(still_filename, "%s/qvga%04d.jpg", folder.c_str(), still_capture_cnt++);
        }
        else if( (width == 640)&&(height == 480) ){
            sprintf(still_filename, "%s/vga%04d.jpg", folder.c_str(), still_capture_cnt++);
        }
        else if( (width == 1280)&&(height == 720) ){
            sprintf(still_filename, "%s/hd%04d.jpg", folder.c_str(), still_capture_cnt++);
        }
        else{
            sprintf(still_filename, "%s/STILL%04d.jpg", folder.c_str(), still_capture_cnt++);
        }

        Serial.print("still_filename = ");
        Serial.println(still_filename);
     
        myFile = theSD.open(still_filename, FILE_WRITE);
        if (!myFile){
            Serial.println("File open error");
            return 0;
        }
        myFile.write(buf, img_size);
        myFile.close();

        Serial.println("capture success");
    }

    return 1;
}

int capture()
{
    CamImage img;
    uint8_t *buf;
    size_t img_size;
    size_t width;
    size_t height;
    uint8_t r, g, b;
    int16_t x, y;
    int      w, h, i;

    char still_filename[FILENAME_LEN] = {0};
    CamErr ret;

    Serial.println("Start - TakePicture!");
    img = theCamera.takePicture();
    Serial.println("Finish - TakePicture!");

    img_size = img.getImgSize();

    if( img_size == 0 ){
        Serial.println("Error - TakePicture!");
        return 0;
    }
    else{
#if 0
        ret = img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
        if( ret != CAM_ERR_SUCCESS ){
            Serial.println("Error Convert");
            return 0;
        }
        else{
            Serial.println("Sucess Convert");
        }
#endif
        buf    = img.getImgBuff();
        width  = img.getWidth();
        height = img.getHeight();

        Serial.print("Image size = ");
        Serial.println(img_size);

        Serial.print("Width = ");
        Serial.println(width);

        Serial.print("Height = ");
        Serial.println(height);

        Serial.print("Format = ");
        switch(img.getPixFormat())
        {
            case CAM_IMAGE_PIX_FMT_RGB565:
                Serial.println("RGB565");
                break;
            case CAM_IMAGE_PIX_FMT_YUV422:
                Serial.println("YUV422");
                break;
            case CAM_IMAGE_PIX_FMT_JPG:
                Serial.println("JPG");
                break;
            case CAM_IMAGE_PIX_FMT_NONE:
                Serial.println("None");
                break;
            default:
                Serial.println("Error - format");
        }

        /* Save to SD card */
        if( (width == 320)&&(height == 240) ){
            sprintf(still_filename, "qvga%04d.jpg", still_capture_cnt++);
        }
        else if( (width == 640)&&(height == 480) ){
            sprintf(still_filename, "vga%04d.jpg", still_capture_cnt++);
        }
        else if( (width == 1280)&&(height == 720) ){
            sprintf(still_filename, "hd%04d.jpg", still_capture_cnt++);
        }
        else{
            sprintf(still_filename, "STILL%04d.jpg", still_capture_cnt++);
        }

        Serial.print("still_filename = ");
        Serial.println(still_filename);

        
        myFile = theSD.open(still_filename, FILE_WRITE);
        if (!myFile){
            Serial.println("File open error");
            return 0;
        }
        myFile.write(buf, img_size);
        myFile.close();
    }

    return 1;
}

void init_test(void)
{
    Serial.println("Start Initialize");
    set_video_param(320, 240, 60);
    start_video();
    Serial.println("Finish Initialize");
}

#define REPEAT_AWB_TEST_TIMES 10

void awb_test(void)
{
    CamErr ret;
    int i;
    String tmp;
    String folder;
    
    Serial.println();
    //Test Auto white balance mode
    Serial.println("Start Auto white balance mode test");

    //Start video streaming
    init_test();

    //Still param
    set_still_param(320,240,3);

    //
    ret = set_awb(true);
    folder = "autotest/Test_16239_0";
    if(ret != CAM_ERR_SUCCESS){
      Serial.println("set_awb Enable error");
      return;
    }
    else{
      //theSD.mkdir(folder);
      capture_test(folder);
    }
    Serial.println("Enable AWB");

    video_capture_folder = "autotest/Test_16228";
    video_capture_start = true;
    //delay(300);
    sleep(3);
    
    ret = set_awb(false);
    folder = "autotest/Test_16239_1";
    if(ret != CAM_ERR_SUCCESS){
        Serial.println("set_awb Disable error");
        return;
    }
    else{
      //theSD.mkdir(folder);
      capture_test(folder);
    }
    Serial.println("Disable AWB");

    video_capture_folder = "autotest/Test_16228";
    video_capture_start = true;
    //delay(400);
    sleep(3);
    
    //Stop Video capture
    stop_video();

    Serial.println("Finish awb test");
}

#define AWB_MODE_PARAM_COUNT 6

void awb_mode_test()
{
    CamErr ret;
    int i = 0;
    String tmp;
    String folder;
    
    Serial.println();

    //Start video streaming
    init_test();

    //Still param
    set_still_param(320,240,3);

    //Test Auto white balance mode
    Serial.println("Start Auto white balance mode test");
    for(i=0;i<AWB_MODE_PARAM_COUNT;i++){
        switch(i)
        {
          case 0: tmp = "CAM_WHITE_BALANCE_INCANDESCENT";
                  folder = "autotest/Test_16241_0";
                  break;
          case 1: tmp = "CAM_WHITE_BALANCE_FLUORESCENT";
                  folder = "autotest/Test_16241_1";
                  break;
          case 2: tmp = "CAM_WHITE_BALANCE_DAYLIGHT";
                  folder = "autotest/Test_16241_2";
                  break;
          case 3: tmp = "CAM_WHITE_BALANCE_FLASH";
                  folder = "autotest/Test_16241_3";
                  break;
          case 4: tmp = "CAM_WHITE_BALANCE_CLOUDY";
                  folder = "autotest/Test_16241_4";
                  break;
          case 5: tmp = "CAM_WHITE_BALANCE_SHADE";
                  folder = "autotest/Test_16241_5";
                  break;
        }
        Serial.print("Test ");
        Serial.println(tmp);
        ret = set_awb_mode(i+1);
        if( ret == CAM_ERR_SUCCESS ){
          //theSD.mkdir(folder);
          capture_test(folder);
          
          video_capture_folder = "autotest/Test_16230";
          video_capture_start = true;
          //delay(400);
          sleep(3);
          
          Serial.println();
        }


    }


    if(i == AWB_MODE_PARAM_COUNT){
        Serial.println("Finish awb mode test");
    }

}

#if 0
void scene_mode_test(void)
{
    CamErr ret;
    int i;
    String tmp;

    Serial.println();

    //Test scene mode
    Serial.println("Start scene mode test");
    for(i=0;i<6;i++){
        switch(i)
        {
          case 0: tmp = "CAM_SCENE_MODE_NONE ";
                  break;
          case 1: tmp = "CAM_SCENE_MODE_BACKLIGHT ";
                  break;
          case 2: tmp = "CAM_SCENE_MODE_BEACH_SNOW ";
                  break;
          case 3: tmp = "CAM_SCENE_MODE_CANDLE_LIGHT ";
                  break;
          case 4: tmp = "CAM_SCENE_MODE_DAWN_DUSK ";
                  break;
          case 5: tmp = "CAM_SCENE_MODE_FALL_COLORS ";
                  break;
          case 6: tmp = "CAM_SCENE_MODE_FIREWORKS  ";
                  break;
          case 7: tmp = "CAM_SCENE_MODE_LANDSCAPE  ";
                  break;
          case 8: tmp = "CAM_SCENE_MODE_NIGHT  ";
                  break;
          case 9: tmp = "CAM_SCENE_MODE_PARTY_INDOOR  ";
                  break;
          case 10: tmp = "CAM_SCENE_MODE_PORTRAIT  ";
                  break;
          case 11: tmp = "CAM_SCENE_MODE_SPORTS  ";
                  break;
          case 12: tmp = "CAM_SCENE_MODE_SUNSET  ";
                  break;
        }
        Serial.print("Test ");
        Serial.println(tmp);
        ret = set_scene_mode(i+1);
        if(ret != CAM_ERR_SUCCESS){
            return;
        }
        Serial.println();
        delay(1000);
    }
    Serial.println("Finish scene mode test");
}
#endif

#define COLOR_MODE_PARAM_COUNT 17

void color_mode_test()
{
    CamErr ret;
    int i;
    String tmp;
    String folder;

    Serial.println();

    //Start video streaming
    init_test();

    //Still param
    set_still_param(320,240,3);

    //Test color mode
    Serial.println("Start color mode test");
    for(i=0;i<COLOR_MODE_PARAM_COUNT;i++){
        switch(i)
        {
          case 0: tmp = "CAM_COLOR_FX_NONE ";
                  folder = "autotest/Test_16244_0";
                  break;
          case 1: tmp = "CAM_COLOR_FX_BW ";
                  folder = "autotest/Test_16244_1";
                  break;
          case 2: tmp = "CAM_COLOR_FX_SEPIA ";
                  folder = "autotest/Test_16244_2";
                  break;
          case 3: tmp = "CAM_COLOR_FX_NEGATIVE ";
                  folder = "autotest/Test_16244_3";
                  break;
          case 4: tmp = "CAM_COLOR_FX_EMBOSS ";
                  folder = "autotest/Test_16244_4";
                  break;
          case 5: tmp = "CAM_COLOR_FX_SKETCH ";
                  folder = "autotest/Test_16244_5";
                  break;
          case 6: tmp = "CAM_COLOR_FX_SKY_BLUE  ";
                  folder = "autotest/Test_16244_6";
                  break;
          case 7: tmp = "CAM_COLOR_FX_GRASS_GREEN  ";
                  folder = "autotest/Test_16244_7";
                  break;
          case 8: tmp = "CAM_COLOR_FX_SKIN_WHITEN  ";
                  folder = "autotest/Test_16244_8";
                  break;
          case 9: tmp = "CAM_COLOR_FX_VIVID  ";
                  folder = "autotest/Test_16244_9";
                  break;
          case 10: tmp = "CAM_COLOR_FX_AQUA  ";
                  folder = "autotest/Test_16244_10";
                  break;
          case 11: tmp = "CAM_COLOR_FX_ART_FREEZE  ";
                  folder = "autotest/Test_16244_11";
                  break;
          case 12: tmp = "CAM_COLOR_FX_SILHOUETTE  ";
                  folder = "autotest/Test_16244_12";
                  break;
          case 13: tmp = "CAM_COLOR_FX_SOLARIZATION  ";
                  folder = "autotest/Test_16244_13";
                  break;
          case 14: tmp = "CAM_COLOR_FX_ANTIQUE  ";
                  folder = "autotest/Test_16244_14";
                  break;
          case 15: tmp = "CAM_COLOR_FX_SET_CBCR  ";
                  folder = "autotest/Test_16244_15";
                  break;
          case 16: tmp = "CAM_COLOR_FX_PASTEL  ";
                  folder = "autotest/Test_16244_16";
                  break;
        }
        Serial.print("Test ");
        Serial.println(tmp);
        ret = set_color_mode(i+1);
        if( ret == CAM_ERR_SUCCESS ){
            //theSD.mkdir(folder);
            capture_test(folder);
    
            Serial.println();
            video_capture_folder = "autotest/Test_16233";
            video_capture_start = true;
            //delay(0);
            sleep(3);
        }
        

    }
   
    //Stop Video capture
    stop_video();
    
    if(i==COLOR_MODE_PARAM_COUNT){
        Serial.println("Finish color mode test");
    }
}

#define ISO_SENSITIVITY_PARAM_COUNT 15

void iso_sensitivity_test()
{
    CamErr ret;
    int i;
    int tmp;
    String folder;

    Serial.println();

    //Start video streaming
    init_test();

    //Still param
    set_still_param(320,240,3);

    //Test scene mode
    Serial.println("Start iso sensitivity test");
    for(i=0;i<ISO_SENSITIVITY_PARAM_COUNT;i++){
        switch(i)
        {
          case 0: tmp = 25;
                  folder = "autotest/Test_16242_0";
                  break;
          case 1: tmp = 50;
                  folder = "autotest/Test_16242_1";
                  break;
          case 2: tmp = 64;
                  folder = "autotest/Test_16242_2";
                  break;
          case 3: tmp = 80;
                  folder = "autotest/Test_16242_3";
                  break;
          case 4: tmp = 100;
                  folder = "autotest/Test_16242_4";
                  break;
          case 5: tmp = 125;
                  folder = "autotest/Test_16242_5";
                  break;
          case 6: tmp = 160;
                  folder = "autotest/Test_16242_6";
                  break;
          case 7: tmp = 200;
                  folder = "autotest/Test_16242_7";
                  break;
          case 8: tmp = 250;
                  folder = "autotest/Test_16242_8";
                  break;
          case 9: tmp = 320;
                  folder = "autotest/Test_16242_9";
                  break;
          case 10: tmp = 500;
                  folder = "autotest/Test_16242_10";
                  break;
          case 11: tmp = 640;
                  folder = "autotest/Test_16242_11";
                  break;
          case 12: tmp = 800;
                  folder = "autotest/Test_16242_12";
                  break;
          case 13: tmp = 1000;
                  folder = "autotest/Test_16242_13";
                  break;
          case 14: tmp = 1600;
                  folder = "autotest/Test_16242_14";
                  break;
          default: tmp = 0;
                  Serial.println("Error- Invalid parameter");
                  return;
        }
        Serial.print("Test ISO=");
        Serial.println(tmp);
        ret = set_iso_sensitivity(tmp);
        if(ret != CAM_ERR_SUCCESS){
            return;
        }
        else{
          //theSD.mkdir(folder);
          capture_test(folder);

          Serial.println();
  
          video_capture_folder = "autotest/Test_16231";
          video_capture_start = true;
  
          //delay(500);
          sleep(3);
        }

    }


    //Stop Video capture
    stop_video();

    if(i==ISO_SENSITIVITY_PARAM_COUNT){
        Serial.println("Finish iso sensitivity test");
    }

}

#define REPEAT_AISO_TEST_TIMES 10

void aiso_test(void)
{
    CamErr ret;
    int i;
    String tmp;
    String folder;
    
    Serial.println();
    //Test Auto ISO
    Serial.println("Start Auto ISO test");

    //Start video streaming
    init_test();

    //Still param
    set_still_param(320,240,3);

    //
    ret = set_aiso(true);
    folder = "autotest/Test_16243_0";
    if(ret != CAM_ERR_SUCCESS){
      Serial.println("set_aiso error");
      return;
    }
    else{
      //theSD.mkdir(folder);
      capture_test(folder);
    }
    Serial.println("Enable AISO");

    video_capture_folder = "autotest/Test_16232";
    video_capture_start = true;
    //delay(500);
    sleep(3);
    
    ret = set_aiso(false);
    folder = "autotest/Test_16243_1";
    if(ret != CAM_ERR_SUCCESS){
        Serial.println("set_aiso error");
        return;
    }
    else{
      //theSD.mkdir(folder);
      capture_test(folder);
    }

    Serial.println("Disable AISO");
    video_capture_folder = "autotest/Test_16232";
    video_capture_start = true;
    //delay(500);
    sleep(3);

    //Stop Video capture
    stop_video();


    Serial.println("Finish ISO test");

}

void msc_enable(void)
{
    Serial.println(" Wait 5sec.");
    delay(5000);     
    Serial.println(" Finished Waiting ");
}
#define CAPTURE_TIMES 10

void still_capture_test(char capture_type[])
{
    int i;
    CamErr ret;
    String folder;
    
    //Start video streaming
    init_test();


    Serial.println("");
    if (strcmp(capture_type, "qvga") == 0){
        Serial.println("Start QVGA still capture test");
        folder = "autotest/Test_16234";
        ret = set_still_param(320,240,3);
      }

    if (strcmp(capture_type, "vga") == 0){
        Serial.println("Start VGA still capture test");
        folder = "autotest/Test_16235";
        ret = set_still_param(640,480,3);
      }

    if (strcmp(capture_type, "hd") == 0){
        Serial.println("Start HD still capture test");
        folder = "autotest/Test_16237";
        ret = set_still_param(1280,720,3);
    }

    if (strcmp(capture_type, "quadvga") == 0){
        Serial.println("Start QuADVGA still capture test");
        folder = "autotest/Test_16236";
        ret = set_still_param(1280,960,3);  
    }


    if (strcmp(capture_type, "fullhd") == 0){
        Serial.println("Start FullHD still capture test");
        folder = "autotest/Test_16238";
        ret = set_still_param(1920,1080,3);
      }


    if (strcmp(capture_type, "5m") == 0){
        Serial.println("Start 5M still capture test");
        folder = "autotest/Test_16252";
        ret = set_still_param(2560,1920, 3);
      }


    if (strcmp(capture_type, "3m") == 0){
        Serial.println("Start 3M still capture test");
        folder = "autotest/Test_16253";
        ret = set_still_param(2048, 1536, 3);
      }



    if(ret == CAM_ERR_SUCCESS){
      //theSD.mkdir(folder);
      capture_test(folder);
    }
        
    //Stop Video capture
    stop_video();
    Serial.println("Still capture test end");
}

#define STILL_PARAM_TEST_TIMES 10

void still_param_test(void)
{
    int i;

    Serial.println("Set video param");
    set_video_param(320,240,60);

    for(i=0;i<STILL_PARAM_TEST_TIMES;i++){
       Serial.print("Set video param(QuadVGA)=");
       Serial.println(i);
       set_still_param(1280,960,3);
    }

    for(i=0;i<STILL_PARAM_TEST_TIMES;i++){
       Serial.print("Set video param(HD)=");
       Serial.println(i);
       set_still_param(1280,720,3);
    }

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
        video_capture_folder = "autotest/Test_16222";
        video_capture_start = true;
        //delay(500);
        sleep(3);
        stop_video();

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

int wait_command_input(void)
{
    CamErr ret_cam;
    int    ret;
    int arg_num = 0;
    int i;


    if ( Serial.available() > 0 ) {
        if(!theSD.exists("autotest")){
          //theSD.mkdir("autotest");
        }
        
        String str = Serial.readStringUntil('\r');

        arg_num = parse_args(str);
        if( arg_num == -1 ){
            Serial.println("error argument");
            return 0;
        }
        else{
            Serial.print("> ");
            for( i=0; i<arg_num; i++){
                Serial.print(args[i]);
                Serial.print(" ");
            }
            Serial.println();
        }

        Serial.println();
        if ( args[0] == "set_video_param") {
            if( arg_num == 4 ){
                int arg1,arg2,arg3;
                args[1].toCharArray(buf, MAX_BUF_LEN);
                arg1 = atoi(buf);
                args[2].toCharArray(buf, MAX_BUF_LEN);
                arg2 = atoi(buf);
                args[3].toCharArray(buf, MAX_BUF_LEN);
                arg3 = atoi(buf);
                ret_cam = set_video_param(arg1, arg2, arg3);
                Serial.println("Run: set_video_param");
            }
            else{
               Serial.println("Error invalid parameter");
            }
        }
        else if ( args[0] == "start_video" ) {
            ret_cam = start_video();
            Serial.println("Run: start_video");
        }
        else if ( args[0] == "stop_video" ) {
            ret_cam = stop_video();
            Serial.println("Run: stop_video");
        }
        else if ( args[0] == "set_still_param" ) {
            if( arg_num == 3 ){
                int arg1,arg2,arg3;
                args[1].toCharArray(buf, MAX_BUF_LEN);
                arg1 = atoi(buf);
                args[2].toCharArray(buf, MAX_BUF_LEN);
                arg2 = atoi(buf);
                ret_cam = set_still_param(arg1, arg2, 3);
                Serial.println("Run: capture");
            }
            else{
               Serial.println("Error invalid parameter");
            }
        }
        else if ( args[0] == "capture" ) {
            ret = capture();
            Serial.println("Run: capture");
        }
        else if( args[0] == "set_awb" ){
            if( arg_num == 2 ){
                int arg1;
                args[1].toCharArray(buf, MAX_BUF_LEN);
                arg1 = atoi(buf);
                if( arg1 == 0 ){
                  Serial.println("False");
                  ret_cam = set_awb(false);
                }
                else{
                  Serial.println("True");
                  ret_cam = set_awb(true);
                }

                Serial.println("Run: set_awb");
            }
            else{
               Serial.println("Error invalid parameter");
            }
        }
        else if( args[0] == "set_awb_mode" ){
            if( arg_num == 2 ){
                int arg1;
                args[1].toCharArray(buf, MAX_BUF_LEN);
                arg1 = atoi(buf);
                ret_cam = set_awb_mode(arg1);
                Serial.println("Run: set_awb_mode");
            }
            else{
               Serial.println("Error invalid parameter");
            }
        }
#if 0
         else if( args[0] == "set_ae" ){
            if( arg_num == 2 ){
                int arg1;
                args[1].toCharArray(buf, MAX_BUF_LEN);
                arg1 = atoi(buf);
                ret_cam = set_ae((bool)arg1);
                Serial.println("Run: set_ae");
            }
            else{
               Serial.println("Error invalid parameter");
            }
        }
#endif
         else if( args[0] == "set_iso_sensitivity" ){
            if( arg_num == 2 ){
                int arg1;
                args[1].toCharArray(buf, MAX_BUF_LEN);
                arg1 = atoi(buf);
                ret_cam = set_iso_sensitivity(arg1);
                Serial.println("Run: set_iso_sensitivity");
            }
            else{
               Serial.println("Error invalid parameter");
            }
        }
         else if( args[0] == "set_aiso" ){
            if( arg_num == 2 ){
                int arg1;
                args[1].toCharArray(buf, MAX_BUF_LEN);
                arg1 = atoi(buf);
                if( arg1 == 0 ){
                  Serial.println("False");
                  ret_cam = set_aiso(false);
                }
                else{
                  Serial.println("True");
                  ret_cam = set_aiso(true);
                }

                Serial.println("Run: set_aiso");
            }
            else{
               Serial.println("Error invalid parameter");
            }

        }
#if 0
        else if( args[0] == "set_scene_mode" ){
            if( arg_num == 2 ){
                int arg1;
                args[1].toCharArray(buf, MAX_BUF_LEN);
                arg1 = atoi(buf);
                ret_cam = set_scene_mode(arg1);
                Serial.println("Run: set_scene_mode");
            }
            else{
               Serial.println("Error invalid parameter");
            }
        }
#endif
        else if( args[0] == "set_color_mode" ){
            if( arg_num == 2 ){
                int arg1;
                args[1].toCharArray(buf, MAX_BUF_LEN);
                arg1 = atoi(buf);
                ret_cam = set_color_mode(arg1);
                Serial.println("Run: set_color_mode");
            }
            else{
               Serial.println("Error invalid parameter");
            }
        }
        else if ( args[0] == "init_test" ) {
            init_test();
            Serial.println("Run: init_test");
        }
        else if ( args[0] == "awb_mode_test" ) {
            awb_mode_test();
            Serial.println("Run: auto_test");
        }
#if 0
        else if ( args[0] == "scene_mode_test" ) {
            scene_mode_test();
            Serial.println("Run: auto_test");
        }
#endif
        else if( args[0] == "iso_test" ){
            iso_sensitivity_test();
            Serial.println("Run: iso_sensitivity_test");
        }
        else if( args[0] == "still_capture_test" ){
            if( arg_num == 2 ){
                int arg1;
                args[1].toCharArray(buf, MAX_BUF_LEN);

                still_capture_test(buf);
                Serial.println("Run: still_capture_test");
            }
            else{
               Serial.println("Error invalid parameter");
            }
            
        }
        else if( args[0] == "still_param_test" ){
            still_param_test();
            Serial.println("Run: still_param_test");
        }
        else if( args[0] == "color_mode_test" ){
            color_mode_test();
            Serial.println("Run: color_mode_test");
        }
        else if( args[0] == "video_capture_test" ){
            video_capture_test();
            Serial.println("Run: video_capture_test");
        }
        else if( args[0] == "awb_test" ){
            awb_test();
            Serial.println("Run: awb_test");
        }
        else if( args[0] == "aiso_test" ){
            aiso_test();
            Serial.println("Run: aiso_test");
        }
#if 1
        else if( args[0] == "msc_on" ){
            msc_enable();
            Serial.println("Finish: msc_enable");
        }

#endif
        else if( args[0] == "usbmsc" ){
            //Initalize MSC
            init_msc();
        }
        else if( args[0] == "end" ){
            fin_msc();
        }
        else {
            Serial.print("error [");
            Serial.print(str);
            Serial.println("]");
        }
    }
    return 1;
}
