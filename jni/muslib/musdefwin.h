// とりあえずコピペ

#ifndef MUSDEFWIN_H
#define MUSDEFWIN_H

#include <stdlib.h>
#include <memory.h>
#include <string.h>

// サウンド関係のサービス
#define PW_TYPE_8BITPCM    0
#define PW_TYPE_16BITPCM   1
#define PW_TYPE_4BITADPCM  2
#define PW_TYPE_CONT       0x10    // 連続出力
#define PW_TYPE_ADP_INI    0x20    // ADPCM初期化要求
#define PW_TYPE_VR         0x40    // 可変レート

#define PW_TYPE_ADPCM      (PW_TYPE_4BITADPCM|PW_TYPE_ADP_INI)
#define PW_TYPE_ADPCM_V    (PW_TYPE_4BITADPCM|PW_TYPE_VR|PW_TYPE_ADP_INI)
#define PW_TYPE_ADPCM_NI   (PW_TYPE_4BITADPCM)
#define PW_TYPE_ADPCM_V_NI (PW_TYPE_4BITADPCM|PW_TYPE_VR)

#define PW_STAT_START      1       // 再生開始
#define PW_STAT_END        2       // 再生終了

typedef struct tagPCEWAVEINFO {
	volatile unsigned char stat;					// 0  ステータス
	unsigned char type;								// 1  データ形式
	unsigned short resv;							// 2  予約
	const void *pData;								// 4  データへのポインタ
	unsigned long len;								// 8  データの長さ(サンプル数)
	struct tagPCEWAVEINFO *next;					// 12 次へのポインタ
	void (*pfEndProc)( struct tagPCEWAVEINFO *);	// 16 終了時コールバック
} PCEWAVEINFO;

int pceWaveCheckBuffs( int ch );
int pceWaveDataOut( int ch, PCEWAVEINFO *pwave );
int pceWaveAbort( int ch );
int pceWaveSetChAtt( int ch, int att );
int pceWaveSetMasterAtt( int att );
void pceWaveStop( int hard );

#endif	//!MUSDEFWIN_H
