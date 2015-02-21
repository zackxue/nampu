
#ifndef __WAV_HEADER_H__
#define __WAV_HEADER_H__

#include <Windows.h>

struct tagHXD_WAVFLIEHEAD
{
	CHAR RIFFNAME[4];
	DWORD nRIFFLength;
	CHAR WAVNAME[4];
	CHAR FMTNAME[4];
	DWORD nFMTLength;
	WORD nAudioFormat;
	WORD nChannleNumber;
	DWORD nSampleRate;
	DWORD nBytesPerSecond;
	WORD nBytesPerSample;
	WORD nBitsPerSample;
	CHAR DATANAME[4];
	DWORD nDataLength;
};

void copy_wav_header(FILE *file)
{
	struct tagHXD_WAVFLIEHEAD DestionFileHeader;
	DestionFileHeader.RIFFNAME[0] = 'R';
	DestionFileHeader.RIFFNAME[1] = 'I';
	DestionFileHeader.RIFFNAME[2] = 'F';
	DestionFileHeader.RIFFNAME[3] = 'F';

	DestionFileHeader.WAVNAME[0] = 'W';
	DestionFileHeader.WAVNAME[1] = 'A';
	DestionFileHeader.WAVNAME[2] = 'V';
	DestionFileHeader.WAVNAME[3] = 'E';

	DestionFileHeader.FMTNAME[0] = 'f';
	DestionFileHeader.FMTNAME[1] = 'm';
	DestionFileHeader.FMTNAME[2] = 't';
	DestionFileHeader.FMTNAME[3] = 0x20;
	DestionFileHeader.nFMTLength = 16; // 真 FMT 真?   DestionFileHeader.nAudioFormat = 6; //真真a law PCM
	DestionFileHeader.nAudioFormat = 6;//6;//0x01;

	DestionFileHeader.DATANAME[0] = 'd';
	DestionFileHeader.DATANAME[1] = 'a';
	DestionFileHeader.DATANAME[2] = 't';
	DestionFileHeader.DATANAME[3] = 'a';
	DestionFileHeader.nBitsPerSample = 16;
	DestionFileHeader.nBytesPerSample = 1;//2; //
	DestionFileHeader.nSampleRate = 8000; //
	DestionFileHeader.nBytesPerSecond = 8000;//16000;
	DestionFileHeader.nChannleNumber = 1;

	if(file != NULL)
	{
		if(file)
		{
			//bFir = false;
			fwrite(&DestionFileHeader, 1, sizeof(DestionFileHeader), file);
		}
	}
}

#endif