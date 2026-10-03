# NetHack 5.0.0 sous Lubuntu 26.04

Code source de [NetHack](https://www.nethack.org/) 5.0.0, compilé et installé
sous **Lubuntu 26.04**, avec la documentation d'installation, une
configuration de joueur prête à l'emploi et une **traduction française
complète** du jeu.

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

## Jouer en français

La configuration fournie contient `OPTIONS=language:fr` : le jeu démarre
en français. Avec `OPTIONS=language:en` (ou sans l'option), il reste en
anglais.

Sont traduits, pour les interfaces tty et curses :

- tous les messages du jeu, les menus, l'inventaire, les noms des
  monstres et des objets (avec leur genre et leur pluriel), la ligne
  d'état, la pierre tombale et le tableau des scores ;
- le tutoriel, les textes des quêtes de chaque rôle, les rumeurs, les
  oracles, les gravures et les épitaphes ;
- l'encyclopédie (`;` puis `?`), les fichiers d'aide (`?`) et
  l'écran `#version` ;
- le mode magicien et les messages d'erreur internes ; le `paniclog`
  garde le texte anglais pour les rapports de bugs ;
- le guide du joueur (`doc/Guidebook-fr.txt`) et la page de manuel
  (`doc/nethack-fr.txt`).

Restent en anglais : les interfaces X11 et Qt, les citations de Terry
Pratchett (`dat/tribute`), la licence et les noms des options du fichier
de configuration. La traduction doit encore être vérifiée en jeu ; ce
qui reste à faire est listé dans [TODO.md](TODO.md), et son
fonctionnement est décrit dans la section 7 de [INSTALL.md](INSTALL.md).

## Ce qui a été ajouté au code d'origine

| Fichier            | Rôle                                                              |
|--------------------|-------------------------------------------------------------------|
| `install.sh`       | Script d'installation automatique                                 |
| `INSTALL.md`       | Guide d'installation en français pour Lubuntu 26.04               |
| `config/nethackrc` | Exemple de configuration joueur (interface curses, couleurs de menus, filtres de messages) |
| `sys/unix/sysconf` | Ajout de l'utilisateur `util01` à `WIZARDS` (accès au mode debug) |
| `CLAUDE.md`        | Notes pour Claude Code : compilation, tests, architecture du code |
| `src/nhi18n.c`, `include/nhi18n.h` | Traduction des messages du jeu (option `language`) |
| `po/fr.po`         | Traduction française des messages, compilée en `dat/fr.mo`        |
| `po/*.sh`, `po/*.py` | Extraction des textes à traduire (`make update-po`)             |
| `dat/*.fr`, `dat/*-fr.*` | Fichiers d'aide, encyclopédie, rumeurs, oracles... traduits |
| `doc/Guidebook-fr.mn`, `doc/nethack-fr.6` | Guide du joueur et page de manuel en français |
| `TODO.md`          | État de la traduction et ce qui reste à faire                     |

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
