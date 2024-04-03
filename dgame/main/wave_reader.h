#pragma once
#include "main.h"

typedef struct wave_format
{
    short   tag;               // format type
    short   channels;          // number of channels (i.e. mono, stereo...)
    dword   samplesPerSec;     // sample rate, Hz
    dword   avgBytesPerSec;    // (sampleRate * bitsPerSample * channels) / 8.
    short   blockAlign;        // (bitsPerSample * Channels) / 8,  1 - 8 bit mono, 2 - 8 bit stereo/16 bit mono, 4 - 16 bit stereo
    short   bitsPerSample;     // number of bits per sample of mono data
} wave_format;

typedef struct wav_chunk
{
    dword    fcc;
    dword    length;
} wav_chunk;

class WaveReader
{
    FILE*           fp = nullptr;       // file to read from
    wave_format     fmt = { 0 };        // format

    int             dataLen = 0;        // size of data 
    int             dataPos = 0;        // position in file of data samples

    result readFormat( FILE* fp ); // try to read wave format
    bool   isFormatSupported();    // return true if format is supported
    result findData( FILE* fp );   // try to find data chunk
    
public:
    // opens wave file, returns 0 if format is supported
    result loadWave( lpcstr fileName );

    // return the wave format of the current file
    const wave_format& getFormat() const { return fmt; }

    // return the nummber of channels
    int  getChannels() const { return fmt.channels; }

    // return the length of the PCM data in bytes
    int  getDataLength() const { return dataLen; }
    
    // input accepts pointer to buffer and the buffer length in samples
    // returns buffer filled with samples and sumples count, 0 if eof is reached
    int getPcmData( void* buffer, int length );

    // moves the file pointer to the beginning of data chunk
    void rewind();

    void close(); // close wave file
};

