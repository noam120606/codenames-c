#include "../lib/all.h"

/* Constantes internes du module audio. */
#define AUDIO_PI 3.14159265358979323846f

/* Etat d'un filtre passe-bas appliqué en continu sur un canal. */
typedef struct {
    int active;
    float alpha;
    float previous_left;
    float previous_right;
} ChannelFilterState;

/* Etat d'un filtre passe-bas interpolé (fade in/out par filtre). */
typedef struct {
    int active;
    Uint32 start_ticks;
    Uint32 duration_ms;
    float start_alpha;
    float end_alpha;
    int deactivate_on_complete;
    float previous_left;
    float previous_right;
} ChannelFadeFilterState;

/* Etat global interne du module audio. */
static Mix_Chunk* sounds[MAX_SOUNDS];
static int sound_channels[MAX_SOUNDS];
static SoundConfig sound_cfgs[MAX_SOUNDS];
static int type_volumes[2] = {MIX_MAX_VOLUME, MIX_MAX_VOLUME};
static SoundID channel_sound_ids[MAX_SOUNDS];
static ChannelFilterState channel_filters[MAX_SOUNDS];
static ChannelFadeFilterState channel_fade_filters[MAX_SOUNDS];

/* Clamp un volume SDL_mixer dans [0, MIX_MAX_VOLUME]. */
static int clamp_volume(int volume) {
    if (volume < 0) return 0;
    if (volume > MIX_MAX_VOLUME) return MIX_MAX_VOLUME;
    return volume;
}

/* Vérifie qu'un SoundID est dans la plage valide. */
static int is_valid_sound_id(SoundID id) {
    return id >= 0 && id < MAX_SOUNDS;
}

/* Vérifie qu'une catégorie audio est valide. */
static int is_valid_kind(AudioSoundKind kind) {
    return kind == AUDIO_SOUND_KIND_MUSIC || kind == AUDIO_SOUND_KIND_SFX;
}

/* Vérifie qu'un canal est actuellement associé au SoundID attendu. */
static int is_channel_bound_to_sound(int channel, SoundID id) {
    if (channel < 0 || channel >= MAX_SOUNDS || !is_valid_sound_id(id)) {
        return 0;
    }
    return channel_sound_ids[channel] == id;
}

/* Calcule le volume final appliqué au canal (volume son * volume type). */
static int compute_effective_volume(SoundID id) {
    SoundConfig* cfg = &sound_cfgs[id];
    int kind_volume = type_volumes[cfg->kind];
    return (cfg->volume * kind_volume) / MIX_MAX_VOLUME;
}

/* Retourne le premier canal libre, ou -1 s'il n'y en a pas. */
static int find_available_channel(void) {
    for (int i = 0; i < MAX_SOUNDS; ++i) {
        if (!Mix_Playing(i) && !Mix_Paused(i)) {
            return i;
        }
    }
    return -1;
}

/* Convertit une fréquence de coupure (Hz) en coefficient alpha du passe-bas. */
static float compute_low_pass_alpha(float cutoff_hz) {
    int frequency = 0;
    Uint16 format = 0;
    int channels = 0;

    if (Mix_QuerySpec(&frequency, &format, &channels) == 0 || frequency <= 0) {
        frequency = 44100;
    }

    if (cutoff_hz <= 0.0f) {
        cutoff_hz = AUDIO_FILTER_DEFAULT_CUTOFF_HZ;
    }

    float dt = 1.0f / (float)frequency;
    float rc = 1.0f / (2.0f * AUDIO_PI * cutoff_hz);
    return dt / (rc + dt);
}

/* Charge un son de manière sûre: vérifie la présence du fichier avant chargement. */
static Mix_Chunk* load_wav_safe(const char* path) {
    if (!path || access(path, R_OK) != 0) {
        /* fichier non trouvé */
        return NULL;
    }
    Mix_Chunk* c = Mix_LoadWAV(path);
    return c;
}

/* Callback SDL_mixer: applique un passe-bas fixe sur le flux stéréo du canal. */
static void low_pass_effect(int channel, void* stream, int len, void* udata) {
    (void)channel;

    ChannelFilterState* state = (ChannelFilterState*)udata;
    if (!state || !state->active || !stream || len <= 0) {
        return;
    }

    Sint16* samples = (Sint16*)stream;
    int sample_count = len / (int)sizeof(Sint16);

    for (int i = 0; i < sample_count; i += 2) {
        float left_in = (float)samples[i];
        state->previous_left += state->alpha * (left_in - state->previous_left);
        samples[i] = (Sint16)state->previous_left;

        if (i + 1 < sample_count) {
            float right_in = (float)samples[i + 1];
            state->previous_right += state->alpha * (right_in - state->previous_right);
            samples[i + 1] = (Sint16)state->previous_right;
        }
    }
}

/* Callback SDL_mixer: applique un passe-bas interpolé pour un fade par filtre. */
static void fade_low_pass_effect(int channel, void* stream, int len, void* udata) {
    (void)channel;

    ChannelFadeFilterState* state = (ChannelFadeFilterState*)udata;
    if (!state || !state->active || !stream || len <= 0) {
        return;
    }

    Uint32 now = SDL_GetTicks();
    Uint32 elapsed = now - state->start_ticks;
    float t = 1.0f;
    if (state->duration_ms > 0) {
        t = (float)elapsed / (float)state->duration_ms;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
    }

    float alpha = state->start_alpha + (state->end_alpha - state->start_alpha) * t;

    Sint16* samples = (Sint16*)stream;
    int sample_count = len / (int)sizeof(Sint16);

    for (int i = 0; i < sample_count; i += 2) {
        float left_in = (float)samples[i];
        state->previous_left += alpha * (left_in - state->previous_left);
        samples[i] = (Sint16)state->previous_left;

        if (i + 1 < sample_count) {
            float right_in = (float)samples[i + 1];
            state->previous_right += alpha * (right_in - state->previous_right);
            samples[i + 1] = (Sint16)state->previous_right;
        }
    }

    if (t >= 1.0f && state->deactivate_on_complete) {
        state->active = 0;
    }
}

/* Réinitialise l'état interne d'un canal audio. */
static void reset_channel_state(int channel) {
    if (channel < 0 || channel >= MAX_SOUNDS) {
        return;
    }

    channel_sound_ids[channel] = -1;
    channel_filters[channel].active = 0;
    channel_filters[channel].alpha = 0.0f;
    channel_filters[channel].previous_left = 0.0f;
    channel_filters[channel].previous_right = 0.0f;

    channel_fade_filters[channel].active = 0;
    channel_fade_filters[channel].start_ticks = 0;
    channel_fade_filters[channel].duration_ms = 0;
    channel_fade_filters[channel].start_alpha = 0.0f;
    channel_fade_filters[channel].end_alpha = 0.0f;
    channel_fade_filters[channel].deactivate_on_complete = 0;
    channel_fade_filters[channel].previous_left = 0.0f;
    channel_fade_filters[channel].previous_right = 0.0f;
}

/* (Ré)applique le filtre configuré pour un son sur son canal de lecture. */
static void apply_channel_filter(int channel, SoundID id) {
    if (channel < 0 || channel >= MAX_SOUNDS || !is_valid_sound_id(id)) {
        return;
    }

    float new_alpha = compute_low_pass_alpha(sound_cfgs[id].filter_cutoff_hz);
    int want_filter = !sound_cfgs[id].bypass_filter;

    /* Si le filtre est déjà actif avec le même alpha, ne rien faire
       pour ne pas réinitialiser l'état du filtre (previous_left/right). */
    if (channel_filters[channel].active == want_filter) {
        if (!want_filter) {
            return; /* Pas de filtre demandé, pas de filtre actif → OK */
        }
        if (channel_filters[channel].alpha == new_alpha) {
            return; /* Même filtre déjà appliqué → OK */
        }
    }

    Mix_UnregisterAllEffects(channel);
    channel_filters[channel].active = 0;
    channel_filters[channel].previous_left = 0.0f;
    channel_filters[channel].previous_right = 0.0f;

    if (!want_filter) {
        return;
    }

    channel_filters[channel].alpha = new_alpha;
    channel_filters[channel].active = 1;

    if (Mix_RegisterEffect(channel, low_pass_effect, NULL, &channel_filters[channel]) == 0) {
        printf("Erreur Mix_RegisterEffect: %s\n", Mix_GetError());
        channel_filters[channel].active = 0;
    }
}

int audio_init() {
    /* Si l'audio est désactivé via la variable d'environnement, ignorer l'init. */
    {
        const char* v = getenv("CODENAMES_AUDIO_DISABLED");
        if (v && strcmp(v, "0") != 0 && v[0] != '\0') {
            printf("Audio disabled via CODENAMES_AUDIO_DISABLED, skipping audio_init()\n");
            return EXIT_SUCCESS;
        }
    }
    int frequency = 0;
    Uint16 format = 0;
    int channels = 0;

    if (Mix_QuerySpec(&frequency, &format, &channels) == 0) {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            printf("Erreur Mixer: %s\n", Mix_GetError());
            return EXIT_FAILURE;
        }
    }

    Mix_AllocateChannels(MAX_SOUNDS);
    type_volumes[AUDIO_SOUND_KIND_MUSIC] = MIX_MAX_VOLUME;
    type_volumes[AUDIO_SOUND_KIND_SFX] = MIX_MAX_VOLUME;

    /* Initialisation des tableaux internes. */
    for (int i = 0; i < MAX_SOUNDS; i++) {
        sounds[i] = NULL;
        sound_channels[i] = -1;
        sound_cfgs[i].kind = AUDIO_SOUND_KIND_SFX;
        sound_cfgs[i].volume = MIX_MAX_VOLUME;
        sound_cfgs[i].bypass_filter = 1;
        sound_cfgs[i].filter_cutoff_hz = AUDIO_FILTER_DEFAULT_CUTOFF_HZ;
        channel_sound_ids[i] = -1;
        channel_filters[i].active = 0;
        channel_filters[i].alpha = 0.0f;
        channel_filters[i].previous_left = 0.0f;
        channel_filters[i].previous_right = 0.0f;

        channel_fade_filters[i].active = 0;
        channel_fade_filters[i].start_ticks = 0;
        channel_fade_filters[i].duration_ms = 0;
        channel_fade_filters[i].start_alpha = 0.0f;
        channel_fade_filters[i].end_alpha = 0.0f;
        channel_fade_filters[i].deactivate_on_complete = 0;
        channel_fade_filters[i].previous_left = 0.0f;
        channel_fade_filters[i].previous_right = 0.0f;
    }

    /* Chargement des sons. */
    sound_cfgs[MUSIC_MENU_LOBBY].kind = AUDIO_SOUND_KIND_MUSIC;
    sound_cfgs[MUSIC_MENU_LOBBY].kind = AUDIO_SOUND_KIND_MUSIC;
    sounds[MUSIC_MENU_LOBBY] = load_wav_safe("assets/audio/music/menu_lobby.ogg");
    if (!sounds[MUSIC_MENU_LOBBY]) {
        /* affiche un message plus explicite si le fichier est manquant ou le chargement a échoué */
        if (access("assets/audio/music/menu_lobby.ogg", R_OK) != 0) {
            printf("Audio absent: assets/audio/music/menu_lobby.ogg\n");
        } else {
            printf("Erreur Mix_LoadWAV: %s\n", Mix_GetError());
        }
    }

    sound_cfgs[MUSIC_GAME].kind = AUDIO_SOUND_KIND_MUSIC;
    sounds[MUSIC_GAME] = load_wav_safe("assets/audio/music/game.ogg");
    if (!sounds[MUSIC_GAME]) {
        if (access("assets/audio/music/game.ogg", R_OK) != 0) {
            printf("Audio absent: assets/audio/music/game.ogg\n");
        } else {
            printf("Erreur Mix_LoadWAV: %s\n", Mix_GetError());
        }
    }

    sound_cfgs[SOUND_OPENING_CODENAMES].kind = AUDIO_SOUND_KIND_MUSIC;
    sounds[SOUND_OPENING_CODENAMES] = load_wav_safe("assets/audio/music/opening.ogg");
    if (!sounds[SOUND_OPENING_CODENAMES]) {
        if (access("assets/audio/music/opening.ogg", R_OK) != 0) {
            printf("Audio absent: assets/audio/music/opening.ogg\n");
        } else {
            printf("Erreur Mix_LoadWAV: %s\n", Mix_GetError());
        }
    }

    sound_cfgs[SOUND_BUTTON_CLICKED].kind = AUDIO_SOUND_KIND_SFX;
    sounds[SOUND_BUTTON_CLICKED] = load_wav_safe("assets/audio/sfx/button/clicked.ogg");
    if (!sounds[SOUND_BUTTON_CLICKED]) {
        if (access("assets/audio/sfx/button/clicked.ogg", R_OK) != 0) {
            printf("Audio absent: assets/audio/sfx/button/clicked.ogg\n");
        } else {
            printf("Erreur Mix_LoadWAV: %s\n", Mix_GetError());
        }
    }

    sound_cfgs[SOUND_INFO_OPEN].kind = AUDIO_SOUND_KIND_SFX;
    sounds[SOUND_INFO_OPEN] = load_wav_safe("assets/audio/sfx/whoosh_open.ogg");
    if( !sounds[SOUND_INFO_OPEN]) {
        if (access("assets/audio/sfx/whoosh_open.ogg", R_OK) != 0) {
            printf("Audio absent: assets/audio/sfx/whoosh_open.ogg\n");
        } else {
            printf("Erreur Mix_LoadWAV: %s\n", Mix_GetError());
        }
    }

    sound_cfgs[SOUND_INFO_CLOSE].kind = AUDIO_SOUND_KIND_SFX;
    sounds[SOUND_INFO_CLOSE] = load_wav_safe("assets/audio/sfx/whoosh_close.ogg");
    if( !sounds[SOUND_INFO_CLOSE]) {
        if (access("assets/audio/sfx/whoosh_close.ogg", R_OK) != 0) {
            printf("Audio absent: assets/audio/sfx/whoosh_close.ogg\n");
        } else {
            printf("Erreur Mix_LoadWAV: %s\n", Mix_GetError());
        }
    }

    return EXIT_SUCCESS;
}

void audio_play(SoundID id, int loops) {
    (void)audio_play_with_fade(id, loops, 0, AUDIO_FADE_IN_BY_VOLUME, NULL);
}

int audio_play_with_fade(
    SoundID id,
    int loops,
    int fade_in_ms,
    AudioFadeInType fade_type,
    const AudioFadeInFilterParams* filter_params
) {
    if (!is_valid_sound_id(id) || !sounds[id]) {
        return EXIT_FAILURE;
    }

    int effective_volume = compute_effective_volume(id);
    int is_volume_fade_in = (fade_in_ms > 0 && fade_type == AUDIO_FADE_IN_BY_VOLUME && effective_volume > 0);
    int channel = -1;
    int requested_channel = -1;

    if (is_volume_fade_in) {
        /* SDL_mixer fait varier le volume pendant un fade-in :
           on applique donc le volume cible avant Mix_FadeInChannel. */
        requested_channel = find_available_channel();
        if (requested_channel >= 0) {
            Mix_Volume(requested_channel, effective_volume);
        }

        channel = Mix_FadeInChannel(requested_channel, sounds[id], loops, fade_in_ms);
    } else {
        channel = Mix_PlayChannel(-1, sounds[id], loops);
    }

    if (channel < 0) {
        printf("Erreur Mix_PlayChannel: %s\n", Mix_GetError());
        return EXIT_FAILURE;
    }

    if (channel >= MAX_SOUNDS) {
        Mix_HaltChannel(channel);
        return EXIT_FAILURE;
    }

    /* Désassocie l'ancien son lié à ce canal, si nécessaire. */
    if (is_valid_sound_id(channel_sound_ids[channel]) && channel_sound_ids[channel] != id) {
        sound_channels[channel_sound_ids[channel]] = -1;
    }

    /* Si ce son était associé à un autre canal, on retire l'ancienne association. */
    if (sound_channels[id] >= 0 && sound_channels[id] < MAX_SOUNDS && sound_channels[id] != channel) {
        if (channel_sound_ids[sound_channels[id]] == id) {
            channel_sound_ids[sound_channels[id]] = -1;
        }
    }

    sound_channels[id] = channel;
    channel_sound_ids[channel] = id;
    if (!is_volume_fade_in || requested_channel < 0) {
        Mix_Volume(channel, effective_volume);
    }

    if (fade_in_ms <= 0 || fade_type == AUDIO_FADE_IN_BY_VOLUME) {
        apply_channel_filter(channel, id);
        return EXIT_SUCCESS;
    }

    if (fade_type == AUDIO_FADE_IN_BY_FILTER) {
        AudioFilterType filter_type = AUDIO_FILTER_LOW_PASS;
        float start_cutoff_hz = AUDIO_FILTER_MIN_CUTOFF_HZ;
        float end_cutoff_hz = sound_cfgs[id].bypass_filter
            ? AUDIO_FILTER_MAX_CUTOFF_HZ
            : sound_cfgs[id].filter_cutoff_hz;

        if (filter_params) {
            filter_type = filter_params->filter_type;
            start_cutoff_hz = filter_params->start_cutoff_hz;
            end_cutoff_hz = filter_params->end_cutoff_hz;
        }

        if (filter_type != AUDIO_FILTER_LOW_PASS) {
            Mix_HaltChannel(channel);
            reset_channel_state(channel);
            sound_channels[id] = -1;
            return EXIT_FAILURE;
        }

        if (start_cutoff_hz <= 0.0f) {
            start_cutoff_hz = AUDIO_FILTER_MIN_CUTOFF_HZ;
        }
        if (end_cutoff_hz <= 0.0f) {
            end_cutoff_hz = sound_cfgs[id].bypass_filter
                ? AUDIO_FILTER_MAX_CUTOFF_HZ
                : (sound_cfgs[id].filter_cutoff_hz > 0.0f
                    ? sound_cfgs[id].filter_cutoff_hz
                    : AUDIO_FILTER_DEFAULT_CUTOFF_HZ);
        }

        Mix_UnregisterAllEffects(channel);

        channel_fade_filters[channel].active = 1;
        channel_fade_filters[channel].start_ticks = SDL_GetTicks();
        channel_fade_filters[channel].duration_ms = (Uint32)fade_in_ms;
        channel_fade_filters[channel].start_alpha = compute_low_pass_alpha(start_cutoff_hz);
        channel_fade_filters[channel].end_alpha = compute_low_pass_alpha(end_cutoff_hz);
        channel_fade_filters[channel].deactivate_on_complete = sound_cfgs[id].bypass_filter ? 1 : 0;
        channel_fade_filters[channel].previous_left = 0.0f;
        channel_fade_filters[channel].previous_right = 0.0f;

        if (Mix_RegisterEffect(channel, fade_low_pass_effect, NULL, &channel_fade_filters[channel]) == 0) {
            printf("Erreur Mix_RegisterEffect: %s\n", Mix_GetError());
            Mix_HaltChannel(channel);
            reset_channel_state(channel);
            sound_channels[id] = -1;
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    Mix_HaltChannel(channel);
    reset_channel_state(channel);
    sound_channels[id] = -1;
    return EXIT_FAILURE;
}

int audio_set_sound_config(SoundID id, const SoundConfig* cfg) {
    if (!is_valid_sound_id(id) || !cfg || !is_valid_kind(cfg->kind)) {
        return EXIT_FAILURE;
    }

    sound_cfgs[id] = *cfg;
    sound_cfgs[id].volume = clamp_volume(sound_cfgs[id].volume);
    sound_cfgs[id].bypass_filter = sound_cfgs[id].bypass_filter ? 1 : 0;
    if (sound_cfgs[id].filter_cutoff_hz <= 0.0f) {
        sound_cfgs[id].filter_cutoff_hz = AUDIO_FILTER_DEFAULT_CUTOFF_HZ;
    }

    int channel = sound_channels[id];
    if (channel >= 0 && channel < MAX_SOUNDS) {
        if (!is_channel_bound_to_sound(channel, id)) {
            sound_channels[id] = -1;
        } else if (Mix_Playing(channel)) {
            Mix_Volume(channel, compute_effective_volume(id));
            apply_channel_filter(channel, id);
        }
    }

    return EXIT_SUCCESS;
}

int audio_get_sound_config(SoundID id, SoundConfig* out_cfg) {
    if (!is_valid_sound_id(id) || !out_cfg) {
        return EXIT_FAILURE;
    }

    *out_cfg = sound_cfgs[id];
    return EXIT_SUCCESS;
}

int audio_set_sound_volume(SoundID id, int volume) {
    SoundConfig cfg;
    if (audio_get_sound_config(id, &cfg) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    cfg.volume = clamp_volume(volume);
    return audio_set_sound_config(id, &cfg);
}

int audio_set_filter(SoundID id, AudioFilterType filter_type, float cutoff_frequency) {
    SoundConfig cfg;
    if (audio_get_sound_config(id, &cfg) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if (filter_type == AUDIO_FILTER_LOW_PASS) {
        cfg.filter_cutoff_hz = cutoff_frequency > 0.0f ? cutoff_frequency : AUDIO_FILTER_DEFAULT_CUTOFF_HZ;
        cfg.bypass_filter = 0;
    } else if (filter_type == AUDIO_FILTER_NONE) {
        cfg.bypass_filter = 1;
    } else {
        return EXIT_FAILURE;
    }

    return audio_set_sound_config(id, &cfg);
}

int audio_get_filter(SoundID id, AudioFilterType* out_filter_type, float* out_cutoff_frequency) {
    SoundConfig cfg;
    if (!out_filter_type || audio_get_sound_config(id, &cfg) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    *out_filter_type = cfg.bypass_filter ? AUDIO_FILTER_NONE : AUDIO_FILTER_LOW_PASS;

    if (out_cutoff_frequency) {
        *out_cutoff_frequency = cfg.filter_cutoff_hz;
    }

    return EXIT_SUCCESS;
}

void audio_set_type_volume(AudioSoundKind kind, int volume) {
    if (!is_valid_kind(kind)) {
        return;
    }

    type_volumes[kind] = clamp_volume(volume);

    for (int i = 0; i < MAX_SOUNDS; i++) {
        if (sound_cfgs[i].kind != kind) {
            continue;
        }

        int channel = sound_channels[i];
        if (channel < 0 || channel >= MAX_SOUNDS) {
            continue;
        }

        if (!is_channel_bound_to_sound(channel, i)) {
            sound_channels[i] = -1;
            continue;
        }

        if (Mix_Playing(channel)) {
            Mix_Volume(channel, compute_effective_volume(i));
        }
    }
}

void audio_stop(SoundID id) {
    (void)audio_stop_with_fade(id, 0, AUDIO_FADE_OUT_BY_VOLUME, NULL);
}

int audio_stop_with_fade(
    SoundID id,
    int fade_out_ms,
    AudioFadeOutType fade_type,
    const AudioFadeOutFilterParams* filter_params
) {
    if (!is_valid_sound_id(id) || sound_channels[id] == -1) {
        return EXIT_FAILURE;
    }

    int channel = sound_channels[id];
    if (channel < 0 || channel >= MAX_SOUNDS) {
        sound_channels[id] = -1;
        return EXIT_FAILURE;
    }

    if (!is_channel_bound_to_sound(channel, id)) {
        sound_channels[id] = -1;
        return EXIT_FAILURE;
    }

    if (!Mix_Playing(channel)) {
        reset_channel_state(channel);
        sound_channels[id] = -1;
        return EXIT_SUCCESS;
    }

    if (fade_out_ms <= 0) {
        Mix_UnregisterAllEffects(channel);
        Mix_HaltChannel(channel);
        reset_channel_state(channel);
        sound_channels[id] = -1;
        return EXIT_SUCCESS;
    }

    if (fade_type == AUDIO_FADE_OUT_BY_VOLUME) {
        if (Mix_FadingChannel(channel) == MIX_FADING_OUT) {
            return EXIT_SUCCESS;
        }

        Mix_UnregisterAllEffects(channel);
        channel_fade_filters[channel].active = 0;

        if (Mix_FadeOutChannel(channel, fade_out_ms) == 0) {
            Mix_HaltChannel(channel);
            reset_channel_state(channel);
            sound_channels[id] = -1;
        }
        return EXIT_SUCCESS;
    }

    if (fade_type == AUDIO_FADE_OUT_BY_FILTER) {
        AudioFilterType filter_type = AUDIO_FILTER_LOW_PASS;
        float start_cutoff_hz = sound_cfgs[id].filter_cutoff_hz;
        float end_cutoff_hz = AUDIO_FILTER_DEFAULT_CUTOFF_HZ;

        if (filter_params) {
            filter_type = filter_params->filter_type;
            start_cutoff_hz = filter_params->start_cutoff_hz;
            end_cutoff_hz = filter_params->end_cutoff_hz;
        }

        if (filter_type != AUDIO_FILTER_LOW_PASS) {
            return EXIT_FAILURE;
        }

        if (start_cutoff_hz <= 0.0f) {
            start_cutoff_hz = sound_cfgs[id].filter_cutoff_hz > 0.0f
                ? sound_cfgs[id].filter_cutoff_hz
                : AUDIO_FILTER_DEFAULT_CUTOFF_HZ;
        }
        if (end_cutoff_hz <= 0.0f) {
            end_cutoff_hz = AUDIO_FILTER_MIN_CUTOFF_HZ;
        }

        Mix_UnregisterAllEffects(channel);

        channel_fade_filters[channel].active = 1;
        channel_fade_filters[channel].start_ticks = SDL_GetTicks();
        channel_fade_filters[channel].duration_ms = (Uint32)fade_out_ms;
        channel_fade_filters[channel].start_alpha = compute_low_pass_alpha(start_cutoff_hz);
        channel_fade_filters[channel].end_alpha = compute_low_pass_alpha(end_cutoff_hz);
        channel_fade_filters[channel].deactivate_on_complete = 0;
        channel_fade_filters[channel].previous_left = 0.0f;
        channel_fade_filters[channel].previous_right = 0.0f;

        if (Mix_RegisterEffect(channel, fade_low_pass_effect, NULL, &channel_fade_filters[channel]) == 0) {
            printf("Erreur Mix_RegisterEffect: %s\n", Mix_GetError());
            channel_fade_filters[channel].active = 0;
            return EXIT_FAILURE;
        }

        Mix_ExpireChannel(channel, fade_out_ms);
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

int audio_is_playing(SoundID id) {
    if (!is_valid_sound_id(id) || sound_channels[id] == -1) {
        return 0;
    }

    int channel = sound_channels[id];
    if (!is_channel_bound_to_sound(channel, id)) {
        sound_channels[id] = -1;
        return 0;
    }

    if (channel >= 0 && channel < MAX_SOUNDS && Mix_Playing(channel)) {
        return 1;
    }

    if (channel >= 0 && channel < MAX_SOUNDS) {
        reset_channel_state(channel);
    }

    sound_channels[id] = -1;
    return 0;
}

void audio_cleanup() {
    for (int i = 0; i < MAX_SOUNDS; i++) {
        Mix_UnregisterAllEffects(i);
        reset_channel_state(i);
        if (sounds[i]) Mix_FreeChunk(sounds[i]);
        sounds[i] = NULL;
        sound_channels[i] = -1;
    }
    Mix_CloseAudio();
}