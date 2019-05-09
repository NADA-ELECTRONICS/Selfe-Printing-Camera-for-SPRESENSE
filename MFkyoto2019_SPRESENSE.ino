/*
    重要！！！ https://developer.sony.com/develop/spresense/developer-tools/api-reference/api-references-arduino/Camera_8h.html
    camera.ino - One minute interval time-lapse Camera
    Copyright 2018 Sony Semiconductor Solutions Corporation

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    This is a test app for the camera library.
    This library can only be used on the Spresense with the FCBGA chip package.
*/

#include <Camera.h>
#include "Adafruit_ILI9341.h"
#include "ssci.h"

/* ili9341 */
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_MISO 12
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
/* Serial1 */
#define BAUDRATE  (115200)
/* tilt function */
int tiltpin = 4;
int tiltmode = 2;
/* camera shutter */
int shutterpin = 2;
int CamShutterFlg = 0;
/* image buffer */
int myImageFlg = 0;
uint8_t myImage[320 * 240];

/**
  @brief image to AS-289R2 Thermal Printer Shield
  @param image Data to be printed
  @param imgWidth Image data width size
  @param imgWidth Image data height size
*/
void ImgToAS289R2(uint8_t *image, uint16_t imgWidth, uint16_t imgHeight)
{
  Serial2.print("\x1C\x2A\x65");  // AS-289R2 CMD
  Serial2.write((uint8_t)(imgHeight / 256));
  Serial2.write((uint8_t)(imgHeight % 256));
  for (int iy = 0 ; iy < imgHeight; iy++)
  {
    for (int ix = 0; ix < 48; ix ++)
    {
      uint8_t pixel8;
      if (ix >= 4 && ix < 44)
      {
        pixel8 = 0;
        for (int ib = 0; ib < 8; ib ++) {
          uint8_t pixel = image[(iy * imgWidth) + ((ix - 4) * 8) + ib] ^ 0xFF;
          pixel8 <<= 1;
          if (pixel && 0xFF)
          {
            pixel8 |= 1;
          }
        }
      }
      else {
        //pixel8 = 0;
        //ssci logo
        pixel8 = ssci_image[iy * 48 + ix] ^ 0xFF;
      }
      Serial2.write(pixel8);
    }
  }
}

/**
  @brief Dithering Algorithm
*/
uint8_t saturated_add( uint8_t val1, int8_t val2 )
{
  int16_t val1_int = val1;
  int16_t val2_int = val2;
  int16_t tmp = val1_int + val2_int;
  if ( tmp > 255 )
  {
    return 255;
  }
  else if ( tmp < 0 )
  {
    return 0;
  }
  else
  {
    return tmp;
  }
}

void Dithering( uint8_t *image, uint16_t imgWidth, uint16_t imgHeight )
{
  //  uint8_t *image = img.getImgBuff();
  //  int imgWidth = 320;
  //  int imgHeight = 240;
  int err;
  int8_t a, b, c, d;

  for ( int y = 0; y < imgHeight - 1; y ++ )
  {
    for ( int x = 0; x < imgWidth - 1; x++ )
    {
      if ( image[(y * imgWidth) + x ] > 127 )
      {
        err = image[(y * imgWidth) + x ]  - 255;
        image[(y * imgWidth) + x ] = 255;
      }
      else
      {
        err = image[(y * imgWidth) + x ] - 0;
        image[(y * imgWidth) + x ] = 0;
      }
      a = ( err * 7 ) / 16;
      b = ( err * 1 ) / 16;
      c = ( err * 5 ) / 16;
      d = ( err * 3 ) / 16;
      if ( ( y != ( imgHeight - 1 ) ) && ( x != 0 ) && ( x != ( imgWidth - 1 ) ) )
      {
        image[(y * imgWidth) + x + 1] = saturated_add( image[(y * imgWidth) + x + 1], a );
        image[((y + 1) * imgWidth) + x + 1] = saturated_add( image[((y + 1) * imgWidth) + x + 1], b );
        image[((y + 1) * imgWidth) + x] = saturated_add( image[((y + 1) * imgWidth) + x], c );
        image[((y + 1) * imgWidth) + x - 1] = saturated_add( image[((y + 1) * imgWidth) + x - 1], d );
      }
    }
  }
}

/**
  @brief Callback from Camera library when video frame is captured.
*/
void CamCB(CamImage img)
{
  /* Check the img instance is available or not. */
  if (img.isAvailable())
  {
    if (CamShutterFlg == 0)
    {
      img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
      tft.drawRGBBitmap(0, 0, (uint16_t *)img.getImgBuff(), img.getWidth(), img.getHeight() );
    }
    else
    {
      img.convertPixFormat(CAM_IMAGE_PIX_FMT_GRAY);
      uint8_t *buf = img.getImgBuff();
      for (int ii = 0; ii < 320 * 240; ii ++)
      {
        myImage[ii] = buf[ii];
      }
      CamShutterFlg = 0;
      myImageFlg = 1;
      /* LCD Gray */
      uint16_t color = 0;
      tft.setAddrWindow(0, 0, 320, 240);
      for (int ii = 0; ii < 320 * 240; ii ++)
      {
        //color = myImage[ii * 2 + 1] * 256 + myImage[ii * 2];
        //color = (myImage[ii * 2 + 1] << 8) | myImage[ii * 2 ];
        color = (myImage[ii] & 0xF8) * 256;   //R
        color += (myImage[ii] >> 5) * 256;    //G
        color += (myImage[ii] & 0x1C) << 3;   //G
        color += (myImage[ii] >> 3);          //B
        tft.pushColor(color);
      }
    }
  }
}

/**
  @brief Tilt Function
*/
void tiltFunction()
{
  int tiltval = digitalRead(tiltpin);
  if (tiltmode != tiltval &&  tiltval == 0)
  {
    tft.setRotation(3);
  }
  else if (tiltmode != tiltval &&  tiltval == 1)
  {
    tft.setRotation(1);
  }
  tiltmode = tiltval;
}

/**
  @brief Interrupt routine
*/
void shutterState()
{
  if (CamShutterFlg == 0 && myImageFlg == 0)
  {
    CamShutterFlg = 1;
  }
}

/**
   @brief Setup
*/
void setup()
{
  /* Open serial communications and wait for port to open */
  Serial.begin(BAUDRATE);
  while (!Serial) { }
  Serial2.begin(57600);
  Serial2.print("\r");
  Serial2.print("Power ON.\r\r");
  Serial2.print("いぶし銀カメラ\r");
  Serial2.print("Spresense\r");
  Serial2.print("Spresenseカメラボード\r");
  Serial2.print("Spresense拡張ボード\r");
  Serial2.print("Thermal Printer Shield\r");
  Serial2.print("\r\r\r\r\r\r\r");
  /*  tilt functon */
  pinMode(tiltpin, INPUT);
  /*  shutter button */
  attachInterrupt(shutterpin, shutterState, FALLING);
  /* LCD */
  tft.begin();
  tiltFunction();
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(2);
  tft.setCursor(30, 100);
  tft.println("Maker Faire Kyoto 2019");
  /* CAMERA */
  theCamera.begin();
  theCamera.setAutoISOSensitivity(true);
  theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);  // Auto white balance configuration
  //theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_FLUORESCENT); // Auto white balance configuration
  theCamera.startStreaming(true, CamCB);  // Start video stream.
}

/**
   @brief Loop
*/
void loop()
{
  tiltFunction();
  if (CamShutterFlg == 0 && myImageFlg == 1) {
    Dithering(myImage, 320, 240);
    Serial2.print("\r");
    Serial2.print("    Maker Faire 京都 2019\r\r");
    Serial2.print("  Thank you for coming today\r\r");
    Serial2.print("  ご来場、ありがとうございます\r\r");
    Serial2.print("        ＃MFKyoto2019\r\r");
    ImgToAS289R2(myImage, 320, 240);
    Serial2.print("\rけいはんなｵｰﾌﾟﾝｲﾉﾍﾞｰｼｮﾝｾﾝﾀｰ\r");
    Serial2.print("2019/05/04(Sat)-05(Sun)\r");
    Serial2.print("\r\r\r\r\r\r\r");
    myImageFlg = 0;
  }

}

