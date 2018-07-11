//
// フォントライブラリ利用サンプル
// 作成 2018/07/10 by たま吉さん
//

#include <sdfonts.h>
#include <Adafruit_GFX_AS.h>     
#include <Adafruit_ILI9341_STM.h>
#include <XPT2046_touch.h>

// TFT制御用ピン
#define TFT_CS  PA0
#define TFT_RST PA1
#define TFT_DC  PA2

// タッチスクリーンCSピン
#define TS_CS  PA3 

// タッチスクリーン領域
#define TS_MINX 600
#define TS_MINY 440
#define TS_MAXX 3460
#define TS_MAXY 3460

#define MY_SPIPORT  2   // SPIポートの指定 1:SPI ,2:SPI2

//
// ※SdFatを使う場合は、sdfontsConfig.hのSDFONTS_USE_SDFATに1を設定し、
//   SdFatまたは、SdFatEX型のグローバルオブジェクトSDを用意すること
//

// 利用するSDオブジェクトの定義
#if SDFONTS_USE_SDFAT == 1
  #include <SdFat.h>
  #if ENABLE_EXTENDED_TRANSFER_CLASS == 1
    SdFatEX  SD(MY_SPIPORT);
  #else
    SdFat    SD(MY_SPIPORT);  
  #endif
#else
  #include <SD.h>
#endif

SPIClass  SPI_2(2); // タッチスクリーン用SPI

// タッチスクリーン制御用
XPT2046_touch ts(TS_CS, SPI_2); // Chip Select pin, SPI port

// TFT制御用
SPIClass  SPI_1(1);
Adafruit_ILI9341_STM tft = Adafruit_ILI9341_STM(TFT_CS, TFT_DC, TFT_RST,SPI_1);

// スクロールアップ
void scrollUp(uint16_t y0,uint16_t h,uint16_t bg) {
  uint16_t sc_w = tft.width();
  uint16_t sc_h = tft.height();
  uint16_t buf[sc_w];

  for (uint16_t y=h+y0; y < sc_h; y++) {  
    tft.readPixels(0, y, sc_w-1, y, buf);
    tft.setAddrWindow(0, y-h, sc_w-1, y-h);
    tft.pushColors(buf, sc_w, 0);
  }
  tft.fillRect(0, sc_h-h, sc_w-1, h, bg);
}

// 指定位置に１文字表示
void mputc(uint16_t x, uint16_t y, uint8_t* buf, uint16_t fg, uint16_t bg) {   
  uint16_t w = SDfonts.getWidth();
  uint16_t h = SDfonts.getHeight();
  int16_t byteWidth = (w + 7)>>3;
  uint8_t byte = 0;

  // フォントの描画
  tft.setAddrWindow(x, y, x+w-1, y+h-1); // 描画領域の設定
  for(int16_t j=0; j<h; j++, y++) {
    for(int16_t i=0; i<w; i++) {
      byte = (i & 7) ? byte<<1 : buf[j * byteWidth + (i>>3)];
      tft.pushColor((byte & 0x80) ? fg : bg);
    }
  }
}

// 指定位置に文字列表示
void mprint(uint16_t x, uint16_t y, char* str, uint16_t fg, uint16_t bg) { 
  uint8_t buf[MAXFONTLEN]; 
  int16_t   len,x0 = x, y0 = y;
  char* pUTF8 = str;

  SDfonts.open();   // フォントのオープン
  while ( pUTF8 = SDfonts.getFontData(buf, pUTF8) ) {  // フォントの取得
    mputc(x, y, buf ,fg, bg);
    if (x + SDfonts.getWidth()*2 < tft.width()) {
      x += SDfonts.getWidth()+1;  
    } else {  
      x = x0;
      if (y + SDfonts.getHeight()*2 < tft.height()) {
        y += SDfonts.getHeight()+2;
      } else {  
        scrollUp(y0, SDfonts.getHeight()+2, bg);
      }
    }
  }      
  SDfonts.close();   // フォントのクローズ
}

void setup() {
  Serial.begin(115200);
  delay(1000);
#if SDFONTS_USE_SDFAT == 0
  SPI.setModule(2);
#endif

  // フォント管理の初期化
  if(!SDfonts.init(PB0)) {                
    Serial.println(F("sdfonts init error"));
    exit(1);
  }
  Serial.println(F("test sdfonts liblary"));
  
  ts.begin();
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK); 
}

static const char* text =
   "吾輩は猫である。名前はまだ無い。"
   "どこで生れたかとんと見当がつかぬ。"
   "何でも薄暗いじめじめした所でニャーニャー泣いていた事だけは記憶している。"
   "吾輩はここで始めて人間というものを見た。"
   "しかもあとで聞くとそれは書生という人間中で一番獰悪な種族であったそうだ。"
   "というのは時々我々を捕つかまえて煮にて食うという話である。"
   "しかしその当時は何という考もなかったから別段恐しいとも思わなかった。"
   "ただ彼の掌のひらに載せられてスーと持ち上げられた時何だかフワフワした感じがあったばかりである。"
   "掌の上で少し落ちついて書生の顔を見たのがいわゆる人間というものの見始めであろう。"
 ;

void loop() {
  //7種類のフォントサイズで文字列表示
  for (uint8_t i =0 ; i <7; i++) {
    tft.fillScreen(ILI9341_BLACK);
    SDfonts.setFontSizeAsIndex(i);
    mprint(2, 2, (char*)text, ILI9341_WHITE, ILI9341_BLACK);
/*
    for (uint8_t j=0; j < 5; j++) {
      scrollUp(2,SDfonts.getHeight()+2,ILI9341_BLACK);
    }
*/
    // テキスト表示後、3秒間タッチスクリーンを使って描画出来る
    // (SDfontsとタッチスクリーンのSPIバス共有利用の動作確認)
    uint32_t t = millis() + 3000;
    while(millis() < t) {
      TS_Point p = ts.getPoint();
      p.x = tft.width() - map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
      p.y = tft.height() - map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

      if (p.x >=0 && p.x < tft.width() && p.y >=0 &&  p.y < tft.height()) {
         tft.fillCircle(p.x, p.y, 3, ILI9341_RED);
      }       
    }
  }
}
