
/////////////////////////////////////////////////////////////////////////////
//
//             /
//      -  P  /  E  C  E  -
//           /                 mobile equipment
//
//              System Programs
//
//
// PIECE LIBRARY : muslib : Ver 1.00
//
// Copyright (C)2001 AUQAPLUS Co., Ltd. / OeRSTED, Inc. all rights reserved.
//
// Coded by MIO.H (OeRSTED)
//
// Comments:
//
// PIECE 標準 音楽ドライバー
//
// ドライバ内定義ファイル
//
//  v1.00 2001.11.09 MIO.H
//  v1.01 2003.02.23 S.Kino.	可変周波数に対応させるために，いくつかの
//														マクロをグローバル変数化
//



#ifndef _MUSDEF_H
#define _MUSDEF_H


#define MAXNESTD 24
#define MAXCH 6

typedef struct tagINST {
	unsigned char wtype;
	unsigned char param;
	unsigned char resv1;
	unsigned char resv2;
	unsigned short pitch_fs;
	unsigned short pitch_org;
	signed char *pData;
	unsigned long loop_top;
	unsigned long loop_end;
} INST;

#define IT_LOOP  0x10
#define IT_FAST  0x80

typedef struct tagMDEVICE {
	// 以下の20バイトはアセンブラのルーチンでも処理されるので位置を変えないこと
	signed char *pData;			// +0
	unsigned long freqwk;		// +4
	unsigned long freq;			// +8
	unsigned long loop_end;		// +12
	unsigned long loop_w;		// +16
	//
	unsigned long ipitch;
	unsigned long cpitch;
	signed long ptwk1;
	signed long ptwk2;
	signed long envwk;
	signed long envAR;
	signed long envDR;
	signed long envSL;
	signed long envSR;
	signed long pvol;
	signed long pexp;
	signed short vibwk;
	signed short vibvv;
	signed short vibv1;
	signed short vibv2;
	signed short vibdpwk;
	signed short vibdpe;
	unsigned char ch;
	unsigned char vsta;
	void (*genwave)( struct tagMDEVICE *mp, signed short *p, signed long vv, int cnt );
	void (*genenv)( struct tagMDEVICE *mp );
	void (*genvib)( struct tagMDEVICE *mp );
} MDEVICE;


typedef struct tagSEQWK {
	MDEVICE mdwk;
	unsigned char *pseq;
	signed long porvv;
//	signed long porpv;
	unsigned short cnt;
	unsigned short len;
	unsigned short stop;
	unsigned short gate;
	unsigned char ch;
	unsigned char run;
	unsigned char np;
	unsigned char tatt;
	unsigned char tsta;
	signed char trs;
	signed char dtn;
	signed char porpd;
	signed char lnote;
	void (*portament)( struct tagSEQWK *psw );
	unsigned short nestd[MAXNESTD];
} SEQWK;


#define PITCH44K 47396
#define PITCH22K 44324
#define PITCH16K 42903
#define PITCH440 26976
#define PITCH_A4 PITCH440
#define PITCH_C4 (PITCH_A4-9*256)

// SK グローバル変数化
/*
#if defined(FS22K)
////////////////////// fs=22k
#define FS  44100
#define FS2 22050

//	2.5 * f(Hz)
//
// 22050/128*2.5*16=6890.625
#define MCOUNT 6891

// 22050
#define FOFS PITCH22K

#elif defined(FS44K)
////////////////////// fs=44k
#define FS  88200
#define FS2 44100

//	2.5 * f(Hz)
//
// 44100/128*2.5*16=13781.25
#define MCOUNT 13781

// 44100
#define FOFS PITCH44K

#else
////////////////////// fs=16k
#define FS  32000
#define FS2 16000

// 16000/128*2.5*16=5000
#define MCOUNT 5000

// 16000
#define FOFS PITCH16K

#endif
*/

// Yui: そのままだと BCC でコンパイル不可。実体は winlayer.cpp に移した
extern long FS, FS2, MCOUNT, FOFS;
// SK : ここまで

//              16384                 32
//              -----                 ------
#define IBASE0 (43008+PITCH440-69*256+60*256)


#define WAVEADR(adr) ((signed char *)adr)

void SetInst( MDEVICE *pmd, int no );
void SetVol( MDEVICE *pmd, int vol );
void SetExp( MDEVICE *pmd, int exp );
void SetExpRel( MDEVICE *pmd, int expr );
void SetEnv( MDEVICE *pmd, int ar, int dr, int sl, int sr );
void SetVib( MDEVICE *pmd, int depth, int spd, int t1, int t2 );
void NoteOn( MDEVICE *pmd, int pitch );
void NotePitch( MDEVICE *mp, int pitch );
void NoteOff( MDEVICE *pmd );
void SetupCh( MDEVICE *mp, int ch );

unsigned long gexp( int n );


#define XSF1 10    // 音程変化（ポルタメント）のシフト数
#define XSF2 14    // 音程変化（ビブラート）のシフト数

#endif //ifndef _MUSDEF_H

