#include "wave_reader.h"

void WaveReader::close()
{
    if( fp )
    {
        fclose( fp );
        fp = nullptr;
    }
}

result WaveReader::loadWave( lpcstr fileName )
{
    result ret = -1;

    fp = fopen( fileName, "rb" );
    if( fp )
    {
        if( readFormat( fp ) == 0 )   // if succeeded, we should be at some chunk
        {
            if( isFormatSupported() ) // format ok?
            {
                if( findData( fp ) == 0 ) // locate data chunk
                {
                    ret = 0;
                }
            }
        }

        if( ret != 0 )
            close();
    }

    return ret;
}

result WaveReader::readFormat( FILE* fp )
{
    result ret = -1;
    dword fcc;

    size_t read = fread( &fcc, sizeof(dword), 1, fp );
    if( read == 1 && fcc == 0x46464952 ) // 'RIFF'
    {
        dword length;
        read = fread( &length, sizeof(dword), 1, fp ); // RIFF length
        if( read == 1 && length > 0 )
        {
            read = fread( &fcc, sizeof(dword), 1, fp ); // WAVE fcc
            if( read == 1 && fcc == 0x45564157 ) // 'WAVE'
            {
                wav_chunk chk;
                read = fread( &chk, sizeof(wav_chunk), 1, fp ); // fmt chunk
                if( read == 1 && chk.fcc == 0x20746D66 && chk.length >= sizeof(wave_format) ) // 'fmt '
                {
                    // read wave format
                    read = fread( &fmt, sizeof(wave_format), 1, fp );
                    if( read == 1 )
                    {
                        ret = 0; // we are ok at this point

                        if( chk.length > sizeof(wave_format) ) // check for extended format
                        {
                            dword extSize = chk.length - sizeof(wave_format);
                            if( fseek( fp, extSize, SEEK_CUR ) != 0 ) // skip all extra informatin
                            {
                                ret = -1;
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

result WaveReader::findData( FILE* fp )
{
    result ret = -1;

    wav_chunk chk = { 0 };

    for( ; ; )
    {
        size_t read = fread( &chk, sizeof(wav_chunk), 1, fp );
        if( read == 1 && chk.fcc == 0x61746164 ) // 'data'
        {
            dataLen = chk.length;  // store data chunk length
            dataPos = ftell( fp ); // store file position of the data chunk
            ret = 0;   // all success
            break;
        }
        
        if( read <= 0 ) // error or eof
            break;

        // else advance to next chunk
        if( fseek( fp, chk.length, SEEK_CUR ) != 0 )
        {
            break;
        }
    }

    return ret;
}

bool WaveReader::isFormatSupported()  // return true if format is supported
{
    if( fmt.tag != 1 ) // PCM
        return false;

    if( fmt.channels > 2 )
        return false;

    if( fmt.samplesPerSec != 44100 && fmt.samplesPerSec != 48000 && 
        fmt.samplesPerSec != 11025 && fmt.samplesPerSec != 22050 )
    {
        return false;
    }

    if( fmt.bitsPerSample != 16 && fmt.bitsPerSample != 8 && fmt.bitsPerSample != 24 )
        return false;

    return true;
}

int WaveReader::getPcmData( void* buffer, int length )
{
    int readBytes = 0;
    
    if( fp )
    {
        //uint32_t start = esp_timer_get_time();
        readBytes = fread( buffer, 1, length, fp );
        //uint32_t elapced = esp_timer_get_time() - start;
        //TRACE( "[WAV]: %u mcs\n", elapced ); // gives 27-33 ms
    }

    return readBytes;
}

// moves the file pointer to the beginning of data chunk
void WaveReader::rewind()
{
    fseek( fp, dataPos, SEEK_SET );
}