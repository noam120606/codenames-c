#include "../lib/all.h"

#define FILTER_DEFAULT_CUTOFF_HZ 1200.0f
#define AUDIO_PI 3.14159265358979323846f

typedef struct {
    int active;
    float alpha;
    float previous_left;
    float previous_right;
} ChannelFilterState;

static Mix_Chunk* sounds[MAX_SOUNDS];
static int sound_channels[MAX_SOUNDS];
static SoundConfig sound_cfgs[MAX_SOUNDS];
static int type_volumes[2] = {MIX_MAX_VOLUME, MIX_MAX_VOLUME};
static SoundID channel_sound_ids[MAX_SOUNDS];
static ChannelFilterState channel_filters[MAX_SOUNDS];

static int clamp_volume(int volume) {
    if (volume < 0) return 0;
    if (volume > MIX_MAX_VOLUME) return MIX_MAX_VOLUME;
    return volume;
}

static int is_valid_sound_id(SoundID id) {
    return id >= 0 && id < MAX_SOUNDS;
}

static int is_valid_kind(AudioSoundKind kind) {
    return kind == AUDIO_SOUND_KIND_MUSIC || kind == AUDIO_SOUND_KIND_SFX;
}

static int compute_effective_volume(SoundID id) {
    SoundConfig* cfg = &sound_cfgs[id];
    int kind_volume = type_volumes[cfg->kind];
    return (cfg->volume * kind_volume) / MIX_MAX_VOLUME;
}

static float compute_low_pass_alpha(float cutoff_hz) {
    int frequency = 0;
    Uint16 format = 0;
    int channels = 0;

    if (Mix_QuerySpec(&frequency, &format, &channels) == 0 || frequency <= 0) {
        frequency = 44100;
    }

    if (cutoff_hz <= 0.0f) {
        cutoff_hz = FILTER_DEFAULT_CUTOFF_HZ;
    }

    float dt = 1.0f / (float)frequency;
    float rc = 1.0f / (2.0f * AUDIO_PI * cutoff_hz);
    return dt / (rc + dt);
}

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

static void reset_channel_state(int channel) {
    if (channel < 0 || channel >= MAX_SOUNDS) {
        return;
    }

    channel_sound_ids[channel] = -1;
    channel_filters[channel].active = 0;
    channel_filters[channel].alpha = 0.0f;
    channel_filters[channel].previous_left = 0.0f;
    channel_filters[channel].previous_right = 0.0f;
}

static void apply_channel_filter(int channel, SoundID id) {
    if (channel < 0 || channel >= MAX_SOUNDS || !is_valid_sound_id(id)) {
        return;
    }

    Mix_UnregisterAllEffects(channel);
    channel_filters[channel].active = 0;
    channel_filters[channel].previous_left = 0.0f;
    channel_filters[channel].previous_right = 0.0f;

    if (sound_cfgs[id].bypass_filter) {
        return;
    }

    channel_filters[channel].alpha = compute_low_pass_alpha(sound_cfgs[id].filter_cutoff_hz);
    channel_filters[channel].active = 1;

    if (Mix_RegisterEffect(channel, low_pass_effect, NULL, &channel_filters[channel]) == 0) {
        printf("Erreur Mix_RegisterEffect: %s\n", Mix_GetError());
        channel_filters[channel].active = 0;
    }
}

int audio_init() {
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

    // Initialize arrays
    for (int i = 0; i < MAX_SOUNDS; i++) {
        sounds[i] = NULL;
        sound_channels[i] = -1;
        sound_cfgs[i].kind = AUDIO_SOUND_KIND_SFX;
        sound_cfgs[i].volume = MIX_MAX_VOLUME;
        sound_cfgs[i].bypass_filter = 1;
        sound_cfgs[i].filter_cutoff_hz = FILTER_DEFAULT_CUTOFF_HZ;
        channel_sound_ids[i] = -1;
        channel_filters[i].active = 0;
        channel_filters[i].alpha = 0.0f;
        channel_filters[i].previous_left = 0.0f;
        channel_filters[i].previous_right = 0.0f;
    }

    // Load menu music as a chunk so it can be played on a channel
    sound_cfgs[MUSIC_MENU_LOBBY].kind = AUDIO_SOUND_KIND_MUSIC;
    sounds[MUSIC_MENU_LOBBY] = Mix_LoadWAV("assets/audio/music_menu_lobby.ogg");
    if (!sounds[MUSIC_MENU_LOBBY]) {
        printf("Erreur Mix_LoadWAV: %s\n", Mix_GetError());
    }

    sound_cfgs[MUSIC_GAME].kind = AUDIO_SOUND_KIND_MUSIC;
    sounds[MUSIC_GAME] = Mix_LoadWAV("assets/audio/music_game.ogg");
    if (!sounds[MUSIC_GAME]) {
        printf("Erreur Mix_LoadWAV: %s\n", Mix_GetError());
    }

    sound_cfgs[SOUND_BUTTON_CLICKED].kind = AUDIO_SOUND_KIND_SFX;
    sounds[SOUND_BUTTON_CLICKED] = Mix_LoadWAV("assets/audio/sound_button_clicked.ogg");
    if (!sounds[SOUND_BUTTON_CLICKED]) {
        printf("Erreur Mix_LoadWAV: %s\n", Mix_GetError());
    }

    return EXIT_SUCCESS;
}

void audio_play(SoundID id, int loops) {
    if (!is_valid_sound_id(id) || !sounds[id]) {
        return;
    }

    int channel = Mix_PlayChannel(-1, sounds[id], loops);
    if (channel < 0) {
        printf("Erreur Mix_PlayChannel: %s\n", Mix_GetError());
        return;
    }

    if (channel >= MAX_SOUNDS) {
        return;
    }

    sound_channels[id] = channel;
    channel_sound_ids[channel] = id;
    Mix_Volume(channel, compute_effective_volume(id));
    apply_channel_filter(channel, id);
}

int audio_set_sound_config(SoundID id, const SoundConfig* cfg) {
    if (!is_valid_sound_id(id) || !cfg || !is_valid_kind(cfg->kind)) {
        return EXIT_FAILURE;
    }

    sound_cfgs[id] = *cfg;
    sound_cfgs[id].volume = clamp_volume(sound_cfgs[id].volume);
    sound_cfgs[id].bypass_filter = sound_cfgs[id].bypass_filter ? 1 : 0;
    if (sound_cfgs[id].filter_cutoff_hz <= 0.0f) {
        sound_cfgs[id].filter_cutoff_hz = FILTER_DEFAULT_CUTOFF_HZ;
    }

    int channel = sound_channels[id];
    if (channel >= 0 && channel < MAX_SOUNDS && Mix_Playing(channel)) {
        Mix_Volume(channel, compute_effective_volume(id));
        apply_channel_filter(channel, id);
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

int audio_set_sound_bypass_filter(SoundID id, int bypass_filter) {
    SoundConfig cfg;
    if (audio_get_sound_config(id, &cfg) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    cfg.bypass_filter = bypass_filter ? 1 : 0;
    return audio_set_sound_config(id, &cfg);
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
        if (channel >= 0 && channel < MAX_SOUNDS && Mix_Playing(channel)) {
            Mix_Volume(channel, compute_effective_volume(i));
        }
    }
}

void audio_stop(SoundID id) {
    if (!is_valid_sound_id(id) || sound_channels[id] == -1) {
        return;
    }

    int channel = sound_channels[id];
    if (channel >= 0 && channel < MAX_SOUNDS) {
        Mix_UnregisterAllEffects(channel);
        Mix_HaltChannel(channel);
        reset_channel_state(channel);
        sound_channels[id] = -1;
    }
}

int audio_is_playing(SoundID id) {
    if (!is_valid_sound_id(id) || sound_channels[id] == -1) {
        return 0;
    }

    int channel = sound_channels[id];
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