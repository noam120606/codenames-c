/**
 * @file text.h
 * @brief Rendu et gestion du texte à l'écran.
 */

#ifndef TEXT_H
#define TEXT_H

#include "sdl.h"

/**
 * Structure de configuration pour le rendu de texte.
 * @param font_path Chemin vers le fichier de police TTF.
 * @param font_size Taille de la police.
 * @param color Couleur du texte.
 * @param x Position X du texte.
 * @param y Position Y du texte.
 * @param angle Angle de rotation du texte.
 * @param opacity Opacité du texte.
 */
typedef struct TextConfig {
    const char* font_path;
    int font_size;
    SDL_Color color;
    int x;
    int y;
    double angle;
    Uint8 opacity;
} TextConfig;

/**
 * Structure pour le rendu de texte.
 * @param content Contenu textuel de la chaîne.
 * @param cfg Paramètres de configuration pour le rendu du texte.
 * @param texture Texture SDL utilisée pour le rendu du texte.
 */
typedef struct Text {
    char* content;
    TextConfig cfg;
    SDL_Texture* texture;
} Text;

/**
 * Charge une police TTF.
 * @param font_path Chemin vers le fichier de police TTF.
 * @param size Taille de la police.
 * @return Pointeur vers la police TTF_Font chargée, ou NULL en cas d'échec.
 */
TTF_Font* load_font(const char* font_path, int size);

/**
 * Crée une configuration de texte.
 * @param font_path Chemin vers le fichier de police TTF.
 * @param size Taille de la police.
 * @param color Couleur du texte.
 * @param x Position X du texte.
 * @param y Position Y du texte.
 * @param angle Angle de rotation du texte.
 * @param opacity Opacité du texte.
 * @return Structure TextConfig initialisée avec les valeurs fournies.
 */
TextConfig create_text_config(const char* font_path, int size, SDL_Color color, int x, int y, double angle, Uint8 opacity);

/**
 * Recharge un objet texte.
 * @param context Contexte de l'application.
 * @param text Objet texte à recharger.
 */
void reload_text(AppContext* context, Text* text);

/**
 * Crée un objet texte.
 * @param content Contenu textuel de la chaîne.
 * @param cfg Paramètres de configuration pour le rendu du texte.
 * @return Pointeur vers l'objet Text créé, ou NULL en cas d'échec.
 */
Text* create_text(const char* content, TextConfig cfg);

/**
 * Initialise un objet texte.
 * @param context Contexte de l'application.
 * @param content Contenu textuel de la chaîne.
 * @param cfg Paramètres de configuration pour le rendu du texte.
 * @return Pointeur vers l'objet Text initialisé, ou NULL en cas d'échec.
 */
Text* init_text(AppContext* context, const char* content, TextConfig cfg);

/**
 * Met à jour le contenu d'un objet texte.
 * @param context Contexte de l'application.
 * @param text Objet texte à mettre à jour.
 * @param new_content Nouveau contenu pour l'objet texte.
 */
void update_text(AppContext* context, Text* text, const char* new_content);

/**
 * Met à jour la position d'un objet texte.
 * @param text Objet texte à mettre à jour.
 * @param x Nouvelle position X du texte.
 * @param y Nouvelle position Y du texte.
 */
void update_text_position(Text* text, int x, int y);

/**
 * Met à jour la couleur d'un objet texte.
 * @param context Contexte de l'application.
 * @param text Objet texte à mettre à jour.
 * @param color Nouvelle couleur pour l'objet texte.
 */
void update_text_color(AppContext* context, Text* text, SDL_Color color);

/**
 * Affiche un objet texte.
 * @param context Contexte de l'application.
 * @param text Objet texte à afficher.
 */
void display_text(AppContext* context, Text* text);

/**
 * Détruit un objet texte et libère ses ressources.
 * @param text Objet texte à détruire.
 * @return 0 en cas de succès, -1 en cas d'échec.
 */
int destroy_text(Text* text);

#endif /* TEXT_H */