#include "../lib/all.h"

static Mix_Chunk* sounds[MAX_SOUNDS];
static int sound_channels[MAX_SOUNDS];

int audio_init() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Erreur Mixer: %s\n", Mix_GetError());
        return EXIT_FAILURE;
    }

    // Initialize arrays
    for (int i = 0; i < MAX_SOUNDS; i++) {
        sounds[i] = NULL;
        sound_channels[i] = -1;
    }

    // Load menu music as a chunk so it can be played on a channel
    sounds[MUSIC_MENU] = Mix_LoadWAV("assets/audio/music_menu.ogg");
    if (!sounds[MUSIC_MENU]) {
        printf("Erreur Mix_LoadWAV: %s\n", Mix_GetError());
    }

    return EXIT_SUCCESS;
}

void audio_play(SoundID id, int loops) {
    if (id < MAX_SOUNDS && sounds[id]) {
        int channel = Mix_PlayChannel(-1, sounds[id], loops);
        if (channel >= 0) {
            sound_channels[id] = channel;
        }
    }
}

void audio_stop(SoundID id) {
    if (id < MAX_SOUNDS && sound_channels[id] != -1) {
        Mix_HaltChannel(sound_channels[id]);
        sound_channels[id] = -1;
    }
}

int audio_is_playing(SoundID id) {
    if (id < MAX_SOUNDS && sound_channels[id] != -1) {
        return Mix_Playing(sound_channels[id]);
    }
    return 0;
}

void audio_cleanup() {
    for (int i = 0; i < MAX_SOUNDS; i++) {
        if (sounds[i]) Mix_FreeChunk(sounds[i]);
        sounds[i] = NULL;
        sound_channels[i] = -1;
    }
    Mix_CloseAudio();
}