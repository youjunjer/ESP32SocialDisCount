#include <WiFi.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
//台灣社交安全距離APP的UUID
String SocialDistancingAPPUUID = "0000fd6f-0000-1000-8000-00805f9b34fb";
//RSSI範圍(距離偵測)
int RSSIthreshold = -100;
//用來紀錄符合的裝置MacAddress
String TotlaMacAddress = "";
int MacAddressCount = 0;
//每次掃描間隔
int scanTime = 15;
//上網設定
char ssid[] = "SSID";
char password[] = "Password";
//ThingSpeak網址
String url = "http://api.thingspeak.com/update?api_key=ThingSpeak API Key";

BLEScan* pBLEScan;
//每當掃描到藍芽裝置時的Callback
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      //列印UUID
      String UUID = advertisedDevice.getServiceDataUUID().toString().c_str();
      Serial.print("UUID:"); Serial.print(UUID);
      //列印RSSI
      int RSSI = advertisedDevice.getRSSI();
      Serial.print(",RSSI:"); Serial.print(RSSI);
      //列印Mac Address
      String Address = advertisedDevice.getAddress().toString().c_str();
      Serial.print(",Address:"); Serial.println(Address);
      //判斷是否符合條件
      if (UUID == SocialDistancingAPPUUID && RSSI >= RSSIthreshold) {
        if (TotlaMacAddress.indexOf(Address) < 0) {
          TotlaMacAddress = TotlaMacAddress + "," + Address;
          Serial.println("TotlaMacAddress=" + TotlaMacAddress);
          MacAddressCount++;
        }
      }
    }
};

void setup() {
  Serial.begin(115200);

  //設定藍芽掃描參數
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  //連線WiFi
  if (WiFi.status() != WL_CONNECTED) {
    ConnectWiFi();
  }
  //開始藍芽掃描
  Serial.println("Scan Start!");
  //清除上一次掃描資料
  TotlaMacAddress = "";
  MacAddressCount = 0 ;
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  pBLEScan->clearResults();//清除掃描結果
  Serial.println("Scan done!");
  //顯示掃描結果
  TotlaMacAddress = TotlaMacAddress.substring(1);
  Serial.print("TotlaMacAddress:"); Serial.println(TotlaMacAddress);
  Serial.print("Count:"); Serial.println(MacAddressCount);
  //將人數上傳ThingSpeak
  //上傳ThingSpeak
  HTTPClient http;
  //產生Get網址
  String url1 = url + "&field1=" + String(MacAddressCount);
  //http client取得網頁內容
  http.begin(url1);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK)      {
    //讀取網頁內容到payload
    String payload = http.getString();
    //將內容顯示出來
    Serial.print("網頁內容=");
    Serial.println(payload);
  } else {
    //讀取失敗
    Serial.println("網路傳送失敗");
  }
  http.end();

  delay(1000);
}

void ConnectWiFi() {
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int WiFicount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    //20秒內連不上就重新開機
    if (WiFicount > 40) {ESP.restart();} else { WiFicount++; }
  }
  //連線成功，顯示取得的IP
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

}
