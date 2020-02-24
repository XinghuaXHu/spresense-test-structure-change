/*
 *  number_recognition.ino - hand written number recognition sample application
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file number_recognition.ino
 * @author Sony Semiconductor Solutions Corporation
 * @brief DNNRT sample application.
 *
 * This sample uses the network model file (.nnb) and recognize image
 * in pgm (portable greyscale map) file. Both of requred files should be
 * placed at the SD card. And adjust file path (nnbfile and pgmfile) if
 * needed.
 */

#include <SDHCI.h>
#include <NetPBM.h>
#include <DNNRT.h>

DNNRT dnnrt;
SDClass SD;

#define INPUT_MAX   1
#define OUTPUT_MAX  1
#define IN_DIM_MAX  4
#define OUT_DIM_MAX 2

/* each expected value */
#define LENET_INPUT_NUM    1
#define LENET_INPUT_SZ     784
#define LENET_INPUT_NDIM   IN_DIM_MAX 
#define LENET_INPUT_SHP0   1
#define LENET_INPUT_SHP1   1
#define LENET_INPUT_SHP2   28
#define LENET_INPUT_SHP3   28

#define LENET_OUTPUT_NUM   1
#define LENET_OUTPUT_SZ    10
#define LENET_OUTPUT_NDIM  OUT_DIM_MAX
#define LENET_OUTPUT_SHP0  1
#define LENET_OUTPUT_SHP1  10

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  File nnbfile = SD.open("network.nnb");
  if (!nnbfile) {
    Serial.print("nnb not found");
    return;
  }
  int ret = dnnrt.begin(nnbfile);
  if (ret < 0) {
    Serial.print("Runtime initialization failure. ");
    Serial.print(ret);
    Serial.println();
    return;
  }
  /* check some parameters at lenet by CSD  */
  int Inval;
  int Outval;
  int InDimVal;
  int InShapeVal[IN_DIM_MAX];
  int OutDimVal;
  int OutShapeVal[OUT_DIM_MAX];
  int Insz;
  int Outsz;  
  int i;

  printf("param check start !\n");
  
  Inval = dnnrt.numOfInput();
  if(Inval != LENET_INPUT_NUM) {
    printf("param error(numOfInput) %d is not %d \n", Inval, LENET_INPUT_NUM);
    goto end;
  }else{
    printf("param OK (numOfInput) %d \n", Inval);    
  }
  
  Outval = dnnrt.numOfOutput();
  if(Outval != LENET_OUTPUT_NUM) {
    printf("param error(numOfOutput) %d is not %d \n", Outval, LENET_OUTPUT_NUM);
    goto end;
  }else{
    printf("param OK (numOfOutput) %d \n", Outval);    
  }

  /* For Input */  
  InDimVal = dnnrt.inputDimension(0);
  if(InDimVal != LENET_INPUT_NDIM) {
    printf("param error(inputDimension) %d is not %d \n", InDimVal, LENET_INPUT_NDIM);
    goto end;
  }else{
    printf("param OK (inputDimension) %d \n",InDimVal); 
  }

  for(i=0; i<IN_DIM_MAX; i++) {
    InShapeVal[i]= dnnrt.inputShapeSize(0, i);
    switch (i){
        case 0:
          if(InShapeVal[i] != LENET_INPUT_SHP0){
            printf("param error(inputShapeSize [%d]) %d is not %d \n", i, InShapeVal[i], LENET_INPUT_SHP0);
            goto end;
          }else{
            printf("param OK (inputShapeSize [%d]) %d \n", i, InShapeVal[i]);                     
          }
          break;
        case 1:
          if(InShapeVal[i] != LENET_INPUT_SHP1){
            printf("param error(inputShapeSize [%d]) %d is not %d \n", i, InShapeVal[i], LENET_INPUT_SHP1);
            goto end;
          }else{
            printf("param OK (inputShapeSize [%d]) %d \n", i, InShapeVal[i]);                     
          }
          break;
        case 2:
          if(InShapeVal[i] != LENET_INPUT_SHP2){
            printf("param error(inputShapeSize [%d]) %d is not %d \n", i, InShapeVal[i], LENET_INPUT_SHP2);
            goto end;
          }else{
            printf("param OK (inputShapeSize [%d]) %d \n", i, InShapeVal[i]);                     
          }
          break;
        case 3:
          if(InShapeVal[i] != LENET_INPUT_SHP3){
            printf("param error(inputShapeSize [%d]) %d is not %d \n", i, InShapeVal[i], LENET_INPUT_SHP3);
            goto end;
          }else{
            printf("param OK (inputShapeSize [%d]) %d \n", i, InShapeVal[i]);                     
          }
          break;
        default:
          printf("Error input value:%d(range over) \n",i);
          goto end;       
    }
  }

  Insz = dnnrt.inputSize(0);
  if(Insz != LENET_INPUT_SZ) {    
    printf("param error(inputSize) %d is not %d \n", Insz, LENET_INPUT_SZ);
    goto end;         
  }else{
    printf("param OK (inputSize) %d \n",Insz); 
  }

  
  /* For Output */  
  OutDimVal = dnnrt.outputDimension(0);
  if(OutDimVal != LENET_OUTPUT_NDIM) {
    printf("param error(outputDimension) %d is not %d \n", OutDimVal, LENET_OUTPUT_NDIM);
    goto end;
  }else{
    printf("param OK (outputDimension) %d \n",OutDimVal); 
  }
  
  for(i=0; i<OUT_DIM_MAX; i++) {
    OutShapeVal[i]= dnnrt.outputShapeSize(0,i);
    switch (i){
        case 0:
          if(OutShapeVal[i] != LENET_OUTPUT_SHP0){
            printf("param error(outputShapeSize [%d]) %d is not %d \n", i, OutShapeVal[i], LENET_OUTPUT_SHP0);
            goto end;
          }else{
            printf("param OK (inputShapeSize [%d]) %d \n", i, OutShapeVal[i]);                     
          }          
          break;
        case 1:
          if(OutShapeVal[i] != LENET_OUTPUT_SHP1){
            printf("param error(outputShapeSize [%d]) %d is not %d \n", i, OutShapeVal[i], LENET_OUTPUT_SHP1);
            goto end;
          }else{
            printf("param OK (inputShapeSize [%d]) %d \n", i, OutShapeVal[i]);                     
          }          
          break;
        default:
          printf("Error output value:%d(range over) \n",i);
          goto end;       
    }
  }
 
  Outsz = dnnrt.outputSize(0);
  if(Outsz != LENET_OUTPUT_SZ) {    
    printf("param error(outputSize) %d is not %d \n", Outsz, LENET_OUTPUT_SZ);
    goto end;
  }else{
    printf("param OK (outputSize) %d \n",Outsz); 
  }
  printf("Test Finished Successfully! \n");

#if 0
  // Image size for this network model is 28 x 28.

  File pgmfile("number4.pgm");
  NetPBM pgm(pgmfile);

  unsigned short width, height;
  pgm.size(&width, &height);

  DNNVariable input(width * height);
  float *buf = input.data();
  int i = 0;

  /*
   * Normalize pixel data into between 0.0 and 1.0.
   * PGM file is gray scale pixel map, so divide by 255.
   * This normalization depends on the network model.
   */

  for (int x = 0; x < height; x++) {
    for (int y = 0; y < width; y++) {
      buf[i] = float(pgm.getpixel(x, y)) / 255.0;
      i++;
    }
  }

  dnnrt.inputVariable(input, 0);
  dnnrt.forward();
  DNNVariable output = dnnrt.outputVariable(0);

  /*
   * Get index for maximum value.
   * In this example network model, this index represents a number,
   * so you can determine recognized number from this index.
   */

  int index = output.maxIndex();
  Serial.print("Image is ");
  Serial.print(index);
  Serial.println();
  Serial.print("value ");
  Serial.print(output[index]);
  Serial.println();

#endif
end:
  dnnrt.end();
}

void loop() {
  // put your main code here, to run repeatedly:

}
