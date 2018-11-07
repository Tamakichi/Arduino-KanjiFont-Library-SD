// フォントライブラリ利用サンプル
// 作成 2016/05/16 by Tamakichi
// 修正 2018/11/07 by Tamakichi,Serailインスタンスの接続待ち追加
//

#include <sdfonts.h>

//sdfonts ft;  // フォント管理オブジェクト

// ビットパターン表示
// d: 8ビットパターンデータ
//
void bitdisp(uint8_t d) {
  for (uint8_t i=0; i<8;i++) {
    if (d & 0x80>>i) 
      Serial.print("#");
    else
      Serial.print(".");
  }
}

// フォントデータの表示
// c(in) : フォントコード(UTF16コード)
//
void fontdisp(uint16_t c) {
  uint8_t buf[MAXFONTLEN]; 
  SDfonts.getFontData(buf,c);           // フォントデータの取得
  uint8_t bn= SDfonts.getRowLength();   // 1行当たりのバイト数取得
  
  Serial.print(SDfonts.getWidth(),DEC);
  Serial.print("x");      
  Serial.print(SDfonts.getHeight(),DEC);      
  Serial.print(" ");      
  Serial.println((uint16_t)c, HEX);  // UTF16コード表示

  for (uint8_t i = 0; i < SDfonts.getLength(); i += bn ) {
      for (uint8_t j = 0; j < bn; j++) {
        bitdisp(buf[i+j]);
      }
      Serial.println("");
  }
  Serial.println("");
} 

// フォントデータの表示
// buf(in) : フォント格納アドレス
//
void fontdisp2(uint8_t* buf) {
  uint8_t bn= SDfonts.getRowLength();                // 1行当たりのバイト数取得
  Serial.print(SDfonts.getWidth(),DEC);             // フォントの幅の取得
  Serial.print("x");      
  Serial.print(SDfonts.getHeight(),DEC);            // フォントの高さの取得
  Serial.print(" ");      
  Serial.println((uint16_t)SDfonts.getCode(), HEX); // 直前し処理したフォントのUTF16コード表示

  for (uint8_t i = 0; i < SDfonts.getLength(); i += bn ) {
      for (uint8_t j = 0; j < bn; j++) {
        bitdisp(buf[i+j]);
      }
      Serial.println("");
  }
  Serial.println("");
} 

//
// 指定した文字列を7種類のフォントで表示((UTF16文字列に事前変換)
//
void test1(char* pUTF8) {
  uint8_t buf[MAXFONTLEN];                  // フォントデータ格納アドレス(最大24x24/8 = 72バイト)
  uint16_t pUTF16[64];                      // UTF16文字列
  int n;

  SDfonts.open();                           // フォントのオープン
  n = sdfonts::Utf8ToUtf16(pUTF16, pUTF8);  // UTF8からUTF16に変換する

  // 7種類フォントサイズで表示
  for (uint8_t i=0; i < MAXSIZETYPE; i++) {
    SDfonts.setFontSizeAsIndex(i);          // フォントサイズの設定
    for (uint8_t j=0; j < n; j++) {
      SDfonts.getFontData(buf,pUTF16[j]);   // フォントデータの取得
      fontdisp2(buf);                       // フォントパターンの表示
    }
  }
  SDfonts.close();
}

//
// 指定した文字列を7種類のフォントで表示(UTF16事前変換無し)
//
void test2(char* pUTF8) {
  uint8_t buf[MAXFONTLEN];                         // フォントデータ格納アドレス(最大24x24/8 = 72バイト)
  char* str;
  
  // 7種類フォントサイズで表示
  SDfonts.open();                                  // フォントのオープン
  for (uint8_t i=0; i < MAXSIZETYPE; i++) {
    str = pUTF8; 
    SDfonts.setFontSizeAsIndex(i);                 // フォントサイズを種類番号で設定(0:8ドット～ 6:24ドット)
    while ( str = SDfonts.getFontData(buf, str) )  // フォントの取得
      fontdisp2(buf);                              // フォントパターンの表示
  }
  SDfonts.close();                                 // フォントのクローズ
}

void setup() {
  char *pUTF8 = "ABＡｱｲｳｴｵ１２Ａ＼￠￡§¨￢°±´¶×÷埼玉さいたま";
  Serial.begin(115200);                   // シリアル通信の初期化
  while(!Serial);
  Serial.println(F("sdfonts liblary"));

  SDfonts.init(10);                        // フォント管理の初期化

  test1(pUTF8);
  //test2(pUTF8);
  
}

void loop() {
}

