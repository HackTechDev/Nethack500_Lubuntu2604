# NetHack 5.0.0 sous Lubuntu 26.04

Code source de [NetHack](https://www.nethack.org/) 5.0.0, compilé et installé
sous **Lubuntu 26.04**, avec la documentation d'installation et une
configuration de joueur prête à l'emploi.

## Démarrage rapide

```sh
sudo apt install build-essential libncurses-dev
git clone https://github.com/HackTechDev/Nethack500_Lubuntu2604.git
cd Nethack500_Lubuntu2604
cd sys/unix && sh setup.sh hints/linux.500 && cd ../..
make fetch-lua
make WANT_WIN_TTY=1 WANT_WIN_CURSES=1
make install WANT_WIN_TTY=1 WANT_WIN_CURSES=1
cp config/nethackrc ~/.nethackrc
~/nh/install/games/nethack
```

Le détail de chaque étape, les emplacements des fichiers installés et les
précautions à prendre avant une réinstallation sont dans
[INSTALL.md](INSTALL.md).

## Ce qui a été ajouté au code d'origine

| Fichier            | Rôle                                                              |
|--------------------|-------------------------------------------------------------------|
| `INSTALL.md`       | Guide d'installation en français pour Lubuntu 26.04               |
| `config/nethackrc` | Exemple de configuration joueur (interface curses, couleurs de menus, filtres de messages) |
| `sys/unix/sysconf` | Ajout de l'utilisateur `util01` à `WIZARDS` (accès au mode debug) |
| `CLAUDE.md`        | Notes pour Claude Code : compilation, tests, architecture du code |

Le reste du dépôt est le code source de NetHack 5.0.0 sans modification. Les
informations générales de l'équipe NetHack se trouvent dans [README](README).

## Liens

- Site officiel de NetHack : <https://www.nethack.org/>
- Dépôt officiel : <https://github.com/NetHack/NetHack>
- Origine de la configuration joueur :
  <https://labo.hacktech.dev/jeu-libre/installation_roguelike_evilhack>

## Licence

NetHack est distribué sous la NetHack General Public License, voir
[dat/license](dat/license).
