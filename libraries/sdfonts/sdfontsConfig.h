//
// フォント利用ライブラリ クラス定義 sdfontsConfig.h 
// 作成 2018/07/10 by たま吉さん
//

#ifndef ___sdfontsConfig_h___
#define ___sdfontsConfig_h___

// SDカード用ライブラリの選択
#define SDFONTS_USE_SDFAT   0 // 0:SDライブラリ利用, 1:SdFatライブラリ利用

// Sdfat利用時 SPI速度
#define SDFONTS_SPI_SPEED SD_SCK_MHZ(18)

#endif
