// Copyright (c) 2012 Anne Aaron, Milan Stevanovic, Pradip Gajjar.  All rights reserved.
// avxanne@gmail.com, avxsynth@gmail.com, pradip.gajjar@gmail.com
// http://www.avxsynth.org


// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.auto

#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include "avxiface.h"
#include "avxplugin.h"
#include "utilities.h"

#include "frameserverlib.h"

namespace avxsynth
{

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif 

#define MODULE_NAME appInterface

#define BITS_PER_BYTE   (8)

#define SAFE_DELETE(x)              if((x)){ delete (x); (x) = NULL; }
#define SAFE_DELETE_ARRAY(x)        if((x)){ delete [] (x); (x) = NULL; }

#define SHELL "/bin/sh"
typedef void* PLUGIN_HANDLE;

typedef struct
{
    bool            isMPlayerLaunchRequired;
    bool            isVideoFieldBased;
    IAVIStream*     pAVIStream;
    long            nFramesSoFar;
    AvisynthError*  pLastError;
} PROCESS_STREAM_INFO, *PPROCESS_STREAM_INFO;

void printScript(const char* pStrScriptFilename)
{   
    FILE* fpTemp = fopen(pStrScriptFilename, "rb");
    if(NULL == fpTemp)
    {
      AVXLOG_ERROR("Failed opening %s", pStrScriptFilename);
      return;
    }
        
    char chStrLine[FILENAME_MAX];
    while(fgets(chStrLine, FILENAME_MAX, fpTemp))
    {
      char* pTemp = strchr(chStrLine, '\n');
      if(pTemp)
        *pTemp = 0;
      AVXLOG_INFO("%s", chStrLine);
    }
}

void printVideoInfo(BITMAPINFOHEADER bih)
{
    AVXLOG_INFO("%s", "Retrieved video info");
    AVXLOG_INFO("%s", "=======================");
    AVXLOG_INFO("Video width    = %d", bih.biWidth);
    AVXLOG_INFO("Video height   = %d", bih.biHeight);
  
    switch(bih.biCompression)
    {
    case BI_RGB:    // 0  
      AVXLOG_INFO("%s", "Color format   = BI_RGB");
      break;
    case BI_RLE8:   // 1
      AVXLOG_INFO("%s", "Color format   = BI_RLE8");
      break;
    case BI_RLE4:       // 2
      AVXLOG_INFO("%s", "Color format   = BI_RLE4");
      break;
    case BI_BITFIELDS:  // 3
      AVXLOG_INFO("%s", "Color format   = BI_BITFIELDS");
      break;
    case BI_JPEG:       // 4
      AVXLOG_INFO("%s", "Color format   = BI_JPEG");
      break;
    case BI_PNG:    // 5
      AVXLOG_INFO("%s", "Color format   = BI_PNG");
      break;
    default:    
      unsigned char *fourCC = (unsigned char *) (&bih.biCompression);
      AVXLOG_INFO("Color format   = %c%c%c%c", fourCC[0], fourCC[1], fourCC[2], fourCC[3] );
      break;      
    }
    AVXLOG_INFO("Bits per pixel = %d", bih.biBitCount);
    AVXLOG_INFO("Image bytes    = %d", bih.biSizeImage);
  
}

void printAudioInfo(void* pAudioInfo, long cbFormat)
{
    switch(cbFormat)
    {
    case sizeof(WAVEFORMATEX):
        {
            AVXLOG_INFO("%s", "Retrieved audio info (WAVEFORMATEX)");
            WAVEFORMATEX* pWfx = (WAVEFORMATEX*)pAudioInfo;
            AVXLOG_INFO(" wFormatTag      = 0x%02X", pWfx->wFormatTag);
            AVXLOG_INFO(" nChannels       = %d",     pWfx->nChannels);
            AVXLOG_INFO(" nSamplesPerSec  = %d", pWfx->nSamplesPerSec);
            AVXLOG_INFO(" nAvgBytesPerSec = %d", pWfx->nAvgBytesPerSec);
            AVXLOG_INFO(" nBlockAlign     = %d", pWfx->nBlockAlign);
            AVXLOG_INFO(" wBitsPerSample  = %d", pWfx->wBitsPerSample);
            AVXLOG_INFO(" cbSize          = %d", pWfx->cbSize);
        }
        break;
    case sizeof(WAVEFORMATEXTENSIBLE):
        AVXLOG_INFO("%s", "Retrieved audio info (WAVEFORMATEXTENSIBLE)");
        break;
    default:
        break;
    }
}

bool retrieveVideoSettings(IAVIStream* pAVIStream, BITMAPINFOHEADER& bih)
{
    if(NULL == pAVIStream)
      return false;
    
    LONG   cbFormat = 0;
    pAVIStream->ReadFormat(0, NULL, &cbFormat);
    if(0 == cbFormat)
    {
        AVXLOG_ERROR("%s", "Unknown media format byte size");
        return false;
    }
    
    if(sizeof(BITMAPINFOHEADER) != cbFormat)
    {
        AVXLOG_ERROR("%s", "Unknown video format specifier (size != sizeof(BITMAPINFOHEADER))");
        return false;
    }
    
    memset(&bih, 0, sizeof(BITMAPINFOHEADER));
    
    pAVIStream->ReadFormat(0, &bih, &cbFormat);
    printVideoInfo(bih);
    return true;
}

bool retrieveAudioSettings(IAVIStream* pAVIStream, void* & pAudioInfo, LONG & cbFormat)
{
    if(NULL == pAVIStream)
      return false;
    
    pAVIStream->ReadFormat(0, NULL, &cbFormat);
    if(0 == cbFormat)
    {
        AVXLOG_ERROR("%s", "Unknown media format byte size");
        return false;
    }
    
    switch(cbFormat)
    {
    case sizeof(WAVEFORMATEX):
        {
            pAudioInfo = new WAVEFORMATEX;
            memset(pAudioInfo, 0, sizeof(WAVEFORMATEX));
            pAVIStream->ReadFormat(0, pAudioInfo, &cbFormat);
            printAudioInfo(pAudioInfo, cbFormat);
        }
        break;
    case sizeof(WAVEFORMATEXTENSIBLE):
        {
            pAudioInfo = new WAVEFORMATEXTENSIBLE;
            memset(pAudioInfo, 0, sizeof(WAVEFORMATEXTENSIBLE));
            pAVIStream->ReadFormat(0, pAudioInfo, &cbFormat);
            printAudioInfo(pAudioInfo, cbFormat);
        }
        break;
    default:
        AVXLOG_ERROR("%s", "Unexpected audio format specifier byte length");
        return false;
        break;
    }
    
    return true;
}

bool launchMPlayerToRenderVideo(BITMAPINFOHEADER const& bih)
{  
    // Execute mplayer process
    // mplayer -demuxer rawvideo -rawvideo w=$1:h=$2:format=yv12 -
    
    char command[1024];
    char colorspace[6];
    
    bool bRGBFlippingRequired = false;
    switch(bih.biCompression)
    {
    case 0x32315659:        // '21VY'
        sprintf(colorspace, "yv12");                       
        break;
    case 0x32595559:        // '2YUY
        sprintf(colorspace, "yuy2");
        break;
    case BI_RGB:            // RGB is 0
        bRGBFlippingRequired = true;
        sprintf(colorspace, "bgr%d", bih.biBitCount);
        break;
    default:
        AVXLOG_ERROR("Invalid colorspace %d", bih.biCompression);
        return false;                                                        
    }
    
    sprintf(command, "mplayer %s -demuxer rawvideo -rawvideo w=%d:h=%d:format=%s - 1> /dev/null", 
             bRGBFlippingRequired ? "-flip" : "", bih.biWidth, bih.biHeight, colorspace);
    if (-1 == execl(SHELL, SHELL, "-c", command, NULL))
    {
        AVXLOG_ERROR("%s", "Error launching mplayer");
        return false;
    }
    return true;
}

bool launchMPlayerToRenderAudio(void* pAudioFormat)
{  
    // Execute mplayer process
    // mplayer -demuxer rawvideo -rawvideo w=$1:h=$2:format=yv12 -
    
    char command[1024];
    char colorspace[6];
        
    WAVEFORMATEX* pWfx = (WAVEFORMATEX*)pAudioFormat;
    AVXLOG_INFO("Setting up mplayer to render %d channels, %d bytes per sample, at %d/sec\n", pWfx->nChannels, pWfx->wBitsPerSample/BITS_PER_BYTE, pWfx->nSamplesPerSec);
    // mplayer -demuxer rawaudio -rawaudio channels=1:rate=44100:samplesize=2:format=0
    sprintf(command, "mplayer -demuxer rawaudio -rawaudio channels=%d:rate=%d:samplesize=%d:format=0 - 1> /dev/null", 
            pWfx->nChannels, pWfx->nSamplesPerSec, pWfx->wBitsPerSample/BITS_PER_BYTE);
    AVXLOG_INFO("mplayer command line: \"%s\"", command);
    if (-1 == execl(SHELL, SHELL, "-c", command, NULL))
    {
        AVXLOG_ERROR("%s", "Error launching mplayer");
        return false;
    }
    return true;
}

bool launchVideoStreamRecipientProcess(BITMAPINFOHEADER const& bih, pid_t *mplayerpid)
{
    int commpipe[2];
    pid_t pid = 0;
  
    if(pipe(commpipe) == -1)
    {    
        AVXLOG_ERROR("%s", "Error creating pipe");
        return false;
    }                
              
    pid = fork();
    switch(pid)
    {
    case -1:
        AVXLOG_ERROR("%s", "Error creating fork");
        return false;
    case 0:
        // Child process
        close(commpipe[1]);                             // Close write end of pipe
        if (dup2(commpipe[0], STDIN_FILENO) == -1)      // Assign stdin to read end of pipe
        {
            AVXLOG_ERROR("%s", "Failure in dup2 of child stdin");
            return false;                            
        }
        // Redirect stdout to stderr
        if (dup2(STDERR_FILENO, STDOUT_FILENO) == -1)   // Assign stdout to stderr
        {
            AVXLOG_ERROR("%s", "Failure in dup2 of child stdout");
            return false;                            
        }                  
        
        //
        // Our default stream recipient is mplayer (but there may be many others, encoders, etc)
        // 
        if(false == launchMPlayerToRenderVideo(bih))
            return false;
        
        break; 
    default:                                                
        // Parent process
        *mplayerpid = pid;
        close(commpipe[0]);                             // Close read end of pipe
        if (dup2(commpipe[1], STDOUT_FILENO) == -1)     // Assign stdout to write end of pipe
        {
            AVXLOG_ERROR("%s", "Failure in dup2 of parent stdout");
            return false;                            
        }
        break;                         
    }
          
    return true;
}

bool launchAudioStreamRecipientProcess(void* pAudioInfo, pid_t *mplayerpid)
{
    int commpipe[2];
    pid_t pid = 0;
  
    if(pipe(commpipe) == -1)
    {    
        AVXLOG_ERROR("%s", "Error creating pipe");
        return false;
    }                
              
    pid = fork();
    switch(pid)
    {
    case -1:
        AVXLOG_ERROR("%s", "Error creating fork");
        return false;
    case 0:
        // Child process
        close(commpipe[1]);                             // Close write end of pipe
        if (dup2(commpipe[0], STDIN_FILENO) == -1)      // Assign stdin to read end of pipe
        {
            AVXLOG_ERROR("%s", "Failure in dup2 of child stdin");
            return false;                            
        }
        // Redirect stdout to stderr
        if (dup2(STDERR_FILENO, STDOUT_FILENO) == -1)   // Assign stdout to stderr
        {
            AVXLOG_ERROR("%s", "Failure in dup2 of child stdout");
            return false;                            
        }                  
        
        //
        // Our default stream recipient is mplayer (but there may be many others, encoders, etc)
        // 
        if(false == launchMPlayerToRenderAudio(pAudioInfo))
            return false;
        
        break; 
    default:                                                
        // Parent process
        *mplayerpid = pid;
        close(commpipe[0]);                             // Close read end of pipe
        if (dup2(commpipe[1], STDOUT_FILENO) == -1)     // Assign stdout to write end of pipe
        {
            AVXLOG_ERROR("%s", "Failure in dup2 of parent stdout");
            return false;                            
        }
        break;                         
    }
          
    return true;
}

void TerminateProcess(pid_t pid)
{
    char cmd[256];
    sprintf(cmd, "kill -9 %d 2> /dev/null", pid);
    system(cmd);
}

void* ProcessVideoStream(void* pArgument) // very likely to end up being pthread function
{
    PROCESS_STREAM_INFO* pInfo = (PROCESS_STREAM_INFO*)pArgument;
    if(NULL == pInfo)
    {
        pInfo->pLastError = new AvisynthError("Passed NULL address of PROCESS_STREAM_INFO structure");
        return pInfo;
    }
                        
    //
    // Examine video format details:
    // 
    BITMAPINFOHEADER bih;
    if(false == retrieveVideoSettings(pInfo->pAVIStream, bih))
    {
        AVXLOG_ERROR("%s", "Failed retrieved video settings");
        pInfo->pLastError = new AvisynthError("Failed retrieving video settings");        
        return pInfo;
    }

    //
    // Allocate image buffer
    //
    long nImageBytes   = bih.biSizeImage;
    char* pImageBuffer = new char[nImageBytes];
    if(NULL == pImageBuffer)
    {
        AVXLOG_ERROR("%s", "Failed allocating %ld bytes for image buffer", nImageBytes);
        pInfo->pLastError = new AvisynthError("Failed allocating memory for image buffer");        
        return pInfo;
    }
                  
    //const long nMaxFrames = (pAVIStream->GetVideoInfo()).nun_frames;
    HRESULT hr;  
    int nWrittenImageBytes = -1;
    pid_t mplayerpid = -1;
    while(1)
    {
        int32_t nNextFrameStreamPos = pInfo->pAVIStream->FindSample(pInfo->nFramesSoFar, FIND_KEY);
        if(-1 == nNextFrameStreamPos)
        {
            AVXLOG_ERROR("%s", "Invalid stream position");
            pInfo->pLastError = new AvisynthError("Invalid stream position");
            break;
        }
              
        AVXLOG_DEBUG("Preparing to fetch frame #%ld (%d frame of the stream)", pInfo->nFramesSoFar, nNextFrameStreamPos);
        memset(pImageBuffer, 0, bih.biSizeImage*sizeof(char));
              
        int32_t nBytesRead = 0;
        int32_t nSamplesRead = 0;
        int32_t nSamplesToRead = 1;
        hr = pInfo->pAVIStream->Read(nNextFrameStreamPos, nSamplesToRead, (void*)pImageBuffer, nImageBytes, &nBytesRead, &nSamplesRead);
        if (nBytesRead==0 && nSamplesRead==0 && !FAILED(hr))
        {
            AVXLOG_ALERT("Reached end of stream at frame #%02ld", pInfo->nFramesSoFar);
            break;
        }
        if((FAILED(hr)) || 
           (nBytesRead != nImageBytes) || 
           (nSamplesRead != nSamplesToRead)
          )
        {
            AVXLOG_ERROR("Error in reading frame %02ld: nSamplesRead=%d, nBytesRead=%d, nImageBytes=%ld", pInfo->nFramesSoFar, nSamplesRead, nBytesRead, nImageBytes);
            pInfo->pLastError = new AvisynthError("Invalid stream position");
            break;
        }
              
        if(pInfo->isMPlayerLaunchRequired && 0 == pInfo->nFramesSoFar)
        {
            if(false == launchVideoStreamRecipientProcess(bih, &mplayerpid))
            {
                pInfo->pLastError = new AvisynthError("Failed launching video stream recipient app");
                break;
            }
        } 
              // send output to stdout
        if (nImageBytes != (nWrittenImageBytes = fwrite(pImageBuffer, 1, nImageBytes, stdout)))
        {
            AVXLOG_ERROR("Failed writing %d bytes to the stdout, it just wrote %d out of %d bytes. ERROR: %d (%s)",
                                nImageBytes, nWrittenImageBytes, nImageBytes, errno, strerror(errno));
            break;
        }
        AVXLOG_INFO("Delivered frame %ld", pInfo->nFramesSoFar);        
        pInfo->nFramesSoFar++;
        //if(nMaxFrames <= nFramesSoFar)
              //  break;
    }
    if (pInfo->isMPlayerLaunchRequired && mplayerpid)
        TerminateProcess(mplayerpid + 1);
    
    SAFE_DELETE_ARRAY(pImageBuffer);

    return pInfo;
}

void* ProcessAudioStream(void* pArgument) // very likely to end up being pthread function
{
    PROCESS_STREAM_INFO* pInfo = (PROCESS_STREAM_INFO*)pArgument;
    if(NULL == pInfo)
    {
        pInfo->pLastError = new AvisynthError("Passed NULL address of PROCESS_STREAM_INFO structure");
        return pInfo;
    }
                        
    //
    // Examine audio format details:
    // 
    void* pAudioInfo = NULL;
    LONG cbFormat = 0;
    if(false == retrieveAudioSettings(pInfo->pAVIStream, pAudioInfo, cbFormat))
    {
        AVXLOG_ERROR("%s", "Failed retrieved audio settings");
        pInfo->pLastError = new AvisynthError("Failed retrieving audio settings");        
        return pInfo;
    }

    //
    // Allocate audio buffer
    //
    WAVEFORMATEX* pWfx = (WAVEFORMATEX*)pAudioInfo; 
    long nAudioFrameBytes   = (pWfx->nChannels * pWfx->wBitsPerSample)/BITS_PER_BYTE; // 1 audio frame 
    long nAudioBufferBytes = pWfx->nSamplesPerSec*nAudioFrameBytes; // 1 second worth
    char* pAudioBuffer = new char[nAudioBufferBytes];
    if(NULL == pAudioBuffer)
    {
        AVXLOG_ERROR("%s", "Failed allocating %ld bytes for audio buffer", nAudioBufferBytes);
        pInfo->pLastError = new AvisynthError("Failed allocating memory for audio buffer");        
        return pInfo;
    }
    memset(pAudioBuffer, 0, nAudioBufferBytes);
                  
    //const long nMaxFrames = (pAVIStream->GetVideoInfo()).nun_frames;     
    int nWrittenAudioBytes = -1;
    int nSamplesToRead = pWfx->nSamplesPerSec;
    HRESULT hr;
    pid_t mplayerpid = -1;
    while(1)
    {
        int32_t nNextFrameStreamPos = pInfo->pAVIStream->FindSample(pInfo->nFramesSoFar, FIND_KEY);
        if(-1 == nNextFrameStreamPos)
        {
            AVXLOG_ERROR("%s", "Invalid stream position");
            pInfo->pLastError = new AvisynthError("Invalid stream position");
            break;
        }
                
        AVXLOG_INFO("Preparing to fetch audio frames #%ld - %ld", pInfo->nFramesSoFar, pInfo->nFramesSoFar + nSamplesToRead);
        //memset(pAudioBuffer, 0, bih.biSizeImage*sizeof(char));
                
        int32_t nBytesRead = 0;
        int32_t nSamplesRead = 0;
        hr = pInfo->pAVIStream->Read(nNextFrameStreamPos, nSamplesToRead, pAudioBuffer, nAudioBufferBytes, &nBytesRead, &nSamplesRead);
        if (nBytesRead==0 && nSamplesRead==0 && !FAILED(hr))
        {
            AVXLOG_ALERT("Reached end of stream at frame #%02ld", pInfo->nFramesSoFar);
            break;
        }
        if((FAILED(hr)) || 
           (nBytesRead != nAudioBufferBytes) || 
           (nSamplesRead != nSamplesToRead)
          )
        {
            AVXLOG_INFO("Read less bytes than expected, frame %02ld: nSamplesRead=%d, nBytesRead=%d, nImageBytes=%ld", pInfo->nFramesSoFar, nSamplesRead, nBytesRead, nAudioFrameBytes);
            break;
        }
                          
        if(pInfo->isMPlayerLaunchRequired && 0 == pInfo->nFramesSoFar)
        {
            if(false == launchAudioStreamRecipientProcess(pAudioInfo, &mplayerpid))
            {
                pInfo->pLastError = new AvisynthError("Failed launching audio stream recipient app");
                break;
            }
        } 
              // send output to stdout
        if (nAudioBufferBytes != (nWrittenAudioBytes = fwrite(pAudioBuffer, 1, nAudioBufferBytes, stdout)))
        {
            AVXLOG_ERROR("Wrote only %d out of %d bytes. ERROR: %d (%s)",
                                nWrittenAudioBytes, nAudioBufferBytes, errno, strerror(errno));
            break;
        }
        pInfo->nFramesSoFar+= nSamplesToRead;
    }
    if (pInfo->isMPlayerLaunchRequired && mplayerpid)
        TerminateProcess(mplayerpid + 1);

    SAFE_DELETE(pWfx);
    SAFE_DELETE_ARRAY(pAudioBuffer);

    return pInfo;
}

bool DetermineStreamType(IAVXSynth* pAVXSynth, IAVIStream* & pVideoStream, IAVIStream* & pAudioStream)
{
    DWORD dwFourCC;
    LONG lResult = 0;
    HRESULT hr;

    if(NULL == pAVXSynth)
    {
        AVXLOG_FATAL("%s", "IAVXSynth ptr has NULL value");        
        return false;
    }
    
    //
    // Check for video
    //
    dwFourCC = MAKEDWORD('s','d','i','v'); // byte-swapped 'vids' = 0x73646976;
    hr = pAVXSynth->GetStream(&pVideoStream, dwFourCC, lResult);
    if((S_OK != hr) || (NULL == pVideoStream))
    {
        AVXLOG_INFO("%s", "No video stream identified (or failed initializing video processing)");
        pVideoStream = NULL;
    }
    
    //
    // Check for audio
    //
    dwFourCC = MAKEDWORD('s','d','u','a'); // byte-swapped 'auds' = 0x73647561;
    hr = pAVXSynth->GetStream(&pAudioStream, dwFourCC, lResult);
    if((S_OK != hr) || (NULL == pAudioStream))
    {
        AVXLOG_INFO("%s", "No audio stream identified (or failed initializing audio processing)");
        pAudioStream = NULL;
    }
    return true;
}

extern int ProcessScript(const char *scriptName, bool isMPlayerLaunchRequired)
{
     AVXLOG_INFO("%s", __FUNCTION__);
    
    int result = -1;
    
    PROCESS_STREAM_INFO processStreamInfo;
    memset(&processStreamInfo, 0, sizeof(PROCESS_STREAM_INFO));
    
    PLUGIN_HANDLE hAvxSynth         = NULL;
    PCREATEAVISYNTH pCreateAVISynth = NULL;
    PDELETEAVISYNTH pDeleteAVISynth = NULL;
    IAVXSynth* pAVXSynth            = NULL;
    IAVIStream* pVideoStream        = NULL;
    IAVIStream* pAudioStream        = NULL;

    if (scriptName != NULL)
    {
        try 
        {  
          AVXLOG_INFO("Processing script %s:", scriptName);
          printScript(scriptName);
          
          hAvxSynth = dlopen("libavxsynth.so", RTLD_NOW | RTLD_GLOBAL);
          if(NULL == hAvxSynth)
          {
              AVXLOG_ERROR("%s", "Failed loading libavxsynth.so");
              AVXLOG_ERROR("Error: %s", dlerror());
              return -1;
          }
          
          pCreateAVISynth = (PCREATEAVISYNTH)dlsym(hAvxSynth, "CreateAVISynth");
          if(NULL == pCreateAVISynth)
          {
              AVXLOG_ERROR("%s", "Failed retrieving shared library \"CreateAVISynth\" symbol");
              AVXLOG_ERROR("Error: %s", dlerror());
          }

          pDeleteAVISynth = (PDELETEAVISYNTH)dlsym(hAvxSynth, "DeleteAVISynth");
          if(NULL == pDeleteAVISynth)
          {
              AVXLOG_ERROR("%s", "Failed retrieving shared library \"DeleteAVISynth\" symbol");
              AVXLOG_ERROR("Error: %s", dlerror());              
          }
          pCreateAVISynth(&pAVXSynth);
          if(NULL == pAVXSynth)
          {
              AVXLOG_ERROR("%s", "Failed retrieving IAVXSynth interface pointer");
              dlclose(hAvxSynth);
              return -1;
          }
          
          pAVXSynth->Load(scriptName, OF_READ);
              
          DetermineStreamType(pAVXSynth, pVideoStream, pAudioStream);
          
          if((NULL == pVideoStream) && (NULL == pAudioStream))
          {
                AVXLOG_FATAL("%s", "No streams found (or failed initializing any stream processing");
                throw AvisynthError("No streams found (or failed initializing any stream processing)");
          }
          memset(&processStreamInfo, 0, sizeof(PROCESS_STREAM_INFO));
          processStreamInfo.isMPlayerLaunchRequired = isMPlayerLaunchRequired;
          PROCESS_STREAM_INFO* pRetValue = NULL;
          
          //
          // If video stream has been detected, give it priority and treat everything as 'video' case; 
          // only if there is no video stream consider the audio case next.
          //
          if(pVideoStream)
          {
            processStreamInfo.pAVIStream          = pVideoStream;
            processStreamInfo.isVideoFieldBased   = pAVXSynth->IsFieldBased();
            pRetValue = (PROCESS_STREAM_INFO*)ProcessVideoStream((void*)&processStreamInfo);
          }
          else if(pAudioStream)
          {
            processStreamInfo.pAVIStream          = pAudioStream;
            pRetValue = (PROCESS_STREAM_INFO*)ProcessAudioStream((void*)&processStreamInfo);               
          }
          if(NULL == pRetValue)
              throw AvisynthError("Stream processing function returns NULL !?");
          if(pRetValue->pLastError)
              throw pRetValue->pLastError;
          
          AVXLOG_INFO("Delivered %ld frames", processStreamInfo.nFramesSoFar);
                  
          result = 0;
        }
        catch(AvisynthError error)
        {
            AVXLOG_ERROR("%s: ", "AVXSynthError");
            AVXLOG_ERROR("%s", error.msg);
        }
        catch(...)
        {
            AVXLOG_ERROR("%s", "Caught unspecified AVXSynth exception !!!");
        }
    }
    
    
    SAFE_DELETE(processStreamInfo.pLastError);
    
    if(pDeleteAVISynth)
    {
        pDeleteAVISynth(pAVXSynth);
        pDeleteAVISynth = NULL;
    }
    
    SAFE_DELETE(pVideoStream);
    SAFE_DELETE(pAudioStream);
    
    dlclose(hAvxSynth);

    return result;
}
} // namespace avxsynth
