
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
// 音源ドライバー
//
//  v1.00 2001.11.09 MIO.H
//  v1.01 2003.02.23 S.Kino.	可変周波数に対応
//  v1.02 2003   ?   Yui N.		演奏終了を検出できるようにした
//  v1.03 2003.05.07 Yui N.		はらやんさんのプチノイズ対策を実装
//  v1.04 2003.08.03 Yui N., Kobarin
//														Kobarin さんの msvcrt レス化パッチを取り込むも、
//														コンパイラを変更したので無用に（ぉ
//  v1.05 2003.08.07 Yui N.   エンベロープとビブラートの調整が大嘘だったので修正;;
//



#include "musdefwin.h"
// #include <piece.h>
#include "musdef.h"
#include <math.h>

#define BLKN 4
#define BLKS 128

void InitSeq( void );
void StartSeq( unsigned char *seq );
void StopSeq( void );
int ProcSeq( void );

extern INST *inst[];

//static 解除 by Kobarin
/*static*/ signed short wavebuff[BLKN][BLKS];
/*static*/ volatile unsigned char blkip;
/*static*/ volatile unsigned char blkop;
unsigned char finish;		// Yui: winlayer.cpp から参照するため static をはずした

/*static*/ PCEWAVEINFO waveinfo[BLKN];

/*static*/ MDEVICE *music[MAXCH];

unsigned char music_wch;

#ifndef FASTCODE
// これが波形合成処理の核です。(→アセンブラ・バージョンあり)
// 波形読み出し
//
void MakeWaveLP( MDEVICE *mp, signed short *p, signed long vv, int cnt )
{
	signed char *tbl = mp->pData;
	unsigned long cc = mp->freqwk;
	do {
		unsigned long o = ( cc >> 14 );
		unsigned long x = ( cc & 0x3fff );
		signed long d1 = tbl[o];
		signed long d2 = tbl[o+1];
		if ( (cc += mp->freq) >= mp->loop_end ) cc -= mp->loop_w;
		d2 -= d1;
		d2 *= x;
		d2 >>= 14;
		d1 += d2;
		d1 *= vv;
		*p++ += (signed short)(d1>>8);
	} while ( --cnt );
	mp->freqwk = cc;
}

void MakeWaveNL( MDEVICE *mp, signed short *p, signed long vv, int cnt )
{
	signed char *tbl = mp->pData;
	unsigned long cc = mp->freqwk;
	do {
		unsigned long o = ( cc >> 14 );
		unsigned long x = ( cc & 0x3fff );
		signed long d1 = tbl[o];
		signed long d2 = tbl[o+1];
		if ( (cc += mp->freq) >= mp->loop_end ) {
			mp->freq = 0;
			return;
		}
		d2 -= d1;
		d2 *= x;
		d2 >>= 14;
		d1 += d2;
		d1 *= vv;
		*p++ += (signed short)(d1>>8);
	} while ( --cnt );
	mp->freqwk = cc;
}

// 矩形波に最適化
//
void MakeWaveSQR( MDEVICE *mp, signed short *p, signed long vv, int cnt )
{
	signed long cc = mp->freqwk;
	vv >>= 1;
	do {
		*p++ += (signed short)((cc & 0x8000) ? vv : -vv);
		cc += mp->freq;
	} while ( --cnt );
	mp->freqwk = cc;
}

// のこぎり波に最適化
//
void MakeWaveSAW( MDEVICE *mp, signed short *p, signed long vv, int cnt )
{
	signed long cc = mp->freqwk;
	do {
		signed short aa = (signed short)cc;
		*p++ += (signed short)((aa * vv) >>16);
		cc += mp->freq;
	} while ( --cnt );
	mp->freqwk = cc;
}

// 三角波に最適化
//
void MakeWaveTRI( MDEVICE *mp, signed short *p, signed long vv, int cnt )
{
	signed short cc = (signed short)mp->freqwk;
	vv >>= 1;
	do {
		signed long dd = cc * vv;
		if ( cc < 0 ) dd = -dd;
		dd -= (vv<<14);
		*p++ += (signed short)(dd >> 14);
		cc += (signed short)mp->freq;
	} while ( --cnt );
	mp->freqwk = cc;
}
#else
void MakeWaveLP_fast( MDEVICE *mp, signed short *p, signed long vv, int cnt );
void MakeWaveNL_fast( MDEVICE *mp, signed short *p, signed long vv, int cnt );
void MakeWaveSQR_fast( MDEVICE *mp, signed short *p, signed long vv, int cnt );
void MakeWaveSAW_fast( MDEVICE *mp, signed short *p, signed long vv, int cnt );
void MakeWaveTRI_fast( MDEVICE *mp, signed short *p, signed long vv, int cnt );
#define MakeWaveLP MakeWaveLP_fast
#define MakeWaveNL MakeWaveNL_fast
#define MakeWaveSQR MakeWaveSQR_fast
#define MakeWaveSAW MakeWaveSAW_fast
#define MakeWaveTRI MakeWaveTRI_fast
#endif

// --------------------------------
// エンベロープ・ジェネレーター

// SK : グローバル変数化
/*
#define VOLS 1

#define ENVSTART ((-4*12*256)<<VOLS)
#define ENVSTOP  ((-10*12*256)<<VOLS)
#define ENVRR    (4000<<VOLS)
*/

long VOLS, ENVSTART, ENVSTOP, ENVRR;
// SK : ここまで

static signed long SetEnvSpeed( int n )
{
	return gexp( n*192 + 4*12*256 )<<VOLS;
}

// SK : 可変周波数対応 エンベロープスピード生成関数
static signed long SetEnvSpeed2( int n )
{
	signed long r;
	signed long long rr;

	// 2003.08.07 Yui N.: これはレート値なので Freq. の調整が必要
	rr = (gexp( n*192 + 4*12*256 ) << VOLS) *  16000;
	rr /= FS2;
	return rr;
}

static void GenEnvR( MDEVICE *mp )
{
	if ( ( mp->envwk -= ENVRR ) < ENVSTOP ) {
		mp->envwk = ENVSTOP;
		mp->freq = 0;
	}
}

static void GenEnvS( MDEVICE *mp )
{
	if ( ( mp->envwk -= mp->envSR ) < ENVSTOP ) {
		mp->envwk = ENVSTOP;
		mp->freq = 0;
	}
}

static void GenEnvD( MDEVICE *mp )
{
	if ( ( mp->envwk -= mp->envDR ) < mp->envSL ) {
		mp->genenv = GenEnvS;
	}
}

static void GenEnvA( MDEVICE *mp )
{
	if ( ( mp->envwk += mp->envAR ) > 0 ) {
		mp->envwk = 0;
		mp->genenv = GenEnvD;
	}
}

static void GenEnvInit( MDEVICE *mp )
{
	mp->envwk = mp->envAR ? ENVSTART : 0;
	mp->genenv = GenEnvA;
}


void SetEnv( MDEVICE *mp, int ar, int dr, int sl, int sr )
{
	if ( sl > 127 ) sl = 127;
	mp->envSL = (sl-127) * (192*(1<<VOLS));
	// SK : 可変周波数に対応した関数を呼び出す
	mp->envAR = SetEnvSpeed2( ar );//SetEnvSpeed( ar );
	mp->envDR = SetEnvSpeed2( dr );//SetEnvSpeed( dr );
	mp->envSR = SetEnvSpeed2( sr );//SetEnvSpeed( sr );
#ifdef DEBUGENV
	printf( "SetEnv=%d,%d,%d,%d\n", ar, dr, sl, sr );
	printf( "St=%d ", ENVSTART>>VOLS );
	printf( "AR=%d", mp->envAR>>VOLS );
	printf( "(%5.3f)", -ENVSTART/(double)mp->envAR/125.0 );
	printf( "DR=%d", mp->envDR>>VOLS );
	printf( "(%5.3f)", -mp->envSL/(double)mp->envDR/125.0 );
	printf( "SL=%d ", mp->envSL>>VOLS );
	printf( "SR=%d", mp->envSR>>VOLS );
	printf( "(%5.3f)", (mp->envSL-ENVSTOP)/(double)mp->envSR/125.0 );
	printf( "RR=%d ", ENVRR>>VOLS );
	printf( "Ed=%d\n", ENVSTOP>>VOLS );
#endif
}

// --------------------------------
// ビブラート・ジェネレーター

#define T1CONST 0x4000

// SK : 可変周波数対応 ビブラートスピード生成関数
static signed short SetVibSpeed2( int n )
{
	signed short r;
	signed long rr;

	// 2003.08.07 Yui N.: これはレート値なので Freq. の調整が必要
	rr = gexp( n * 192 + 8*12*256 ) * 16000;
	rr /= FS2;
	return rr;
}

static void GenVibS2( MDEVICE *mp )
{
	int a = ( mp->vibwk += mp->vibvv );
	if ( a < 0 ) a = -a;
	a = 0x4000 - a;
	mp->ptwk2 = ((a * mp->vibdpwk)>>3);
#ifdef DEBUGVIB
	sprintf( debug_msg, "%6d %6d", (mp->vibdpwk*0x4000)>>(14+3), mp->ptwk2>>14 );
#endif
}

static void GenVibS1( MDEVICE *mp )
{
	if ( ( mp->vibdpwk += mp->vibv2 ) >= mp->vibdpe ) {
		mp->genvib = GenVibS2;
		mp->vibdpwk = mp->vibdpe;
	}
	GenVibS2( mp );
}

static void GenVibS0( MDEVICE *mp )
{
	if ( ( mp->vibdpwk += mp->vibv1 ) >= T1CONST ) {
		mp->genvib = GenVibS1;
		mp->vibwk = 0;
		mp->vibdpwk = 0;
	}
}

static void GenVibInit( MDEVICE *mp )
{
	mp->genvib = GenVibS0;
	mp->vibdpwk = 0;
}

void SetVib( MDEVICE *mp, int depth, int spd, int t1, int t2 )
{
	mp->vibdpe = depth * 12 * 8;
	// SK : 可変周波数に対応した関数を呼び出す
	mp->vibvv  = SetVibSpeed2( spd );//(signed short)gexp( spd * 192 + 8*12*256 );
	mp->vibv1  = SetVibSpeed2( t1 );//(signed short)gexp( t1 * 192 + 8*12*256 );
	mp->vibv2  = SetVibSpeed2( t2 );//(signed short)gexp( t2 * 192 + 8*12*256 );
#ifdef DEBUGVIB
	printf( "SetVib=%d,%d,%d,%d\n", depth, spd, t1, t2 );
	printf( "vibdpe = %d ", mp->vibdpe );
	printf( "vibvv = %d ", mp->vibvv );
	printf( "vibv1 = %d ", mp->vibv1 );
	printf( "vibv2 = %d\n", mp->vibv2 );
#endif
}



void SetInst( MDEVICE *mp, int no )
{
	INST *ip = inst[no];

	mp->pData = ip->pData;
	mp->loop_end = ip->loop_end;
	mp->loop_w = ip->loop_end - ip->loop_top;
	mp->freq = 0;
	mp->ipitch = ip->pitch_fs - ip->pitch_org + ( IBASE0 - FOFS );
	mp->envAR = 0;

	if ( ip->wtype & IT_FAST ) {
		switch ( ip->param ) {
			case 0  : mp->genwave = MakeWaveSQR; break;
			case 1  : mp->genwave = MakeWaveSAW; break;
			case 2  : mp->genwave = MakeWaveTRI; break;
		}
	}
	else if ( ip->wtype & IT_LOOP ) {
		mp->genwave = MakeWaveLP;
	}
	else {
		mp->genwave = MakeWaveNL;
	}
}

void SetVol( MDEVICE *mp, int vol )
{
	mp->pvol = ((vol * 192 + 11*12*256)<<VOLS);
	mp->pexp = 0;
}

void SetExp( MDEVICE *mp, int exp )
{
	exp -= 127;
	mp->pexp = ((exp * 192)<<VOLS);
}

void SetExpRel( MDEVICE *mp, int expr )
{
	mp->pexp += ((expr * 192)<<VOLS);
}

void NoteOn( MDEVICE *mp, int pitch )
{
	mp->freqwk = 0;
	mp->ptwk1 = 0;
	mp->ptwk2 = 0;
	mp->cpitch = mp->ipitch + pitch;
	mp->freq = 1;
	GenEnvInit( mp );
	GenVibInit( mp );

	//if ( mp->ch == 3 ) printf( "%x,%x ", pitch, mp->cpitch );
}


void NotePitch( MDEVICE *mp, int pitch )
{
	mp->cpitch = mp->ipitch + pitch;
}


void NoteOff( MDEVICE *mp )
{
	mp->genenv = GenEnvR;
}


void SetupCh( MDEVICE *mp, int ch )
{
	music[ch] = mp;
	if ( mp ) mp->ch = ch;
}


void BuffFree( PCEWAVEINFO *pwi )
{
	int n = blkop+1;

	if ( finish ) {
		finish = 2;
		return;
	}

	if ( n == BLKN ) n = 0;
	blkop = n;

	while ( 1 ) {
		n = blkip+1;
		if ( n == BLKN ) n = 0;
		if ( n == blkop ) break;
		{
			PCEWAVEINFO *pwi = waveinfo + blkip;
			unsigned short *p0 = wavebuff[blkip];
			int ch;

			pwi->pData = p0;
			pwi->len = BLKS;
			pwi->type = PW_TYPE_16BITPCM|PW_TYPE_CONT;
			pwi->pfEndProc = BuffFree;
			pwi->stat = 0;
			pwi->resv = 0;
			pwi->next = 0;

			memset(p0, 0, 2 * BLKS);
			for ( ch = 0; ch < MAXCH; ch++ ) {
				MDEVICE *mp = music[ch];
				if ( mp && mp->freq ) {
					signed long vv;

					mp->genvib( mp );
					mp->freq = gexp(mp->cpitch + (mp->ptwk1>>XSF1) + (mp->ptwk2>>XSF2));

					mp->genenv( mp );
					vv = ((mp->pvol + mp->pexp + mp->envwk) >> VOLS);
					if ( vv <= 11 ) vv = 0; else vv = gexp( vv );
					mp->genwave( mp, p0, vv, BLKS );
				}
			}

			pceWaveDataOut(music_wch, pwi);
			blkip = n;
		}
	}

	if ( !ProcSeq() ) finish = 2;
}




int MusicCheck( void )
{
	return finish==2;
}


void init_wk( void )
{
	finish = 1;
	memset(music, 0, sizeof music);
	memset(waveinfo, 0, sizeof waveinfo);
	blkip = 0;
	blkop = 0;
}


void InitMusic( void )
{
	music_wch = 0;
	init_wk();
}


void PlayMusic( unsigned char *seq )
{
	init_wk();

	StartSeq( seq );

	{
		PCEWAVEINFO *pwi;
		int n = blkip+1;
		pwi = waveinfo + blkip;
		// yui: はらやんさんによるプチノイズ対策
		memset(wavebuff + blkip, 0, 2 * BLKS);
		pwi->pData = wavebuff+blkip;
		pwi->len = BLKS;
		pwi->type = PW_TYPE_16BITPCM|PW_TYPE_CONT;
		pwi->pfEndProc = BuffFree;
		pwi->stat = 0;
		pwi->resv = 0;
		pwi->next = 0;
		pceWaveDataOut(music_wch, pwi);
		blkip = n;
		finish = 0;
	}
}


void StopMusic( void )
{
	if ( !finish ) finish = 1;
}




