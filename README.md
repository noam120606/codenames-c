<header>
    <img src=".github/logo_lmu.png" height="64" alt="Logo Le Mans Université">
</header>
<hr/>

# Codenames

Ce projet est un jeu codenames multijoueur en langage C.
Il a été initié en groupe de 4 dans le cadre d'un projet de groupe universitaire.

## Instalation

Après avoir cloné le projet, il faut executer le script `./setup.sh` pour installer les differentes librairies requises

Avant lancement, installez les packages requis si ils ne sont pas présent sur votre systeme :
```
sudo apt install build-essential pkg-config libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```


## Transmition client/serveur

Tunnel TCP initié par le client.

Format de donnée :
```
CODENAMES HEADER [...Données]
```

Headers : entier<br/>
Données : `Variables séparé par des espaces`
