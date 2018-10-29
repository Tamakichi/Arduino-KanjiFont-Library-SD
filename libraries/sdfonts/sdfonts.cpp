//
// フォント利用ライブラリ クラス実装定義 sdfonts.cpp
// 作成 2016/05/13 by Tamakichi
// 修正 2016/05/15 by Tamakichi 半角カナ全角変換テーブル、フォント種別テーブルをフラッシュメモリ配置
// 修正 2016/05/16 by Tamakichi インスタンスをグローバル変数化、不具合対応
// 修正 2016/05/17 by Tamakichi fontfile_read()をブロック読み込みに修正
// 修正 2016/05/19 by たま吉さん, グラフィック液晶用フォントモードの追加(setLCDMode()関数追加)
// 修正 2016/06/26 by たま吉さん, ESP8266対応(ARDUINO_ARCH_AVRの判定追加),read_code()の不具合対応
// 修正 2016/12/15 by たま吉さん, findcode()の不具合対応(flg_stopの初期値を-1から0に訂正)
// 修正 2017/03/22 by たま吉さん, getFontData()の不具合対応(0x3000以下の全角文字が取得できなかった)
// 修正 2018/07/10 by たま吉さん, sdfat対応,Utf8ToUtf16()の戻り値型をint16_tに修正
// 修正 2018/10/29 by たま吉さん, sdfat利用時のグローバルオブジェクトSDの実装ミス対応
//

#define MYDEBUG 0 
#define USE_CON 0
 
#include "sdfonts.h"
#if defined(ARDUINO_ARCH_AVR)
  #include <avr/pgmspace.h>
#endif


#define SD_CS_PIN 10              // SDカード CSピン
#define FONTFILE   "FONT.BIN"     // フォントファイル名
#define FONT_LFILE "FONTLCD.BIN"  // グラフィック液晶用フォントファイル名

#define OFSET_IDXA  0
#define OFSET_DATA  3
#define OFSET_FNUM  6
#define OFSET_BNUM  8
#define OFSET_W     9
#define OFSET_H    10
#define RCDSIZ     11

// フォント種別テーブル
static PROGMEM const uint8_t  _finfo[] = {
   0x00,0x00,0x00, 0x00,0x01,0x7E,  0x00,0xbf,  8,  4,  8 , // 0:u_4x8a.hex
   0x00,0x07,0x76, 0x00,0x09,0x76,  0x01,0x00, 10,  5, 10 , // 1:u_5x10a.hex
   0x00,0x13,0x76, 0x00,0x15,0x76,  0x01,0x00, 12,  6, 12 , // 2:u_6x12a.hex
   0x00,0x21,0x76, 0x00,0x23,0x30,  0x00,0xdd, 14,  7, 14 , // 3:u_7x14a.hex
   0x00,0x2F,0x46, 0x00,0x31,0x00,  0x00,0xdd, 16,  8, 16 , // 4:u_8x16a.hex
   0x00,0x3E,0xD0, 0x00,0x40,0x4C,  0x00,0xbe, 40, 10, 20 , // 5:u_10x20a.hex
   0x00,0x5D,0xFC, 0x00,0x5F,0xB6,  0x00,0xdd, 48, 12, 24 , // 6:u_12x24a.hex
   0x00,0x89,0x26, 0x00,0xBE,0xE4,  0x1a,0xdf,  8,  8,  8 , // 7:u_8x8.hex
   0x01,0x95,0xDC, 0x01,0xCB,0x96,  0x1a,0xdd, 20, 10, 10 , // 8:u_10x10.hex
   0x03,0xE4,0xDA, 0x04,0x1A,0x98,  0x1a,0xdf, 24, 12, 12 , // 9:u_12x12.hex
   0x06,0x9F,0x80, 0x06,0xD5,0x3E,  0x1a,0xdf, 28, 14, 14 , // 10:u_14x14.hex
   0x09,0xC5,0xA2, 0x09,0xFB,0x60,  0x1a,0xdf, 32, 16, 16 , // 11:u_16x16.hex
   0x0D,0x57,0x40, 0x0D,0x8C,0xFE,  0x1a,0xdf, 60, 20, 20 , // 12:u_20x20.hex
   0x13,0xD9,0x42, 0x14,0x0E,0xFC,  0x1a,0xdd, 72, 24, 24 , // 13:u_24x24.hex
};

// 半角カナ全角変換テーブル
static PROGMEM const uint8_t _hkremap [] = {
   0x02,0x0C,0x0D,0x01,0xFB,0xF2,0xA1,0xA3,0xA5,0xA7,0xA9,0xE3,0xE5,0xE7,0xC3,0xFD,
   0xA2,0xA4,0xA6,0xA8,0xAA,0xAB,0xAD,0xAF,0xB1,0xB3,0xB5,0xB7,0xB9,0xBB,0xBD,0xBF,
   0xC1,0xC4,0xC6,0xC8,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD2,0xD5,0xD8,0xDB,0xDE,0xDF,
   0xE0,0xE1,0xE2,0xE4,0xE6,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEF,0xF3,0x9B,0x9C
};

// フラッシュメモリ上の3バイトをuint32_t型で取り出し
uint32_t sdfonts::cnvAddres(uint8_t pos, uint8_t ln) {
  uint32_t addr;
  pos += ln*RCDSIZ;
  addr =  (((uint32_t)pgm_read_byte(_finfo+pos))<<16)+(((uint32_t)pgm_read_byte(_finfo+pos+1))<<8)+pgm_read_byte(_finfo+pos+2);
  return addr;
}

// 初期化
bool sdfonts::init(uint8_t cs) {
  _CSpin = cs;
  //return SD.begin(_CSpin);
#if SDFONTS_USE_SDFAT == 1
  return _mSD.begin(_CSpin, SDFONTS_SPI_SPEED);
#else
  return _mSD.begin(_CSpin);
#endif  
}

// グラフィック液晶モードの設定
void sdfonts::setLCDMode(bool flg) {
	_lcdmode = flg;
}


// 利用フォント種類の設定 fno : フォント種別番号 (0-13)
void sdfonts::setFontNo(uint8_t fno) {
  _fontNo = fno;
}

// 現在の利用フォント種類の取得
uint8_t sdfonts::getFontNo() {
  return _fontNo;
}

//利用サイズの設定
void sdfonts::setFontSizeAsIndex(uint8_t sz) {
  _fontSize = sz;
  _fontNo = sz+FULL_OFST;
}

// 現在の利用フォントサイズの取得
uint8_t sdfonts::getFontSizeIndex() {
  return _fontSize; 
}
// 利用サイズの設定
void sdfonts::setFontSize(uint8_t sz) {
  if (sz < 10) 
    setFontSizeAsIndex(EXFONT8);   
  else if (sz < 12) 
    setFontSizeAsIndex(EXFONT10);   
  else if (sz < 14)
    setFontSizeAsIndex(EXFONT12);   
  else if (sz < 16)
    setFontSizeAsIndex(EXFONT14);   
  else if (sz < 20)
    setFontSizeAsIndex(EXFONT16);   
  else if (sz < 24)
    setFontSizeAsIndex(EXFONT20);   
  else if (sz >= 24)
    setFontSizeAsIndex(EXFONT24); 
}

// 現在利用フォントサイズの取得                             
uint8_t sdfonts::getFontSize() {
  return getHeight(); 
}

// 現在利用フォントの幅の取得
uint8_t sdfonts::getWidth() {
  return pgm_read_byte(_finfo +_fontNo * RCDSIZ + OFSET_W);
}
  
// 現在利用フォントの高さの取得
uint8_t sdfonts::getHeight() {
  return pgm_read_byte(_finfo +_fontNo * RCDSIZ + OFSET_H);

}
  
// 現在利用フォントのデータサイズの取得
uint8_t sdfonts::getLength() { 
  return pgm_read_byte(_finfo +_fontNo * RCDSIZ + OFSET_BNUM);
}

// 半角カナコード判定
uint8_t sdfonts::isHkana(uint16_t ucode) {
  if (ucode >=0xFF61 && ucode <= 0xFF9F)
     return 1;
  else 
    return 0;  
}

// 半角カナ全角変換
// JISX0208 -> UTF16の不整合対応
uint16_t sdfonts::hkana2kana(uint16_t ucode) {
  if (isHkana(ucode))
    return pgm_read_byte(_hkremap+ucode-0xFF61) + 0x3000; 
  return ucode;
}

uint16_t sdfonts::hkana2uhkana(uint16_t ucode) {
  if (isHkana(ucode))
     return ucode - 0xFF61 + 0x0A1;
  return ucode;      
}

//
// フォントファイルのオープン
//
bool sdfonts::open() {
  if (_lcdmode)
	fontFile = _mSD.open(FONT_LFILE, FILE_READ);
  else	
	fontFile = _mSD.open(FONTFILE, FILE_READ);
	
	
  if (!fontFile) {
#if MYDEBUG == 1 && USE_CON == 1    
    Serial.print(F("cant open:"));
    Serial.println(F(FONTFILE));
#endif
    return false;
  }
  return true;  
}

//
// ファイルのクローズ
//
void sdfonts::close(void) {
  fontFile.close(); 
}

//
// フォントファイルからのデータ取得
// pos(in) 読み込み位置
// dt(out) データ格納領域
// sz(in)  データ取得サイズ
//
bool sdfonts::fontfile_read(uint32_t pos, uint8_t* dt, uint8_t sz) {
  if (!fontFile) {
    return false;  
  } else {
    if ( !fontFile.seek(pos) )   
      return false;
 /*	
    for (uint8_t i = 0 ; i < sz; i++) {
      if ( !fontFile.available() ) 
        return false;
      dt[i] = fontFile.read();  
    }
*/
  	if (fontFile.read(dt, sz) != sz)
  		return false;	

  }
  return true;  
}

// フォントコード取得
// ファイル上検索テーブルのフォントコードを取得する
// pos(in) フォントコード取得位置
// 戻り値 該当コード or 0xFFFF (該当なし)
//
uint16_t sdfonts::read_code(uint16_t pos) {
  uint8_t rcv[2];
  uint32_t addr = cnvAddres(OFSET_IDXA, _fontNo) + pos+pos;
  if (!fontfile_read(addr, rcv, 2))  
    return 0xFFFF;
  return  (rcv[0]<<8)+rcv[1]; 
}

// フォントコード検索
// (コードでROM上のテーブルを参照し、フォントコードを取得する)
// ucode(in) UTF-16 コード
// 戻り値    該当フォントがある場合 フォントコード(0-FTABLESIZE)
//           該当フォントが無い場合 -1

int16_t sdfonts::findcode(uint16_t  ucode) {
   uint16_t  t_p = 0;                        //　検索範囲上限
   uint16_t  e_p = (((uint16_t)pgm_read_byte(_finfo+_fontNo*RCDSIZ+OFSET_FNUM))<<8) 
                  + pgm_read_byte(_finfo+_fontNo*RCDSIZ+OFSET_FNUM+1) -1;   //  検索範囲下限
   uint16_t  pos;
   uint16_t  d = 0;
   int8_t flg_stop = 0;
 
 while(true) {
    pos = t_p + ((e_p - t_p+1)>>1);
    d = read_code (pos);
    if (d==0xFFFF)
      return -1;  
 	
   if (d == ucode) {
     // 等しい
     flg_stop = 1;
     break;
   } else if (ucode > d) {
     // 大きい
     t_p = pos + 1;
     if (t_p > e_p) 
       break;
   } else {
     // 小さい
    e_p = pos -1;
    if (e_p < t_p) 
      break;
   }
 } 

 if (!flg_stop)
    return -1;   
 return pos;   
}

//
// UTF16に対応するフォントデータを取得する
//   data(out): フォントデータ格納アドレス
//   utf16(in): UTF16コード
//   戻り値: true 正常終了１, false 異常終了
//
boolean sdfonts::getFontData(byte* fontdata, uint16_t utf16) {
  boolean flgZenkaku = true;
  
  // 文字コードの変更(＼￠￡￢)
  switch(utf16) {
    case 0xFF3C: utf16 = 0x5C;  break;
    case 0xFFE0: utf16 = 0xA2;  break;
    case 0xFFE1: utf16 = 0xA3;  break;
    case 0xFFE2: utf16 = 0xAC;  break;
  }
  
 // 文字コードから全角、半角を判定する
 if (utf16 < 0x100) {
     switch (utf16) {
       case 0x5C:
       case 0xA2:
       case 0xA3:
       case 0xA7:
       case 0xA8:
       case 0xAC:
       case 0xB0:
       case 0xB1:
       case 0xB4:
       case 0xB6:
       case 0xD7:
       case 0xF7:
         flgZenkaku = true;
         break;
       default:
         flgZenkaku = false;
     } 
   } else 
    
  // 半角カナは全角カナに置き換える
  if (isHkana(utf16)) {
    utf16 = hkana2kana(utf16);
  }
  
  //フォント種別の設定
  if (flgZenkaku) 
    setFontNo(getFontSizeIndex()+FULL_OFST);  // 全角フォント指定
  else
    setFontNo(getFontSizeIndex());            // 半角フォント指定
    
  _code = utf16;
  return getFontDataByUTF16(fontdata, utf16);
}

// 指定したUTF8文字列の先頭のフォントデータの取得
//   data(out): フォントデータ格納アドレス
//   utf8(in) : UTF8文字列
//   戻り値   : 次の文字列位置、取得失敗の場合NULLを返す
//
char* sdfonts::getFontData(byte* fontdata,char *pUTF8) {
  uint16_t utf16;
  uint8_t  n;
  if (pUTF8 == NULL)
    return NULL;
  if (*pUTF8 == 0) 
    return NULL;   
  n = charUFT8toUTF16(&utf16, pUTF8);
  if (n == 0)
    return NULL;  
  if (false == getFontData(fontdata, utf16) ) 
    return NULL;
  return (pUTF8+n);
}

//
// UTF16に対応するフォントデータを取得する
//   data(out): フォントデータ格納アドレス
//   utf16(in): UTF16コード
//   戻り値: true 正常終了１, false 異常終了
//
boolean sdfonts::getFontDataByUTF16(byte* fontdata, uint16_t utf16) {  
  int16_t code;
  uint32_t addr;
  uint8_t bnum;
 
  code = findcode(utf16);
  if ( 0 > code)  
    return false;       // 該当するフォントが存在しない
    
  bnum = pgm_read_byte(_finfo+_fontNo*RCDSIZ+OFSET_BNUM);
  addr = cnvAddres(OFSET_DATA, _fontNo ) + (uint32_t)code * (uint32_t)bnum;
  return fontfile_read(addr, fontdata, bnum );
}

//
// フォントデータ1行のバイト数の取得
// 戻り値： バイト数
uint8_t sdfonts::getRowLength() {
    return ( pgm_read_byte( _finfo + _fontNo * RCDSIZ + OFSET_W ) + 7 ) >>3;
}

//
// UTF8文字(1～3バイト)をUTF16に変換する
// pUTF16(out): UTF16文字列格納アドレス
// pUTF8(in):   UTF8文字列格納アドレス
// 戻り値: 変換処理したUTF8文字バイト数、0の場合は変換失敗

uint8_t sdfonts::charUFT8toUTF16(uint16_t *pUTF16, char *pUTF8) { 
  uint8_t  bytes[3]; 
  uint16_t unicode16; 
 
  bytes[0] = *pUTF8++; 
  if( bytes[0] < 0x80 ) { 
   *pUTF16 = bytes[0]; 
   return(1); 
 } 
  bytes[1] = *pUTF8++; 
  if( bytes[0] >= 0xC0 && bytes[0] < 0xE0 )  { 
     unicode16 = 0x1f&bytes[0]; 
     *pUTF16 = (unicode16<<6)+(0x3f&bytes[1]); 
     return(2); 
 } 
 
  bytes[2] = *pUTF8++; 
  if( bytes[0] >= 0xE0 && bytes[0] < 0xF0 ) { 
    unicode16 = 0x0f&bytes[0]; 
    unicode16 = (unicode16<<6)+(0x3f&bytes[1]); 
    *pUTF16 = (unicode16<<6)+(0x3f&bytes[2]); 
    return(3); 
  } else 
  return(0); 
} 

// UTF8文字列をUTF16文字列に変換する
// pUTF16(out): UFT16文字列
// pUTF8(in):   UTF8文字列
// 戻り値: UFT16文字長さ (変換失敗時は-1を返す)
//
int16_t sdfonts::Utf8ToUtf16(uint16_t* pUTF16, char *pUTF8) {
  int len = 0;
  int n;
  uint16_t wstr;

  while (*pUTF8) {
    n = charUFT8toUTF16(pUTF16,pUTF8);
    if (!n) 
      return -1;
    
    pUTF8 += n;
    len++;
    pUTF16++;
  }
  return len; 
}

//
// グローバルオブジェクトの宣言
//
#if SDFONTS_USE_SDFAT == 1
MYSDCLASS SD;
#endif
sdfonts SDfonts(SD);
