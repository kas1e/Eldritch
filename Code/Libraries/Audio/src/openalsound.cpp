#include "core.h"
#include "openalsound.h"
#include "packstream.h"
#include "openalsoundinstance.h"
#include "openalaudiosystem.h"
#include <AL/alut.h>
#ifdef USE_TREMOR
#include <tremor/ivorbisfile.h>
#else
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#endif


OpenALSound::OpenALSound(IAudioSystem* const pSystem,
                         const SSoundInit& SoundInit) {
  buffer = 0;
  ASSERT(pSystem);
  SetAudioSystem(pSystem);
  // OpenALAudioSystem* const pOALAudioSystem = static_cast<OpenALAudioSystem*>(
  // pSystem );

  ASSERT(SoundInit.m_Filename != "");

  // Get the file extension
  size_t ExtOffset = strlen(SoundInit.m_Filename.CStr()) - 3;
  unsigned int Ext = *(unsigned int*)(SoundInit.m_Filename.CStr() + ExtOffset);

  if (SoundInit.m_IsStream && SoundInit.m_Category == "Music") {
    CreateSampleFromOGG(PackStream(SoundInit.m_Filename.CStr()),
                 SoundInit.m_IsLooping);
  } else {
    switch (Ext) {
      case EXT_OGG:
        CreateSampleFromOGG(PackStream(SoundInit.m_Filename.CStr()),
                            SoundInit.m_IsLooping);
        break;
      case EXT_WAV:
        CreateSampleFromWAV(PackStream(SoundInit.m_Filename.CStr()),
                            SoundInit.m_IsLooping);
        break;
      default:
        PRINTF("Error: Unknown file extension for sample loading!\n");
    }
  }
}

OpenALSound::~OpenALSound() { alDeleteBuffers(1, &buffer); }

/*virtual*/ ISoundInstance* OpenALSound::CreateSoundInstance() {
  // should try to get an existing source rather than create a new one?
  auto  const pInstance = new OpenALSoundInstance(this);
  ALuint src = pInstance->source;

  alSourcei(src, AL_BUFFER, buffer);
  alSource3f(src, AL_POSITION, 0, 0, 0);
  alSourcei(src, AL_LOOPING, GetLooping() ? AL_TRUE : AL_FALSE);
  alSourcePlay(src);

  return pInstance;
}

void OpenALSound::CreateSampleFromOGG(const IDataStream& Stream, bool Looping) {
  int Length = Stream.Size();
  auto  pBuffer = new byte[Length];
  OggVorbis_File vf;
  ov_callbacks ov_call = {nullptr, nullptr, nullptr, nullptr};
  Stream.Read(Length, pBuffer);
  
  ov_open_callbacks(nullptr, &vf, (const char *)pBuffer, Length, ov_call);
  int bitstream = 0;
  vorbis_info *info = ov_info(&vf, -1);
  long size = 0;
  ALenum format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
  ALuint freq = info->rate;
  long rc = 0;
  #ifdef __amigaos4__
  long allocated = 1024*1024*16;
  #else
  long allocated = 4*1024*16;
  #endif
  ALubyte *retval = static_cast<ALubyte*>(malloc(allocated));
  char *buff = static_cast<char*>(malloc(allocated));

#ifdef USE_TREMOR
  while ( (rc = ov_read(&vf, buff, allocated, &bitstream)) != 0 ) 
#else
  #ifdef __amigaos4__
  while ( (rc = ov_read(&vf, buff, allocated, 1, 2, 1, &bitstream)) != 0 ) 
  #else
  while ( (rc = ov_read(&vf, buff, allocated, 0, 2, 1, &bitstream)) != 0 ) 
  #endif
#endif
  {
    if (rc <= 0) {
      break;
    }
    size += rc;

    ALubyte *tmp = static_cast<ALubyte*>(realloc(retval, size));
    if (tmp == nullptr)
      {
        free(retval);
        retval = nullptr;
        break;
      }
    retval = tmp;
    memmove(retval+size-rc, buff, rc);
  }
  alGenBuffers(1, &buffer);
  alBufferData(buffer, format, retval, size, freq);
  free(retval);
  free(buff);
  ov_clear(&vf);
}

void OpenALSound::CreateSampleFromWAV(const IDataStream& Stream, bool Looping) {
  int Length = Stream.Size();
  auto  pBuffer = new byte[Length];
  Stream.Read(Length, pBuffer);
  buffer = alutCreateBufferFromFileImage(pBuffer, Length);
}  

void OpenALSound::CreateStream(const PackStream& Stream, bool Looping) {
  // PRINTF("OpenALSound::CreateStream() STUB, doing nothing.\n");
  //~ const char*	pFilename = Stream.GetPhysicalFilename();
  //~ uint		Offset = Stream.GetSubfileOffset();
  //~ uint		Length = Stream.GetSubfileLength();
  //~
  //~ FMOD_CREATESOUNDEXINFO ExInfo = { 0 };
  //~ ExInfo.cbsize = sizeof( FMOD_CREATESOUNDEXINFO );
  //~ ExInfo.length = Length;
  //~ ExInfo.fileoffset = Offset;
  //~
  //~ FMOD_MODE Mode = FMOD_CREATESTREAM | FMOD_SOFTWARE;	// Use software so
  //we can apply reverb
  //~ Mode |= Looping ? FMOD_LOOP_NORMAL : 0;
  //~ FMOD_RESULT Result = m_FMODSystem->createSound( pFilename, Mode, &ExInfo,
  //&m_Sound );
  //~ ASSERT( Result == FMOD_OK );
  //~ Unused( Result );
}

float OpenALSound::GetLength() const {
  PRINTF("OpenALSound::GetLength() STUB, returning 1.\n");
  return (float)1.0f;
}
