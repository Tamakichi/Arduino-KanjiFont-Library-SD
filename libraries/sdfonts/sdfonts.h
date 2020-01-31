//
// フォント利用ライブラリ クラス定義 sdfonts.h 
// 作成 2016/05/13 by たま吉さん
// 修正 2016/05/16 by たま吉さん, インスタンスをグローバル変数化
// 修正 2016/05/19 by たま吉さん, グラフィック液晶用フォントモードの追加(setLCDMode()関数追加)
// 修正 2016/06/26 by たま吉さん, ESP8266対応(ARDUINO_ARCH_AVRの判定追加),read_code()の不具合対応
// 修正 2018/07/10 by たま吉さん, sdfat対応,Utf8ToUtf16()の戻り値型をint16_tに修正
// 修正 2020/01/31 by たま吉さん, ESP32対応
//

#ifndef ___sdfonts_h___
#define ___sdfonts_h___

#include "sdfontsConfig.h"
#include <Arduino.h>
#include <SPI.h>

#if SDFONTS_USE_SDFAT == 1
  #include <SdFat.h>
  #if ENABLE_EXTENDED_TRANSFER_CLASS == 1
    #define MYSDCLASS SdFatEX
  #else
    #define MYSDCLASS SdFat
  #endif
#else
  #if defined(ARDUINO_ARCH_ESP32)
    #include <SD.h>
    #define MYSDCLASS SDFS
  #else
    #include <SD.h>
    #define MYSDCLASS SDClass
  #endif
#endif


#define EXFONTNUM  14    // 登録フォント数
#define FULL_OFST   7    // フォントサイズから全角フォント種類変換用オフセット値
#define MAXFONTLEN  72   // 最大フォントバイトサイズ(=24x24フォント)
#define MAXSIZETYPE 7    // フォントサイズの種類数

// フォントサイズ
#define  EXFONT8    0    // 8ドット美咲フォント
#define  EXFONT10   1    // 10ドット nagaフォント
#define  EXFONT12   2    // 12ドット東雲フォント
#define  EXFONT14   3    // 14ドット東雲フォント
#define  EXFONT16   4    // 16ドット東雲フォント
#define  EXFONT20   5    // 20ドットJiskanフォント
#define  EXFONT24   6    // 24ドットXフォント

// フォント管理テーブル
typedef struct FontInfo {
  uint32_t    idx_addr;  // インデックス格納先頭アドレス
  uint32_t    dat_addr;  // フォントデータ格納先頭アドレス
  uint16_t    f_num;     // フォント格納数
  uint8_t     b_num;     // フォントあたりのバイト数
  uint8_t     w;         // 文字幅
  uint8_t     h;         // 文字高さ
} FontInfo;

class sdfonts {
  // メンバー変数
  private:
    uint8_t   _fontNo;      // 利用フォント種類
    uint8_t   _fontSize;    // 利用フォントサイズ
    uint8_t   _CSpin;       // SDカードCSピン
    uint16_t  _code;        // 直前に処理した文字コード(utf16)
    bool	    _lcdmode;     // グラフィック液晶モード
    MYSDCLASS &_mSD;        // SDオブジェクトの参照
    File	    fontFile;     // ファイル操作オブジェクト
    

  // クラスメンバー関数
  public:  
    static uint8_t charUFT8toUTF16(uint16_t *pUTF16,char *pUTF8 );   // UTF8文字(1～3バイト)をUTF16に変換する
    static int16_t Utf8ToUtf16(uint16_t* pUTF16, char *pUTF8);       // UTF8文字列をUTF16文字列に変換す
    
  // メンバー関数
  public:
#if SDFONTS_USE_SDFAT == 1
  sdfonts(MYSDCLASS& rsd) : _mSD(rsd) {
  #else
  sdfonts(MYSDCLASS& rsd = SD) : _mSD(rsd) {
#endif
    _fontSize = EXFONT8;
      _fontNo   = EXFONT8+FULL_OFST;
      _code     = 0;
      _lcdmode  = false;
    };
    
    bool init(uint8_t cs)  ;                               // 初期化
	  void setLCDMode(bool flg);                             // グラフィック液晶モードの設定
	  void setFontSizeAsIndex(uint8_t sz);                   // 利用サイズを番号で設定
    uint8_t getFontSizeIndex();                            // 現在利用フォントサイズの番号取得      
    void setFontSize(uint8_t sz);                          // 利用サイズの設定
    uint8_t getFontSize();                                 // 現在利用フォントサイズの取得      

    boolean getFontData(byte* fontdata,uint16_t utf16);    // 指定したUTF16コードに該当するフォントデータの取得
    char*   getFontData(byte* fontdata,char *pUTF8);       // 指定したUTF8文字列の先頭のフォントデータの取得
    uint8_t getRowLength();                                // 1行のバイト数
    uint8_t getWidth();                                    // 現在利用フォントの幅の取得
    uint8_t getHeight();                                   // 現在利用フォントの高さの取得
    uint8_t getLength();                                   // 現在利用フォントのデータサイズ
    bool open(void);                                       // フォントファイルのオープン
    void close(void);                                      // フォントファイルのクローズ
    uint16_t getCode() {return _code;};                    // 直前に処理した文字コード
   
  private:
    void setFontNo(uint8_t fno);                           // 利用フォント種類の設定 fno : フォント種別番号 (0-13)
    uint8_t getFontNo();                                   // 現在の利用フォント種類の取得
    uint16_t read_code(uint16_t pos);                      // ROM上検索テーブルのフォントコードを取得する
    int16_t findcode(uint16_t  ucode);                     // UTF16コードに該当するテーブル上のフォントコードを取得する
    uint16_t hkana2kana(uint16_t ucode);                   // 半角カナ全角変換 JISX0208 -> UTF16の不整合対応
    uint16_t hkana2uhkana(uint16_t ucode);                 // UTF半角カナ半角utf16コード変換 JISX0208 -> UTF16の不整合対応
    boolean getFontDataByUTF16(byte* fontdata, uint16_t utf16); // 種類に該当するフォントデータの取得
    uint8_t isHkana(uint16_t ucode);                       // 半角カナ判定
    uint32_t cnvAddres(uint8_t pos, uint8_t ln);  
    bool fontfile_read(uint32_t pos, uint8_t* dt, uint8_t sz) ;
};

extern sdfonts SDfonts;

#endif
