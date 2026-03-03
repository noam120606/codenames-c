#include "../lib/all.h"

SDL_Texture* menu_logo;
SDL_Texture* quagmire;
Input* name_input = NULL;
Input* code_input = NULL;
char* name = NULL;
char* code = NULL;
int joining = 0;

static void name_on_submit(const char* text) {
    printf("Name input submitted: %s\n", text ? text : "");
    if (name) {
        free(name);
        name = NULL;
    }
    if (text) {
        name = strdup(text);
        if (!name) {
            name = malloc(1);
            if (name) name[0] = '\0';
        }
    } else {
        name = NULL;
    }
}

static void code_on_submit(const char* text) {
    printf("Code input submitted: %s\n", text);
    if (code) {
        free(code);
        code = NULL;
    }
    code = malloc(sizeof(char) * strlen(text));
    strcpy(code, text);
}

void menu_handle_event(SDL_Context* context, SDL_Event* e) {
    if (name_input) input_handle_event(name_input, e);
    if (code_input) input_handle_event(code_input, e);
}

ButtonReturn menu_button_click(SDL_Context* context, ButtonId button_id) {
    printf("Button clicked: %d\n", button_id);
    switch (button_id) {
        case BTN_JOIN: joining = 1; break;
        case BTN_CREATE:
            char trame[20];
            format_to(trame, sizeof(trame), "%d %s", MSG_CREATELOBBY, name ? name : "NONE");
            send_tcp(context->sock, trame);
            break;
        case BTN_QUIT: return BTN_RET_QUIT; break;
        default: printf("Unknown button clicked\n"); break;
    }
    return BTN_RET_NONE;
}

int menu_init(SDL_Context * context) {
    int loading_fails = 0;

    // Chargement image
    menu_logo = load_image(context->renderer, "assets/img/others/logo.png");
    if (!menu_logo) {
        printf("Failed to load menu logo image\n");
        loading_fails++;
    }
    quagmire = load_image(context->renderer, "assets/img/others/quagmire.png");
    if (!quagmire) {
        printf("Failed to load quagmire image\n");
        loading_fails++;
    }

    // Chargement bouton
    text_button_create(context->renderer, BTN_CREATE, WIN_WIDTH/2-450, 700, 100, "Créer", FONT_LARABIE, COL_WHITE, menu_button_click);
    text_button_create(context->renderer, BTN_JOIN, WIN_WIDTH/2+50, 700, 100, "Rejoindre", FONT_LARABIE, COL_WHITE, menu_button_click);
    text_button_create(context->renderer, BTN_QUIT, WIN_WIDTH/2-200, 900, 100, "Quitter", FONT_LARABIE, COL_WHITE, menu_button_click);

    // Chargement input
    const char* name_placeholders[] = {"Peter", "Quagmire", "Tom", "Faz Faf"};
    name_input = input_create(INPUT_ID_NAME, WIN_WIDTH/2 + 650, 50, 250, 60, FONT_LARABIE, 28, name_placeholders, 4, "Pseudo : ", 16);
    if (name_input) {
        input_set_on_submit(name_input, name_on_submit);
        input_set_bg(name_input, context->renderer, "assets/img/inputs/empty.png", 24);
    }

    const char* code_placeholders[] = {"CODE"};
    code_input = input_create(INPUT_ID_JOIN_CODE, WIN_WIDTH/2+50, 700, 385, 100, FONT_LARABIE, 28, code_placeholders, 1, "", 16);
    if (code_input) {
        input_set_on_submit(code_input, code_on_submit);
        input_set_bg(code_input, context->renderer, "assets/img/inputs/empty.png", 24);
    }

    return loading_fails;
}

void menu_display(SDL_Context * context) {

    if (context->lobby_id != -1) {
        hide_button(BTN_CREATE);
        hide_button(BTN_JOIN);

        char msg[64];
        format_to(msg, sizeof(msg), "Bienvenue %s ! Tu es lobby %d. Code : %s", name ? name : "invité", context->lobby_id, context->lobby_code ? context->lobby_code : "AUCUN");
        int desired_screen_y = 700;
        int rel_x = 0; // 0 = centré horizontalement
        int rel_y = (WIN_HEIGHT/2) - desired_screen_y; // négatif si desired_screen_y > WIN_HEIGHT/2
        text_display(context->renderer, msg, FONT_LARABIE, 24, COL_WHITE, rel_x, rel_y, 0, 255);

    } else if (joining) {
        show_button(BTN_CREATE);
        hide_button(BTN_JOIN);
        if (code_input) input_render(context->renderer, code_input);
    } else {
        show_button(BTN_CREATE);
        show_button(BTN_JOIN);
    }

    // Afficher le logo à sa taille d'origine
    if (menu_logo) {
        display_image(context->renderer, menu_logo, 0, 200, 0.75, 0, SDL_FLIP_NONE, 1, 255);
    }
    
    // Afficher quagmire à sa taille d'origine
    if (quagmire) {
        display_image(context->renderer, quagmire, 850, -350, 1.0, 0, SDL_FLIP_NONE, 1, 255);
    }

    /* input is drawn by menu */
    if (name_input) input_render(context->renderer, name_input);

    /* submitted text is drawn by input_render */
}

int menu_free() {
    if (menu_logo) free_image(menu_logo);
    if (quagmire) free_image(quagmire);
    if (name_input) input_destroy(name_input);

    return EXIT_SUCCESS;
}