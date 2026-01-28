<header>
    <img src=".github/logo_lmu.png" height="64" alt="Logo Le Mans Université">
</header>
<hr/>

# Codenames

Ce projet est un jeu codenames multijoueur en langage C.
Il a été initié en groupe de 4 dans le cadre d'un projet de groupe universitaire.

## Instalation

Après avoir cloné le projet, il faut executer le script `./setup.sh` pour installer les differentes librairies requises

## TODO List

### Client
- Assets visuels
- Interfaces (menus, jeu, options)
- ~~Connexion TCP vers le serveur~~

### Serveur
- ~~Squelette de gestion d'une partie~~
- ~~Serveur TCP~~
- Parser de requetes TCP et action associé
- Gestion du mot noir
- Choix definitif du format de donnée pour la transmission du plateau
- Gestion des connexions client, lobby

## Transmition client/serveur

Tunnel TCP initié par le client.

Format de donnée :
```
HEADER:TYPE:PAYLOAD
```

Headers : `JOIN, LEAVE, CHAT, REVEAL, HINT`<br/>
Payload : `Données styucturé (json), un message textuel ou rien, en fonction du type`


LOGIN {"username":"noam"}