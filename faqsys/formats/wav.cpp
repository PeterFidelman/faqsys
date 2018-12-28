This section is from the document '/SPIB/news/comp.speech/1994.03'.

From rice!newsfeed.rice.edu!cs.utexas.edu!usc!aludra.usc.edu!not-for-mail Sun Feb 19 23:35:22 1994 
Subject: Re: wav file format
Date: 19 Feb 1994 23:35:22 -0800
References: <9402191956.2D37F0@aaa.vsrc.uab.edu>

Here is the source files in C/C++ for reading .wav files.

There are two files, one is wavefile.cpp; another one is wavefile.hpp.
In the declaration of the struct WaveFile, it is a bit clumsy.  But
I have conformed it to the Microsoft Window's PCM format.  So for the
most compatibility, it is the best not to change it.  There are not
much comments in the code, but I am sure you can figure out from the
code itself.

I can be reached at yunghsia@scf.usc.edu.  Any comments or questions
welcomed.

/*
        File: WaveFile.Hpp

        By Yung-hsiang Lee, Univ. of Southern California

        All Copyright reserved.

        Header file for .Wav File I/Os

        Distribution of the source file is permitted in original form.
          Modified version of the source file distribution is permitted
          if author's name is retained.

          Further modification or incorporation into other programs are 
          allowed without obtaining further agreement from author.

*/
#ifndef _WAVEFILE_
  #define _WAVEFILE_

#define WAVE_FORMAT_PCM 1

typedef struct {
/* File header */
  char riff[4];
  long filesize;
  char rifftype[4];
} RiffHeader;

typedef struct {
  char chunk_id[4];
  long chunksize;
} Chunk; 

typedef struct {
  short wFormatTag;
  short nChannels;
  long nSamplesPerSec;
  long nAvgBytesPerSec;
  short nBlockAlign;
} WAVEFORMAT;

typedef struct {
  WAVEFORMAT wf;
  short wBitsPerSample;
} PCMWAVEFORMAT;

typedef struct {
  RiffHeader header;
  Chunk ch_format;
  PCMWAVEFORMAT format;
  Chunk ch_data;
  // Data follows here.
} WaveFile;

void WaveFile_Init(WaveFile *wf);

void WaveFile_Set(WaveFile *wf,
  short channels,
  long samplerate,
  short datasize);

void WaveFile_Fin(WaveFile *wf,
  short channels,
  long samplerate,
  short datasize,
  long totalbyte);

short WaveFile_Write(const char *filename, WaveFile *wf, void *data);

short WaveFile_Read(const char *filename, WaveFile *wf, void **data);

float wave_length(WaveFile *wf);

#endif _WAVEFILE_
/*
        File: WaveFile.Cpp

        By Yung-hsiang Lee, Univ. of Southern California

        All Copyright reserved.

        Source file for .Wav File I/Os

        Distribution of the source file is permitted in original form.
          Modified version of the source file distribution is permitted
          if author's name is retained.

          Further modification or incorporation into other programs are 
          allowed without obtaining further agreement from author.

*/
#include <mem.h>
#include <stdlib.h>
#include <stdio.h>
#include "wavefile.hpp"

void WaveFile_Init(WaveFile *wf)
{
  memcpy(wf->header.riff , (const void *)"RIFF", 4);
  memcpy(wf->header.rifftype, (const void *)"WAVE", 4);
  memcpy(wf->ch_format.chunk_id, (const void *)"fmt ", 4);
  wf->ch_format.chunksize=16;
  wf->format.wf.wFormatTag=WAVE_FORMAT_PCM;
  memcpy(wf->ch_data.chunk_id, (const void *)"data", 4);
};

void WaveFile_Set(WaveFile *wf,
  short channels,
  long samplerate,
  short datasize)
{
  WaveFile_Init(wf);
  wf->format.wf.nChannels=channels;
  wf->format.wf.nSamplesPerSec=samplerate;
  wf->format.wf.nAvgBytesPerSec=samplerate;
  wf->format.wf.nBlockAlign=datasize*channels/8;
  wf->format.wBitsPerSample=datasize;
};

void WaveFile_Fin(WaveFile *wf,
  short channels,
  long samplerate,
  short datasize,
  long totalsamples)
{
  WaveFile_Set(wf, channels, samplerate, datasize);
  wf->ch_data.chunksize=totalsamples * wf->format.wf.nBlockAlign;
  wf->header.filesize=wf->ch_data.chunksize + 36;
};

short WaveFile_Write(const char *filename, WaveFile *wf, void *data)
{
  FILE *wfile;

  if( ( wfile=fopen(filename, "wb") )!=NULL ) {
    fwrite( wf, sizeof(WaveFile), 1, wfile);
    fwrite( data, wf->ch_data.chunksize , 1, wfile);
    fclose(wfile);
    return(1);
  } else {
//    printf("Error in opening the file!\n");
    return(0);
  }
}

float wave_length(WaveFile *wf)
/* Return wave length in secs. */
{
  float len=(wf->ch_data.chunksize /
    wf->format.wf.nChannels) /
    (float)(wf->format.wf.nSamplesPerSec);
  return(len);
};

short WaveFile_Read(const char *filename, WaveFile *wf, void **data)
{
  FILE *wfile;

  if( ( wfile=fopen(filename, "rb") )!=NULL ) {
    fread( wf, sizeof(WaveFile), 1, wfile);
    *data=(char*)malloc(wf->ch_data.chunksize);
    fread( *data, wf->ch_data.chunksize , 1, wfile);
    fclose(wfile);
    return(1);
  } else {
//    printf("Error in opening the file!\n");
    return(0);
  }
}

