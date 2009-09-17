
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
//  音源(音色)定義ファイルのサンプル
//
//  v1.00 2001.11.09 MIO.H
//



#include <musdef.h>

extern INST i_square0;
extern INST i_saw0;
extern INST i_triangle0;
extern INST i_square;
extern INST i_saw;
extern INST i_triangle;

#if 0
extern INST i_d002;
extern INST i_d003;
extern INST i_d004;
extern INST i_d005;
extern INST i_d006;
extern INST i_d007;
extern INST i_d008;
extern INST i_d009;
extern INST i_d010;
extern INST i_d011;
extern INST i_d012;

INST *inst[] = {
	&i_square0,		// 0
	&i_saw0,		// 1
	&i_triangle0,	// 2
	&i_square,		// 3
	&i_saw,			// 4
	&i_triangle,	// 5
	&i_d002, // 6  bdr
	&i_d003, // 7  sdr
	&i_d004, // 8  rim
	&i_d005, // 9  ohh
	&i_d006, // 10 chh
	&i_d007, // 11 ccy
	&i_d008, // 12 rcy
	&i_d009, // 13 htm
	&i_d010, // 14 mtm
	&i_d011, // 15 ltm
	&i_d012, // 16 hcp
};
#else
extern INST i_BD909;
extern INST i_CYMBD;
extern INST i_HANDCLAP;
extern INST i_HC909;
extern INST i_HO909;
extern INST i_SD909;
extern INST i_SDGATE;
extern INST i_TOMH1;
extern INST i_TOML1;
extern INST i_TOMM1;

INST *inst[] = {
	&i_square0,		// 0
	&i_saw0,		// 1
	&i_triangle0,	// 2
	&i_square,		// 3
	&i_saw,			// 4
	&i_triangle,	// 5
	&i_BD909, // 6  bdr
	&i_SDGATE, // 7  sdr
	&i_SD909, // 8  rim
	&i_HO909, // 9  ohh
	&i_HC909, // 10 chh
	&i_CYMBD, // 11 ccy
	&i_CYMBD, // 12 rcy
	&i_TOMH1, // 13 htm
	&i_TOMM1, // 14 mtm
	&i_TOML1, // 15 ltm
	&i_HANDCLAP, // 16 hcp
};
#endif

