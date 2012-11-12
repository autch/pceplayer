
#include <stdio.h>
#include <stdlib.h>

#include "muslib/muslib.h"
#include "muslib/musdef.h"
#include "muslib/musdefwin.h"
#include "muslib/muslib.h"

// muslib の仕様によって定まる、一度に render() 出来るバッファサイズ
#define RENDER_UNIT (128 * sizeof(signed short))
#define SAMPLING_FREQ	(48000)

volatile PCEWAVEINFO *pWave; // 唯一のインスタンス
long FS, FS2, MCOUNT, FOFS;	 // Yui: musdef.h から移してきたグローバル変数
// SK : muslib の可変周波数化に伴う、各種グローバル変数
extern long VOLS, ENVSTART, ENVSTOP, ENVRR;
extern unsigned char finish;	// mus.c

unsigned char* m_pseq;


// P/ECE カーネル API エントリポイント
int pceWaveDataOut(int ch, PCEWAVEINFO *pwave)
{
	pWave = pwave;

	// g_fReady = 1;
	return 0;
}

void muslib_init()
{
	pWave = NULL;
	m_pseq = NULL;
	finish = 0;
}


void muslib_start()
{
	unsigned char first, second;

	pWave = NULL;
	if(!m_pseq) return;

	// フォーマットチェック
	first = *m_pseq;
	second = *(m_pseq + 1);
	if(first > 0x0f) return;
	if(first == 0x00 && second > 0x0f)
		return;

	// SK : muslib の可変周波数化に伴う、各種グローバル変数の初期化
	FS = 48000 << 1;
	FS2 = FS >> 1;

	//MCOUNT
	//(long)(FS2/128.0*2.5*16.0+0.5) == (long)((FS2*40)/128+0.5) == (long)(FS2*40.0+64.0)/128.0)
	//(FS2*40+64)/128
	//MCOUNT = (FS2 * 40 + 64) >> 7;				// yui: 組み込み実数演算を利用
	MCOUNT = 15000;

	//FOFS = (long)(auLog(FS2) / auLog(2.0) * 12.0 * 256.0 + 0.5)
	//     ==(long)(3072*auLog(FS2)/auLog(2.0)+1/2);
	//FOFS = (long)(3072.0 * log((double)FS2) / log(2.0) + 0.5);
	FOFS = 47772;

	VOLS = 1;

	// 2003.08.07 Yui N.
	// ENVSTART と ENVSTOP はレート値ではないので Freq. による調整は不要！
	ENVSTART = ((-4 * 12 * 256) << VOLS);
	ENVSTOP = ((-10 * 12 * 256) << VOLS);

	// ENVRR はその名のとおりレート値なので調整が必要
	// Freq. が上がったら増やすのではなくて減らす
	// X / (fs2 / 16k) = (X / 1) / (fs2 / 16k) = 16k * x / fs2 * 1
	//ENVRR = (double)(4000 << VOLS) / ((double)FS2 / 16000.0);
	//ENVRR = MulDiv(4000 << VOLS, 16000, FS2);
	ENVRR = 2666;
	// SK : ここまで

	// yui: 曲の再生時に detune と vibrato の初期化をする
	InitMusic();
	PlayMusic(m_pseq);
}

void muslib_stop()
{
	StopMusic();
	pWave = NULL;
}

void muslib_close()
{
	muslib_stop();

	if(m_pseq)
	{
		free(m_pseq); m_pseq = NULL;
	}
}

int muslib_load_from_file(const char *filename)
{
	int fsize, dwBytesRead;
	FILE* fp;

	if(m_pseq) muslib_close();

	fp = fopen(filename, "rb");
	if(!fp) return 0;

	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	rewind(fp);
	m_pseq = malloc(fsize);
	fread(m_pseq, 1, fsize, fp);
	fclose(fp);

	return 1;
}

int muslib_load_from_buffer(const unsigned char* source, int size)
{
	if(m_pseq) muslib_close();
	m_pseq = malloc(size);
	memcpy(m_pseq, source, size);
	return 1;
}

int muslib_render(char* buffer, int size)
{
	int bytesToRender = size, bytesRendered = 0;

	if(!finish)
	{
		do
		{
			if(pWave)
			{
				if(buffer && pWave->pData)
				{
					memcpy(buffer + bytesRendered, pWave->pData, RENDER_UNIT);
				}
				pWave->pfEndProc(pWave);
			}
			bytesToRender -= RENDER_UNIT;
			bytesRendered += RENDER_UNIT;
		}while(bytesToRender > 0);
	}

	return bytesRendered;
}
