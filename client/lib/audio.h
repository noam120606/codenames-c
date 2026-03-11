#ifndef AUDIO_H
#define AUDIO_H

#include "sdl.h"

#define MAX_SOUNDS 32

/**
 * Sound effect IDs.
 */
typedef enum {
    MUSIC_MENU,
    SOUND_CLICKED_BUTTON,
    SOUND_TYPE_WRITING,
    SOUND_CARD_DISTRIBUTION,
    SOUND_CARD_GUESS,
    SOUND_CARD_CONFIRMATION,
    SOUND_CARD_ENEMY,
    SOUND_CARD_FRIEND,
    SOUND_CARD_NONE,
    SOUND_CARD_ASSASSIN,
} SoundID;

/**
 * Initialize the audio subsystem.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int audio_init();

/**
 * Play a sound effect.
 * @param id The ID of the sound effect to play.
 * @param loops The number of times to loop the sound effect (-1 for infinite).
 */
void audio_play(SoundID id, int loops);

/**
 * Stop a sound effect.
 * @param id The ID of the sound effect to stop.
 */
void audio_stop(SoundID id);

/**
 * Check if a sound effect is currently playing.
 * @param id The ID of the sound effect to check.
 * @return 1 if the sound effect is playing, 0 otherwise.
 */
int audio_is_playing(SoundID id);

/**
 * Clean up the audio subsystem.
 */
void audio_cleanup();

#endif