#ifndef AUDIO_H
#define AUDIO_H

#include "sdl.h"

#define MAX_SOUNDS 32

/**
 * Sound effect IDs.
 */
typedef enum {
    MUSIC_MENU_LOBBY, // Musique dans le menu et dans le lobby
    MUSIC_GAME,       // Musique pendant le jeu
    SOUND_BUTTON_CLICKED,
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
 * Logical type of a sound.
 */
typedef enum {
    AUDIO_SOUND_KIND_MUSIC,
    AUDIO_SOUND_KIND_SFX,
} AudioSoundKind;

/**
 * Filter type for audio effects.
 */
typedef enum {
    AUDIO_FILTER_NONE,
    AUDIO_FILTER_LOW_PASS,
} AudioFilterType;

/**
 * Per-sound configuration.
 */
typedef struct {
    AudioSoundKind kind;      /**< Music or sound effect. */
    int volume;               /**< Sound base volume (0-128). */
    int bypass_filter;        /**< 1 = bypass (no filter), 0 = apply low-pass filter. */
    float filter_cutoff_hz;   /**< Low-pass cutoff frequency in Hz. */
} SoundConfig;

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
 * Set all config fields for a sound.
 * @param id The sound ID.
 * @param cfg The new config.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int audio_set_sound_config(SoundID id, const SoundConfig* cfg);

/**
 * Get all config fields for a sound.
 * @param id The sound ID.
 * @param out_cfg Output config pointer.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int audio_get_sound_config(SoundID id, SoundConfig* out_cfg);

/**
 * Set a sound volume.
 * @param id The sound ID.
 * @param volume Volume in range 0..MIX_MAX_VOLUME.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int audio_set_sound_volume(SoundID id, int volume);

/**
 * Set a filter for a sound.
 * @param id The sound ID.
 * @param filter_type The type of filter to apply.
 * @param cutoff_frequency The cutoff frequency for the filter.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int audio_set_filter(SoundID id, AudioFilterType filter_type, float cutoff_frequency);

/**
 * Set global volume for a sound kind (music or sfx).
 * @param kind Audio sound kind.
 * @param volume Volume in range 0..MIX_MAX_VOLUME.
 */
void audio_set_type_volume(AudioSoundKind kind, int volume);

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