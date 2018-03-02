# 開後不理抽風機
一個能偵測/紀錄溫溼度、自動關機的簡易抽風系統，讓你擺脫浴室裡面長香菇的惡夢   
   
 ![開後不理抽風機][closeup]   
   
## 功能特色
* 避免使用浴室內的AC插座   
* 電路精簡，可裝入防水保鮮盒   
* 搭配低電流自動斷電的行動電源，達到自動斷電效果   
* 內建 EEPROM Datalogger ，紀錄並修改成適合環境的斷電閥值   
* 使用容易取得的 LM2577 升壓模組和 12V 電腦風扇即可運轉   
* 使用 DHT11 偵測溫溼度，露點溫度轉換，自由選擇偵測/紀錄的參數
* 使用短路 PIN 設定開機模式   
  ( 正常模式 / 序列偵錯輸出 / EEPROM讀取 / EEPROM清除 / 資料紀錄模式 )   
  詳細設定請見參考電路圖   
   
   
**預設軟體參數**
   
|       規格      |        參數       |  單位  | 備註                  |
|-----------------|:-----------------:|:------:|-----------------------|
|   Arduino板子   |   Pro Mini 16MHz  |   -    |   8MHz未測            |
|   風扇更新週期  |   4               |   秒   |   timer1  ISR         |
|   資料記錄週期  |   60              |   秒   |   timer1  ISR*15      |
|   DHT取樣週期   |   1.5             |   秒   |   需大於1秒           |
|   樣本平均量    |   10              | 個樣本 |   平均後做判斷/紀錄   |
   
   
**待新增功能**
* 一個會自動睡眠的 I2C OLED (128*32)   
* 一個能穿透保鮮盒的電容式觸控按鈕 / 滑條輸入   
* 一顆不吵的蜂鳴器   
* 一個簡易到連猴子都會用的介面   
   
   
## 製作說明   

* 電路圖 :   
 ![電路圖][sch]   
   
* 硬體製作 :   
 照著 [電路圖][sch-pdf] 和 [照片][photos] 做就對了   
   
* 軟體燒錄 :   
 [/software/AutoExhaustFan][sw] 資料夾內的程式碼使用 Arduino IDE 開啟  
  - 安裝所使用的外部函式庫  
     - [草稿碼] → [匯入程式庫] → [管理程式庫]中輸入 DHT 搜尋  
     - 選擇 DHT Sensor library by adafruit 按下 [安裝] 按鈕  
     - 若要手動匯入外部程式庫請至 [Arduino Playground - DHT11Lib ][libs] 下載   
      
  安裝相對應的程式庫之後即可編譯上傳     
  初次使用請務必進入資料紀錄模式並且修改適合環境的斷電閥值  
  
  
*** 
  
*Read this in other languages: [English](README.en.md), [正體中文](README.md).*  


   [sch]:       <https://rawgit.com/dcadc/Automatic-Exhaust-Fan/master/images/sch.png>
   [sch-pdf]:   <https://rawgit.com/dcadc/Automatic-Exhaust-Fan/master/hardware/AutoExhaustFan.pdf>
   [photos]:    <https://github.com/dcadc/Automatic-Exhaust-Fan/tree/master/images/photos>
   [closeup]:   <https://rawgit.com/dcadc/Automatic-Exhaust-Fan/master/images/photos/full_set.JPG>
   [sw]:        <https://github.com/dcadc/Automatic-Exhaust-Fan/tree/master/software/AutoExhaustFan>
   [libs]:      <https://playground.arduino.cc/Main/DHT11Lib>
