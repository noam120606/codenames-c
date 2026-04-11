/**
 * @file audio.h
 * @brief Gestion du système audio du client (musiques, effets sonores, filtres).
 */

#ifndef AUDIO_H
#define AUDIO_H

#include "sdl.h"

#define MAX_SOUNDS 32
#define AUDIO_FILTER_DEFAULT_CUTOFF_HZ 1200.0f
#define AUDIO_FILTER_MIN_CUTOFF_HZ 120.0f
#define AUDIO_FILTER_MAX_CUTOFF_HZ 18000.0f

/**
 * @brief Identifiants des sons pris en charge par le client.
 */
typedef enum {
    MUSIC_MENU_LOBBY,           /**< Musique du menu et du lobby. */
    MUSIC_GAME,                 /**< Musique pendant la partie. */
    SOUND_OPENING_CODENAMES,    /**< Audio d'ouverture avant l'arrivée du menu. */
    SOUND_BUTTON_CLICKED,       /**< Son de clic bouton. */
    SOUND_TYPE_WRITING,         /**< Son de saisie clavier. */
    SOUND_CARD_DISTRIBUTION,    /**< Son de distribution des cartes. */
    SOUND_CARD_GUESS,           /**< Son de sélection d'une carte. */
    SOUND_CARD_CONFIRMATION,    /**< Son de validation d'une carte. */
    SOUND_CARD_ENEMY,           /**< Son de révélation carte ennemie. */
    SOUND_CARD_FRIEND,          /**< Son de révélation carte alliée. */
    SOUND_CARD_NONE,            /**< Son de révélation carte neutre. */
    SOUND_CARD_ASSASSIN,        /**< Son de révélation carte assassin. */
    SOUND_VICTORY,              /**< Son de victoire. */
    SOUND_DEFEAT,               /**< Son de défaite. */
    SOUND_INFO_OPEN,            /**< Son d'ouverture d'une infobulle. */
    SOUND_INFO_CLOSE,           /**< Son de fermeture d'une infobulle. */
} SoundID;

/**
 * @brief Catégorie logique d'un son.
 */
typedef enum {
    AUDIO_SOUND_KIND_MUSIC, /**< Musique de fond. */
    AUDIO_SOUND_KIND_SFX,   /**< Effet sonore ponctuel. */
} AudioSoundKind;

/**
 * @brief Type de filtre audio disponible.
 */
typedef enum {
    AUDIO_FILTER_NONE,     /**< Aucun filtre. */
    AUDIO_FILTER_LOW_PASS, /**< Filtre passe-bas. */
} AudioFilterType;

/**
 * @brief Stratégie de fade-out lors de l'arrêt d'un son.
 */
typedef enum {
    AUDIO_FADE_OUT_BY_VOLUME, /**< Fade-out par diminution du volume. */
    AUDIO_FADE_OUT_BY_FILTER, /**< Fade-out par évolution d'un filtre audio. */
} AudioFadeOutType;

/**
 * @brief Paramètres d'un fade-out basé sur un filtre.
 */
typedef struct {
    AudioFilterType filter_type; /**< Filtre à utiliser pendant le fade-out. */
    float start_cutoff_hz;       /**< Fréquence de coupure initiale en Hz (<= 0: fréquence courante/défaut). */
    float end_cutoff_hz;         /**< Fréquence de coupure finale en Hz (<= 0: fréquence par défaut). */
} AudioFadeOutFilterParams;

/**
 * @brief Stratégie de fade-in lors du lancement d'un son.
 */
typedef enum {
    AUDIO_FADE_IN_BY_VOLUME, /**< Fade-in par augmentation du volume. */
    AUDIO_FADE_IN_BY_FILTER, /**< Fade-in par évolution d'un filtre audio. */
} AudioFadeInType;

/**
 * @brief Paramètres d'un fade-in basé sur un filtre.
 */
typedef struct {
    AudioFilterType filter_type; /**< Filtre à utiliser pendant le fade-in. */
    float start_cutoff_hz;       /**< Fréquence de coupure initiale en Hz (<= 0: basse fréquence par défaut). */
    float end_cutoff_hz;         /**< Fréquence de coupure finale en Hz (<= 0: fréquence courante/défaut). */
} AudioFadeInFilterParams;

/**
 * @brief Configuration persistante d'un son.
 */
typedef struct {
    AudioSoundKind kind;    /**< Catégorie du son (musique ou effet). */
    int volume;             /**< Volume de base dans [0, MIX_MAX_VOLUME]. */
    int bypass_filter;      /**< 1: pas de filtre, 0: applique un filtre passe-bas. */
    float filter_cutoff_hz; /**< Fréquence de coupure du passe-bas en Hz. */
} SoundConfig;

/**
 * @brief Initialise le sous-système audio.
 * @return EXIT_SUCCESS si l'initialisation a réussi, EXIT_FAILURE sinon.
 */
int audio_init();

/**
 * @brief Lance la lecture d'un son sans fade-in explicite.
 *
 * Cette fonction est un raccourci vers audio_play_with_fade avec une durée
 * de fade-in nulle.
 *
 * @param id Identifiant du son à lire.
 * @param loops Nombre de répétitions (-1 pour une boucle infinie).
 */
void audio_play(SoundID id, int loops);

/**
 * @brief Lance la lecture d'un son avec fade-in configurable.
 * @param id Identifiant du son à lire.
 * @param loops Nombre de répétitions (-1 pour une boucle infinie).
 * @param fade_in_ms Durée du fade-in en millisecondes (<= 0: lecture immédiate).
 * @param fade_type Type de fade-in à appliquer.
 * @param filter_params Paramètres de filtre utilisés si fade_type vaut AUDIO_FADE_IN_BY_FILTER.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int audio_play_with_fade(
    SoundID id,
    int loops,
    int fade_in_ms,
    AudioFadeInType fade_type,
    const AudioFadeInFilterParams* filter_params
);

/**
 * @brief Définit toute la configuration d'un son.
 * @param id Identifiant du son.
 * @param cfg Nouvelle configuration à appliquer.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int audio_set_sound_config(SoundID id, const SoundConfig* cfg);

/**
 * @brief Récupère toute la configuration d'un son.
 * @param id Identifiant du son.
 * @param out_cfg Pointeur de sortie recevant la configuration.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int audio_get_sound_config(SoundID id, SoundConfig* out_cfg);

/**
 * @brief Définit le volume de base d'un son.
 * @param id Identifiant du son.
 * @param volume Volume dans [0, MIX_MAX_VOLUME].
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int audio_set_sound_volume(SoundID id, int volume);

/**
 * @brief Configure le filtre d'un son.
 * @param id Identifiant du son.
 * @param filter_type Type de filtre à appliquer.
 * @param cutoff_frequency Fréquence de coupure en Hz (si applicable).
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int audio_set_filter(SoundID id, AudioFilterType filter_type, float cutoff_frequency);

/**
 * @brief Récupère le filtre actuellement configuré pour un son.
 * @param id Identifiant du son.
 * @param out_filter_type Pointeur de sortie recevant le type de filtre.
 * @param out_cutoff_frequency Pointeur de sortie recevant la fréquence de coupure en Hz
 * (peut être NULL si non nécessaire).
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int audio_get_filter(SoundID id, AudioFilterType* out_filter_type, float* out_cutoff_frequency);

/**
 * @brief Définit le volume global d'une catégorie de sons.
 * @param kind Catégorie de son ciblée (musique ou effet sonore).
 * @param volume Volume dans [0, MIX_MAX_VOLUME].
 */
void audio_set_type_volume(AudioSoundKind kind, int volume);

/**
 * @brief Arrête un son sans fade-out explicite.
 *
 * Cette fonction est un raccourci vers audio_stop_with_fade avec une durée
 * de fade-out nulle.
 *
 * @param id Identifiant du son à arrêter.
 */
void audio_stop(SoundID id);

/**
 * @brief Arrête un son avec fade-out configurable.
 * @param id Identifiant du son à arrêter.
 * @param fade_out_ms Durée du fade-out en millisecondes (<= 0: arrêt immédiat).
 * @param fade_type Type de fade-out à appliquer.
 * @param filter_params Paramètres de filtre utilisés si fade_type vaut AUDIO_FADE_OUT_BY_FILTER.
 * @return EXIT_SUCCESS en cas de succès, EXIT_FAILURE sinon.
 */
int audio_stop_with_fade(
    SoundID id,
    int fade_out_ms,
    AudioFadeOutType fade_type,
    const AudioFadeOutFilterParams* filter_params
);

/**
 * @brief Indique si un son est actuellement en lecture.
 * @param id Identifiant du son à tester.
 * @return 1 si le son est en cours de lecture, 0 sinon.
 */
int audio_is_playing(SoundID id);

/**
 * @brief Libère les ressources audio et ferme le sous-système audio.
 */
void audio_cleanup();

#endif