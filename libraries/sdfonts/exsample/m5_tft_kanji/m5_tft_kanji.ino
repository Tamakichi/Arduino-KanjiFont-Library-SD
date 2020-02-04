//
// M5Stack フォント表示サンプル
// 作成 2020/02/04 by たま吉さん
//

#include <M5Stack.h>
#include <sdfonts.h>
#define SD_PN 4

// スクロールアップ（M5stackでは動かない）
void scrollUp(uint16_t y0,uint16_t h,uint16_t bg) {
  uint16_t sc_w = M5.Lcd.width();
  uint16_t sc_h = M5.Lcd.height();
  uint16_t buf[sc_w];

  for (uint16_t y=h+y0; y < sc_h; y++) {  
    M5.Lcd.readRect(0, y,  sc_w,1,buf);
    M5.Lcd.pushRect(0, y-h,sc_w,1,buf);
  }
  M5.Lcd.fillRect(0, sc_h-h, sc_w-1, h, bg);
}

// 指定位置に１文字表示
void mputc(uint16_t x, uint16_t y, uint8_t* buf, uint16_t fg, uint16_t bg) {   
  uint16_t w = SDfonts.getWidth();
  uint16_t h = SDfonts.getHeight();
  uint16_t byteWidth = (w + 7)>>3;
  uint8_t byte = 0;

  // フォントの描画
  M5.Lcd.setAddrWindow(x, y, w, h);  // 描画領域の設定
  for(int16_t j=0; j<h; j++, y++) {
    for(int16_t i=0; i<w; i++) {
      byte = (i & 7) ? byte<<1 : buf[j * byteWidth + (i>>3)];
      M5.Lcd.pushColor((byte & 0x80) ? fg : bg);
    }
  }
}

// 指定位置に文字列表示
void mprint(uint16_t x, uint16_t y, char* str, uint16_t fg, uint16_t bg) { 
  uint8_t buf[MAXFONTLEN]; 
  int16_t   len,x0 = x, y0 = y;
  char* pUTF8 = str;

  SDfonts.open();   // フォントのオープン
  while ( (pUTF8 = SDfonts.getFontData(buf, pUTF8)) ) {  // フォントの取得
    mputc(x, y, buf ,fg, bg);
    if (x + SDfonts.getWidth()*2 < 320) {
      x += SDfonts.getWidth()+1;  
    } else {  
      x = x0;
      if (y + SDfonts.getHeight()*2 < 240) {
        y += SDfonts.getHeight()+2;
      } else {  
        //scrollUp(y0, SDfonts.getHeight()+2, bg);
        y = y0;
        M5.Lcd.fillScreen(TFT_BLACK);
      }
    }
  }      
  SDfonts.close();   // フォントのクローズ
}

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.fillScreen(TFT_BLACK);
  delay(1000);

  // フォント管理の初期化
  if(!SDfonts.init(SD_PN)) {                
    Serial.println(F("sdfonts init error"));
    exit(1);
  }
  Serial.println(F("test sdfonts liblary"));
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
    M5.Lcd.fillScreen(TFT_BLACK);
    SDfonts.setFontSizeAsIndex(i);
    mprint(0, 0, (char*)text, TFT_WHITE, TFT_BLACK);
    delay(3000);
  }
}
