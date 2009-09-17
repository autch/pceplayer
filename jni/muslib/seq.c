
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
// シーケンサー
//
//  v1.00 2001.11.09 MIO.H
//  v1.01 2003.02.23 S.Kino.	可変周波数に対応
//  v1.02 2003.05.03 Yui N.		デチューンとヴィブラートが初期化されない仕様を改良
//



#ifdef _WIN32
#include "musdefwin.h"
#endif
#include "musdef.h"


#define TATT_DRUMS 1
#define TATT_NOOFF 2

#define TSTA_PORTASW 1
#define TSTA_INPORTA 2
#define TSTA_LEG1    4
#define TSTA_LEG2    8

char *title;
char *title2;

#define MAXSEQ MAXCH

//static 解除 by Kobarin
/*static*/ SEQWK seqwk[MAXSEQ];
/*static*/ unsigned char *datatop;
/*static*/ unsigned char *drumsbase;
/*static*/ signed short tempo;
/*static*/ signed short tempowk;
/*static*/ char runf;
//static 解除ここまで by Kobarin

int getadrs( unsigned char *p )
{
	return p[0] + (p[1]<<8);
}

static void SeqNop( SEQWK *psw )
{
}

void StartSeq( unsigned char *seqptr )
{
	int i, n, s;

	datatop = seqptr;
	tempo = (120<<4);
	tempowk = 0;
	runf = 1;

	n = *seqptr;
	if ( !n ) n = *++seqptr;

	if ( n > MAXSEQ ) n = MAXSEQ;

	s = 1;

	for ( i = 0; i < n; i++,s+=2 ) {
		SEQWK *psw = seqwk + i;
		psw->pseq = datatop + getadrs( seqptr + s );
		psw->cnt  = 1;
		psw->len  = 1;
		psw->ch   = i;
		psw->np   = 0;
		psw->trs  = 0;
		psw->run  = 1;
		psw->tatt  = 0;
		psw->tsta = 0;
		psw->gate = 1*41; // gate=99
		psw->portament = SeqNop;
		SetupCh( &psw->mdwk, i );
		SetInst( &psw->mdwk, 0 );
		SetVol( &psw->mdwk, 127 - i*16 );
		// yui: ビブラート初期化
		SetVib( &psw->mdwk, 0, 0, 0, 0);
		psw->dtn = 0;			// yui: デチューン初期化
	}
	for ( ; i < MAXSEQ; i++ ) {
		SEQWK *psw = seqwk + i;
		SetupCh( 0, i );
		psw->run  = 0;
	}

	s = 1 + *seqptr * 2;

	drumsbase = (unsigned char *)datatop + getadrs( seqptr + s );
	title = (char *)datatop + getadrs( seqptr + s + 2 );
	title2 = (char *)datatop + getadrs( seqptr + s + 4 );
}

static void SeqPortaExecP( SEQWK *psw )
{
	if ( (psw->mdwk.ptwk1 += psw->porvv) >= 0 ) {
		psw->mdwk.ptwk1 = 0;
		psw->tsta &= ~TSTA_INPORTA;
	}
}

static void SeqPortaExecN( SEQWK *psw )
{
	if ( (psw->mdwk.ptwk1 -= psw->porvv) <= 0 ) {
		psw->mdwk.ptwk1 = 0;
		psw->tsta &= ~TSTA_INPORTA;
	}
}

static void SeqNote( SEQWK *psw, int note )
{
	int ntw = ((note + psw->trs)<<8) + psw->dtn;

	psw->cnt = psw->len;

	if ( (psw->tsta & TSTA_LEG2) && psw->run==2 ) {
		NotePitch( &psw->mdwk, ntw );
	}
	else {
		NoteOn( &psw->mdwk, ntw );
		psw->run = 2;
	}
	
	if ( (psw->tsta & (TSTA_LEG1|TSTA_LEG2)) == TSTA_LEG2 ) {
		psw->tsta &= ~TSTA_LEG2;
	}
	
	if ( psw->tsta & TSTA_PORTASW ) {
		int a = psw->porpd;
		if ( !a ) a = psw->lnote - note;
		psw->mdwk.ptwk1 = (a<<(XSF1+8));
		if ( a < 0 ) {
			psw->portament = SeqPortaExecP;
		}
		else {
			psw->portament = SeqPortaExecN;
		}
		psw->tsta |= TSTA_INPORTA;
//		printf( "porta ==== %d\n",  (a<0?1:-1)*(psw->porvv>>XSF1) );
	}

	psw->lnote = note;

	{
		int len = psw->len;
		int a = 0;

		if ( !(psw->tatt & TATT_NOOFF) ) {
			a = ( ( len * psw->gate ) >> 12 );
			if ( a < 1 ) a = 1;
			else if ( a >= len ) a = len-1;
		}

		psw->stop = a;
	}
}

static void SeqNote2( SEQWK *psw )
{
	int note = *psw->pseq++;
	if ( note >= 0x80 ) note -= 0x80-12;
	SeqNote( psw, note );
}

static void SeqDrums( SEQWK *psw, int note )
{
	int np = psw->np;
	int ad = getadrs( drumsbase + (note<<1) );
	psw->nestd[np++] = 1;
	psw->nestd[np++] = psw->pseq - datatop;
	psw->nestd[np++] = ad;
	psw->pseq = datatop + ad;
	psw->np = np;
}


static void SeqRest( SEQWK *psw )
{
	if ( psw->run == 2 ) {
		NoteOff( &psw->mdwk );
		psw->run = 1;
	}
	psw->cnt = psw->len;
}

static void SeqInst( SEQWK *psw )
{
	SetInst( &psw->mdwk, *psw->pseq++ );
}

static void SeqTempo( SEQWK *psw )
{
	tempo = *psw->pseq++ << 4;
}

static void SeqEnd( SEQWK *psw )
{
	int np = psw->np;
	if ( !np ) {
		psw->run = 0;
		psw->cnt = 1;
		return;
	}

	np -= 3;
	if ( --psw->nestd[np] ) {
		psw->pseq = datatop + psw->nestd[np+2];
	}
	else {
		psw->pseq = datatop + psw->nestd[np+1];
		psw->np = np;
	}
}

static void SeqGate( SEQWK *psw )
{
	psw->gate = ( 100 - *psw->pseq++ ) * 41;
}

static void SeqJump( SEQWK *psw )
{
	psw->pseq = datatop + getadrs( psw->pseq );
}

static void SeqCall( SEQWK *psw )
{
	int np = psw->np;
	int ad = getadrs( psw->pseq );
	psw->nestd[np++] = psw->pseq[2];
	psw->nestd[np++] = psw->pseq+3 - datatop;
	psw->nestd[np++] = ad;
	psw->pseq = datatop + ad;
	psw->np = np;
}

static void SeqRept( SEQWK *psw )
{
	int np = psw->np;
	psw->nestd[np++] = *psw->pseq++;
	psw->nestd[np++] = psw->pseq - datatop;
	np++;
	psw->np = np;
}

static void SeqNext( SEQWK *psw )
{
	int np = psw->np - 3;
	if ( --psw->nestd[np] ) {
		psw->nestd[np+2] = psw->pseq - datatop;
		psw->pseq = datatop + psw->nestd[np+1];
	}
	else {
		psw->np = np;
	}
}

static void SeqBreak( SEQWK *psw )
{
	int np = psw->np - 3;
	if ( psw->nestd[np] == 1 ) {
		psw->pseq = datatop + psw->nestd[np+2];
		psw->np = np;
	}
}



static void SeqTrs( SEQWK *psw )
{
	psw->trs = (signed char)*psw->pseq++;
}



static void SeqEnv( SEQWK *psw )
{
	SetEnv( &psw->mdwk, psw->pseq[0], psw->pseq[1], psw->pseq[2], psw->pseq[3] );
	psw->pseq+=4;
}

static void SeqVol( SEQWK *psw )
{
	SetVol( &psw->mdwk, *psw->pseq++ );
}

static void SeqDtn( SEQWK *psw )
{
	psw->dtn = *psw->pseq++;
}

static void SeqPortaPara( SEQWK *psw )
{
	psw->porpd = *psw->pseq++;
	psw->porvv = gexp(*psw->pseq++ * 192 + 5*12*256) << XSF1;
	psw->porvv *= tempo;
	psw->porvv /= MCOUNT;
}

static void SeqPortaOn( SEQWK *psw )
{
	psw->tsta |= TSTA_PORTASW;
}

static void SeqPortaOff( SEQWK *psw )
{
	psw->tsta &= ~(TSTA_PORTASW|TSTA_INPORTA);
	psw->mdwk.ptwk1 = 0;
}

static void SeqTatt( SEQWK *psw )
{
	psw->tatt = *psw->pseq++;
}

static void SeqVibrato( SEQWK *psw )
{
	SetVib( &psw->mdwk, psw->pseq[0], psw->pseq[1], psw->pseq[2], psw->pseq[3] );
	psw->pseq+=4;
}

static void SeqPartFade( SEQWK *psw )
{
	psw->pseq++;
	psw->pseq++;
}

static void SeqMasterFade( SEQWK *psw )
{
	psw->pseq++;
	psw->pseq++;
}

static void SeqMasterVol( SEQWK *psw )
{
	psw->pseq++;
}

static void SeqBend( SEQWK *psw )
{
	psw->pseq++;
	psw->pseq++;
}

static void SeqLegOn( SEQWK *psw )
{
	psw->tsta |= (TSTA_LEG1|TSTA_LEG2);
}

static void SeqLegOff( SEQWK *psw )
{
	psw->tsta &= ~TSTA_LEG1;
}

static void SeqExp( SEQWK *psw )
{
	SetExp( &psw->mdwk, *psw->pseq++ );
}

static void SeqExpRel( SEQWK *psw )
{
	SetExpRel( &psw->mdwk, *(signed char *)psw->pseq++ );
}



void (*SeqCmd[32])( SEQWK *psw ) = {
	SeqRest,		// 0xe0
	SeqGate,		// 0xe1
	SeqJump,		// 0xe2
	SeqCall,		// 0xe3
	SeqRept,		// 0xe4
	SeqNext,		// 0xe5
	SeqTrs,			// 0xe6
	SeqTempo,		// 0xe7
	SeqInst,		// 0xe8
	SeqVol,			// 0xe9
	SeqEnv,			// 0xea
	SeqDtn,			// 0xeb
	SeqNote2,		// 0xec
	SeqPortaPara,	// 0xed
	SeqPortaOn,		// 0xee
	SeqPortaOff,	// 0xef

	SeqTatt,		// 0xf0
	SeqVibrato,		// 0xf1
	SeqMasterVol,	// 0xf2
	SeqMasterFade,	// 0xf3
	SeqPartFade,	// 0xf4
	SeqBend,		// 0xf5
	SeqBreak,		// 0xf6
	SeqNop,			//
	SeqLegOn,		// 0xf8
	SeqLegOff,		// 0xf9
	SeqExp,			// 0xfa
	SeqExpRel,		// 0xfb
	SeqNop,
	SeqNop,
	SeqNop,
	SeqNop,
};



int ProcSeq( void )
{
	SEQWK *psw;
	int i;

	for ( i = 0,psw = seqwk; i < MAXSEQ; i++,psw++ ) {
		if ( psw->run ) {
			if ( psw->tsta & TSTA_INPORTA ) psw->portament( psw );
		}
	}

	if ( ( tempowk -= tempo ) >= 0 ) return runf;

	tempowk += (short)MCOUNT;//MCOUNT; // SK : グローバル変数化に伴うキャスト

	runf = 0;

	for ( i = 0,psw = seqwk; i < MAXSEQ; i++,psw++ ) {
		runf += psw->run;
		if ( psw->run ) {
			if ( !--psw->cnt ) {
				do {
					int a = *psw->pseq++;
					if ( a >= 0xe0 ) {
						(SeqCmd[a-0xe0])( psw );
					}
					else if ( a >= 0x80 ) {
						if ( psw->tatt & TATT_DRUMS ) {
							SeqDrums( psw, a-0x80 );
						}
						else {
							SeqNote( psw, a-0x80+12 );
						}
					}
					else if ( a ) {
						if ( a == 127 ) {
							a = *psw->pseq++;
							if ( a < 127 ) a += 256;
						}
						psw->len = a;
					}
					else {
						SeqEnd( psw );
					}
				} while ( !psw->cnt );
			}
			else {
				if (psw->cnt==psw->stop && psw->run==2 && !(psw->tsta & TSTA_LEG1)) {
					NoteOff( &psw->mdwk );
					psw->run = 1;
				}
			}
		}
	}

	return runf;
}

