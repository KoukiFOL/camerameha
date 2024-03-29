/*
cameramehaに実装するC++コード
*/

#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_VC0706.h>
//#include <Adafruit_MPU6050.h>
//#include <Adafruit_Sensor.h>

// カメラモジュールのシリアルポートの定義
SoftwareSerial cameraSerial(2, 3);

// Bluetoothモジュールのシリアルポートの定義
SoftwareSerial blueToothSereal(5, 6);

SoftwareSerial sensorSerial(8, 9);

// カメラのオブジェクト生成
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraSerial);

//加速度センサの諸々の定義
const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

void setup() {
  // arduinoシリアル通信の開始
  Serial.begin(9600);

  /*
    加速度センサーのセット
    */

  sensorSerial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  sensorSerial.begin(9600);


  /*
    カメラモジュールのセット
  */

  // カメラモジュールのシリアル通信開始
  cameraSerial.begin(38400);

  if (cam.begin()) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    return;
  }

  takePicture();
}


void loop() {
}

//写真を撮影する関数
void takePicture() {
  // カメラのバージョンの取得
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }
  // 画像サイズの設定
  delay(500);
  cam.setImageSize(VC0706_160x120);

  // 設定された画像サイズの出力
  uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) {
    Serial.println("640x480");
  }
  if (imgsize == VC0706_320x240) {
    Serial.println("320x240");
  }
  if (imgsize == VC0706_160x120) {
    Serial.println("160x120");
  }

  // BlueToothモジュールのセット
  //blueToothSereal.begin(11520);

  Serial.println("Wait 3 sec...");

  // 撮影
  if (!cam.takePicture()) {
    Serial.println("Failed to snap!");
  } else {
    Serial.println("Picture taken!");

    //画像をシリアル通信でpcに送信
    Serial.println("--Binary Data Start--");
    uint16_t jpglen = cam.frameLength();
    Serial.write((jpglen >> 8) & 0xFF);
    Serial.write(jpglen & 0xFF);

    byte wCount = 0;
    
    Serial.println("");
    while (jpglen > 0) {
      uint8_t *buffer;
      uint8_t bytesToRead = min((uint16_t)32, jpglen);  // change 32 to 64 for a speedup but may not work with all setups!
      buffer = cam.readPicture(bytesToRead);
      Serial.write(buffer, bytesToRead);
      if (++wCount >= 64) {  // Every 2K, give a little feedback so it doesn't appear locked up
        Serial.print('.');
        wCount = 0;
      }
      jpglen -= bytesToRead;
    }
    Serial.println("");
    Serial.println("--Binary Data End--");
    

  }
}