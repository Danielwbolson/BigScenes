#ifndef SOUND_H
#define SOUND_H

#include "GPU-Includes.h" //Really, I only need SDL for SDL Audio

#include <vector>

//Sound Manager
// PlaySong() switchs to new active background music, and fades out others
// PlaySoundEffect() plays a one-shot clip
// LoadAudio() loads sound into sound buffer

//To convert sounds to wav files you can use ffmpeg as follows:
//ffmpeg -i in.mp3 -acodec pcm_s16le -ac 2 -ar 48000 out.wav

//TODO: Show how to add echo!

//TODO: Allow a single clip to be played multiple times simultaniously
//TODO: Calatlog of loaded sounds, list of active sounds (and support instansed sound)
//TODO: Spatial audio
//TODO: Try OGG Vorbis http://xiph.org/downloads/
//TODO: Option to speed up sounds in playback (other distortions)
//TODO: Use SDL_QueueAudio instead of callback
//TODO: Max number of sounds to play at once (to prevent clipping)

//Some tutorials:
//  https://adamtcroft.com/playing-sound-with-sdl-c/   (getting started)
//  https://github.com/jakebesworth/Simple-SDL2-Audio/ (a similar audio manager)
//  https://gist.github.com/armornick/3447121 (just the basics)

//Switch to SDL_QueueAudio, much simpler -- https://forums.libsdl.org/viewtopic.php?p=44610
//         Doesn't require power of 2 buffers 

struct AudioClip{
    uint8_t* curPosition;
    uint32_t lengthRemaining;
    uint8_t* startPosition;
    uint32_t clipLength;
    int delay = 0;
    bool enabled = false;
    uint8_t loop = false;
    bool fadeIn = false;
    bool fadeOut = false;
    bool background = false; //Only 1 background song at a time
    uint8_t free;
    uint8_t curVolume = 0;
    uint8_t targetVolume = 127; //Max volume
};


#define AUDIO_FORMAT AUDIO_S16LSB  //SDL_AudioFormat of S16 little endian

//Our sound library assumes all sounds are the same freqency
//  and the same number of channels
#define AUDIO_FREQUENCY 48000 //48kHz
#define AUDIO_CHANNELS 2 //1 mono, 2 stereo, 4 quad, 6 (5.1 surround) 

#define AUDIO_SAMPLES 4096 //How much data to be processed per cycle (must be power of 2)

#define SDL_AUDIO_ALLOW_CHANGES SDL_AUDIO_ALLOW_ANY_CHANGE //Note can effect Emscripten / EMCC 
//#define SDL_AUDIO_ALLOW_CHANGES SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE  //TODO: Maybe this is needed for Windows 7


struct AudioManager{
  SDL_AudioDeviceID audioDevice;
  SDL_AudioSpec audioConfig;
  bool audioInitalized; //TODO: We don't use this, maybe we should check if init has been called
  
  //AudioClip audio;

  int soundCount;

  void init();

  void unpause(); //Play sounds
  void pause(); //Pause sounds

  void playSong(int audioID, int volume);
  void playSoundEffect(int audioID, int volume, float delay=0);
  //void playSong(const char * filename, int volume);
  //void playSoundEffect(const char * filename, int volume);
  int loadAudio(const char * filename);

  ~AudioManager();
};

void audioCallback(void * userdata, uint8_t * stream, int streamLength);

#endif //SOUND_H