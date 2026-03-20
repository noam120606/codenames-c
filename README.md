<header>
    <img src=".github/logo_lmu.png" height="64" alt="Logo Le Mans Université">
</header>
<hr/>

# Codenames - Jeu Multijoueur en C

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))

## Description

**Codenames** est une implémentation complète du célèbre jeu de société en langage C, développée dans le cadre d'un projet universitaire de groupe de 4 étudiants. Le projet est composé de deux applications distinctes :

- **Client graphique** : Interface utilisateur développée avec SDL2 (Simple DirectMedia Layer)
- **Serveur TCP** : Gestion multi-lobbies et logique de jeu côté serveur

Le jeu permet à jusqu'à **8 joueurs** de s'affronter en équipes (Rouge vs Bleue) avec des rôles d'espions (donnent des indices) et d'agents (devinent les mots).

---

## Fonctionnalités

### Client
- Interface graphique complète avec SDL2
- Menu principal avec création/rejoint de lobby
- Système de lobby avec code de partie
- Affichage de la grille de 25 mots
- Interface différenciée pour espions et agents
- Effets audio (musique, sons)
- Sauvegarde automatique des préférences utilisateur
- Affichage du ping et des FPS
- UUID persistant pour identification unique

### Serveur
- Gestion de jusqu'à 50 lobbies simultanés
- Support de 128 clients TCP concurrents
- Génération aléatoire des grilles de jeu
- Gestion des tours (équipe rouge/bleue)
- Système de messages TCP avec headers
- Gestion automatique des déconnexions
- Validation des coups et détection de fin de partie

---

## Architecture du Projet

```
codenames-c/
├── client/                    # Application cliente (SDL2)
│   ├── src/                   # Code source client
│   ├── lib/                   # Headers client
│   ├── assets/                # Ressources (images, sons, polices)
│   ├── SDL2/                  # Bibliothèques SDL2 compilées
│   ├── build/                 # Fichiers compilés
│   ├── makefile               # Compilation client
│   └── run.sh                 # Script de lancement client
│
├── server/                    # Application serveur (TCP)
│   ├── src/                   # Code source serveur
│   ├── lib/                   # Headers serveur
│   ├── assets/                # Liste de mots
│   ├── build/                 # Fichiers compilés
│   ├── makefile               # Compilation serveur
│   └── run.sh                 # Script de lancement serveur
│
├── docs/                      # Documentation Doxygen générée
├── Doxyfile                   # Configuration Doxygen (projet complet)
├── setup.sh                   # Script d'installation des dépendances
└── README.md                  # Ce fichier
```

---

## Installation

### Prérequis système

Avant de commencer, assurez-vous d'avoir installé les packages suivants sur votre système Linux :

```bash
sudo apt install build-essential pkg-config wget
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
```

**Note** : Le script `setup.sh` compilera automatiquement les bibliothèques SDL2 nécessaires dans le dossier `client/SDL2/`.

### Étapes d'installation

1. **Cloner le dépôt**
   ```bash
   git clone https://github.com/noam120606/codenames-c.git
   cd codenames-c
   ```

2. **Exécuter le script d'installation**
   ```bash
   chmod +x setup.sh
   ./setup.sh
   ```
   
   Ce script va :
   - Télécharger SDL2 (v2.30.0), SDL_image (v2.8.2), SDL_ttf (v2.22.0), SDL_mixer (v2.8.0)
   - Compiler les bibliothèques dans `client/SDL2/`
   - Configurer les chemins d'inclusion

3. **Compiler le serveur**
   ```bash
   cd server
   make
   ```

4. **Compiler le client**
   ```bash
   cd ../client
   make
   ```

---

## Utilisation

### Version automatique à chaque commit

Le projet utilise un fichier `VERSION` au format `MAJOR.MINOR.PATCH`.

Un hook Git `pre-commit` incrémente automatiquement le `PATCH` à chaque commit, puis ajoute `VERSION` au commit en cours. Cette version est ensuite injectée à la compilation dans la macro C `CODENAMES_VERSION` (client et serveur).

Configurer les hooks (une fois par clone) :

```bash
git config core.hooksPath .githooks
chmod +x .githooks/pre-commit scripts/bump_version.sh
```

Après compilation, la version est visible au démarrage du client/serveur (logs console).

### Lancement du serveur

```bash
cd server
./run.sh
# Ou directement : ./build/server
```

Le serveur démarre par défaut sur le **port 4242** et attend les connexions des clients.

### Lancement du client

```bash
cd client
./run.sh
# Ou directement : ./build/client
```

Le client lance une fenêtre graphique **1920x1080** (redimensionnable, minimum 960x540).

### Déroulement d'une partie

1. **Connexion** : Le client se connecte au serveur (par défaut localhost:4242)
2. **Menu principal** : Créer un nouveau lobby ou rejoindre avec un code
3. **Lobby** : Attendre les joueurs et choisir son équipe/rôle
4. **Partie** :
   - Les **espions** voient la couleur des mots et donnent des indices
   - Les **agents** cliquent sur les mots selon les indices
   - L'équipe qui révèle tous ses mots en premier gagne
   - Attention au mot **noir** (assassin) qui fait perdre immédiatement !

---

## Documentation

La documentation complète du code est générable avec **Doxygen**.

### Générer la documentation

```bash
# Documentation complète (client + serveur)
doxygen
```

### Consultation 

- **Documentation complète** : `docs/html/index.html`

Ouvrez le fichier `index.html` dans votre navigateur web pour parcourir la documentation.

---

## Protocole de Communication Client/Serveur

La communication entre le client et le serveur utilise un **tunnel TCP** initié par le client.

### Format des messages

```
CODENAMES [HEADER] [DONNÉES...]
```

- **HEADER** : Entier identifiant le type de message
- **DONNÉES** : Variables séparées par des espaces

### Exemples de headers
- Connexion/authentification
- Création/rejoindre un lobby
- Démarrage de partie
- Sélection d'un mot
- Synchronisation de l'état de jeu
- Déconnexion

*Pour plus de détails, consultez les fichiers `client/lib/tcp.h` et `server/lib/message.h`*

---

## Technologies Utilisées

### Client
- **Langage** : C
- **Graphisme** : SDL2, SDL_image, SDL_ttf
- **Audio** : SDL_mixer
- **Build** : Make, GCC

### Serveur
- **Langage** : C
- **Réseau** : Sockets TCP POSIX
- **Build** : Make, GCC

### Outils
- **Documentation** : Doxygen
- **Contrôle de version** : Git
- **Compilation** : GCC avec flags `-Wall -Wextra -O2`

---

## Fonctionalitées prévues

Consultez le fichier [TODO.md](TODO.md) pour la liste des fonctionnalités à venir et des bugs connus.

---

## Contributeurs

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/WolfGang-PRoxa">
        <img src="https://github.com/WolfGang-PRoxa.png" width="100px;" alt="Romain"/><br />
        <sub><b>Romain</b></sub>
      </a><br />
      <sub>Développeur</sub>
    </td>
    <td align="center">
      <a href="https://github.com/noam120606">
        <img src="https://github.com/noam120606.png" width="100px;" alt="Noam"/><br />
        <sub><b>Noam</b></sub>
      </a><br />
      <sub>Développeur</sub>
    </td>
    <td align="center">
      <a href="https://github.com/cloqtn">
        <img src="https://github.com/cloqtn.png" width="100px;" alt="Chloé"/><br />
        <sub><b>Chloé</b></sub>
      </a><br />
      <sub>Développeuse</sub>
    </td>
    <td align="center">
      <a href="https://github.com/kaptainepirate">
        <img src="https://github.com/kaptainepirate.png" width="100px;" alt="Mathis"/><br />
        <sub><b>Mathis</b></sub>
      </a><br />
      <sub>Développeur</sub>
    </td>
  </tr>
</table>

<p align="center">
  <i>Projet développé dans le cadre d'un projet universitaire à l'Université Le Mans</i>
</p>