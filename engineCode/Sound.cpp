#include "Sound.h"

std::vector<AudioClip> audioClips;


void audioCallback(void* userData, Uint8* stream, int streamLength){
  SDL_memset(stream, 0, streamLength);
  if (!userData) return;
  //AudioClip* audio = (AudioClip*)userData;

  for (size_t i = 0; i < audioClips.size(); i++){
    //printf("%d\n",i);
    AudioClip* audio = &audioClips[i];
    if (!audio->enabled){
      //printf("skipping\n");
      continue;
    }


    if (audio->lengthRemaining > 0){
      uint32_t length = (uint32_t)streamLength;

      if (audio->delay > 0){
        audio->delay -= length;
        continue;
      }

      if (audio->fadeOut && audio->curVolume > 0){
        audio->curVolume -= 30; //TODO: This should be framerate independent (Can I asusme this is called 12Hz?)
      }
      if (audio->fadeIn && audio->curVolume < audio->targetVolume){
        audio->curVolume += 30; //TODO: This should be framerate independent
      }
      //printf("Fade %d %d (%d)\n",i,audio->curVolume,audio->lengthRemaining);

      length = (length > audio->lengthRemaining ? audio->lengthRemaining : length); //amount of sound to copy

      //SDL_memcpy(stream, audio->position, length); //Copy audio to stream
      SDL_MixAudioFormat(stream, audio->curPosition, AUDIO_FORMAT, length, audio->curVolume);
      
      audio->curPosition += length;
      audio->lengthRemaining -= length;
    }
    else if (audio->loop){
      printf("Looping\n");
      audio->curPosition = audio->startPosition;
      audio->lengthRemaining = audio->clipLength;
    }
    else { //We've played the sound and it's not a loop
      audio->enabled = false;
    }
  }
}

void AudioManager::init(){
    soundCount = 0;

    if(!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)){
        fprintf(stderr, "[%s: %d]Error: SDL_INIT_AUDIO not initialized\n", __FILE__, __LINE__);
        return;
    }

    AudioClip audio;

    audioConfig.freq = AUDIO_FREQUENCY;
    audioConfig.format = AUDIO_FORMAT;
    audioConfig.channels = AUDIO_CHANNELS;
    audioConfig.samples = AUDIO_SAMPLES;
    audioConfig.callback = audioCallback;
    audioConfig.userdata = &audio;

    audioDevice = SDL_OpenAudioDevice(NULL, 0, &audioConfig, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE); //Note: awllow_any_change can effect Emscripten / EMCC
    if (audioDevice == 0){
      printf("Error Opening Audio Device: %s\n", SDL_GetError());
      exit(1);
    }

    unpause(); //Play Audio
}



AudioManager::~AudioManager(){
  SDL_CloseAudioDevice(audioDevice);
}

void AudioManager::unpause(){
  SDL_PauseAudioDevice(audioDevice, 0);
}

void AudioManager::pause(){
  SDL_PauseAudioDevice(audioDevice, 1);
}

int AudioManager::loadAudio(const char * filename){
  //SDL_AudioSpec newAudio = audioConfig; //TODO: Is it safe to reuse audioConfig?
  AudioClip audio;

  if(SDL_LoadWAV(filename, &audioConfig, &audio.startPosition, &audio.clipLength ) == NULL){
    printf("ERROR: Failed opening audio file '%s' with error: %s.\n",filename,SDL_GetError()); //TODO: Does SDL_GetError() return anything here?
    exit(1);
  }

  audio.curPosition = audio.startPosition;
  audio.lengthRemaining = audio.clipLength;

  audioConfig.userdata = 0;

  audioClips.push_back(audio);

  return audioClips.size()-1; //ID of new audio
}

void AudioManager::playSong(int audioID, int volume){
  audioClips[audioID].curPosition = audioClips[audioID].startPosition;
  audioClips[audioID].lengthRemaining = audioClips[audioID].clipLength;
  audioClips[audioID].targetVolume = volume;
  audioClips[audioID].enabled = true;
  audioClips[audioID].loop = true;
  audioClips[audioID].fadeIn = true;
  audioClips[audioID].background = true;

  for (int i = 0; i < (int)audioClips.size(); i++){
    if (i != audioID && audioClips[i].background){  //Fade out other sounds
      audioClips[i].fadeOut = true;
      audioClips[i].fadeIn = false;
    }
  }

  //TODO: Push playing clips into queue of actively playing clips
}

void AudioManager::playSoundEffect(int audioID, int volume, float delay){
  audioClips[audioID].curPosition = audioClips[audioID].startPosition;
  audioClips[audioID].lengthRemaining = audioClips[audioID].clipLength;
  audioClips[audioID].targetVolume = volume;
  audioClips[audioID].curVolume = volume;
  audioClips[audioID].enabled = true;
  audioClips[audioID].delay = AUDIO_FREQUENCY*delay*4; //TODO: Why times 4? I think b/c the stream expects chars, and our sound is floats (4 chars)

  //TODO: Push playing clips into queue of actively playing clips
}
