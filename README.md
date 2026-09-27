# NetHack 5.0.0 sous Lubuntu 26.04

Code source de [NetHack](https://www.nethack.org/) 5.0.0, compilé et installé
sous **Lubuntu 26.04**, avec la documentation d'installation et une
configuration de joueur prête à l'emploi.

## Démarrage rapide

```sh
git clone https://github.com/HackTechDev/Nethack500_Lubuntu2604.git
cd Nethack500_Lubuntu2604
./install.sh
~/nh/install/games/nethack
```

`install.sh` installe les paquets manquants (via `sudo`), compile le jeu avec
les interfaces tty et curses, l'installe en conservant les parties et les
scores existants, puis copie `config/nethackrc` dans `~/.nethackrc`.

Le détail de chaque étape, les emplacements des fichiers installés et les
précautions à prendre avant une réinstallation sont dans
[INSTALL.md](INSTALL.md).

## Ce qui a été ajouté au code d'origine

| Fichier            | Rôle                                                              |
|--------------------|-------------------------------------------------------------------|
| `install.sh`       | Script d'installation automatique                                 |
| `INSTALL.md`       | Guide d'installation en français pour Lubuntu 26.04               |
| `config/nethackrc` | Exemple de configuration joueur (interface curses, couleurs de menus, filtres de messages) |
| `sys/unix/sysconf` | Ajout de l'utilisateur `util01` à `WIZARDS` (accès au mode debug) |
| `CLAUDE.md`        | Notes pour Claude Code : compilation, tests, architecture du code |
| `src/nhi18n.c`, `include/nhi18n.h` | Traduction des messages du jeu (option `language`) |
| `po/fr.po`         | Traduction française des messages (en cours)                      |

Le reste du dépôt est le code source de NetHack 5.0.0, avec en plus le
marquage `_()` des messages traduits et la prise en charge de l'option
`language` (voir la section 7 de [INSTALL.md](INSTALL.md)). Les
informations générales de l'équipe NetHack se trouvent dans [README](README).

## Liens

- Site officiel de NetHack : <https://www.nethack.org/>
- Dépôt officiel : <https://github.com/NetHack/NetHack>
- Origine de la configuration joueur :
  <https://labo.hacktech.dev/jeu-libre/installation_roguelike_evilhack>

## Licence

NetHack est distribué sous la NetHack General Public License, voir
[dat/license](dat/license).
