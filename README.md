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
les interfaces tty, curses, X11 et Qt (`./install.sh --console` pour tty et
curses seulement), l'installe en conservant les parties et les
scores existants, puis copie `config/nethackrc` dans `~/.nethackrc`.

Le détail de chaque étape, les emplacements des fichiers installés et les
précautions à prendre avant une réinstallation sont dans
[INSTALL.md](INSTALL.md).

### Interfaces X11 et Qt

Les interfaces graphiques ont besoin des paquets de développement
`libxaw7-dev` (widgets Athena, pour X11) et `qt6-multimedia-dev` (sons de
Qt 6, qui fait venir le reste de Qt) ; `install.sh` les installe :

```sh
sudo apt install libxaw7-dev qt6-multimedia-dev
```

On choisit l'interface au lancement :

```sh
~/nh/install/games/nethack -wX11
~/nh/install/games/nethack -wQt
```

Leurs menus, boutons et boîtes de dialogue sont traduits comme le reste du
jeu ; X11 affiche le texte en Latin-1 (polices X classiques), si bien que
`œ` y devient `oe`. Pour compiler à la main :

```sh
make WANT_WIN_ALL=1 WANT_WIN_QT6=1 QT6MANUAL=1 HOSTTYPE=x86_64 all
```

Après un changement d'interfaces, supprimer d'abord les objets
(`rm src/*.o`), sinon l'édition de liens échoue.

### Erreur d'édition de liens avec Lua

**Problème.** La compilation s'arrête à l'édition des liens de `nethack`
sur des dizaines de « référence indéfinie vers « luaG_runerror » » (ou
`lua_getstack`, `lua_getinfo`, `luaG_typeerror`…), suivies de
`internal symbol 'luaG_runerror' isn't defined` et
`ld returned 1 exit status`. Le code de NetHack n'est pas en cause :
la bibliothèque Lua compilée, `lib/lua/liblua-5.4.8.a`, est abîmée.
`ldebug.o` y est bien présent et complet, mais l'index des symboles de
l'archive ne contient aucun de ses symboles ; le lieur ne les trouve
donc pas. On le vérifie avec :

```sh
nm -s lib/lua/liblua-5.4.8.a | grep -c "in ldebug.o"   # 0 : archive abîmée
```

La cause exacte n'a pas été trouvée : Lua se compile en série et une
reconstruction donne une archive correcte. Le plus probable est un
incident pendant la première compilation (script interrompu, ou lancé
deux fois en même temps).

**Résolution.** Supprimer la bibliothèque Lua compilée et `include/nhlua.h`
(tous regénérés), puis relancer le script :

```sh
rm -rf lib/lua include/nhlua.h lib/lua-5.4.8/src/*.o lib/lua-5.4.8/src/liblua.a
./install.sh
```

Le message `troff: cannot load font 'S' for emboldening` affiché pendant
la compilation du Guidebook n'est qu'un avertissement sans conséquence.

## Jouer en français

La configuration fournie contient `OPTIONS=language:fr` : le jeu démarre
en français. Avec `OPTIONS=language:en` (ou sans l'option), il reste en
anglais.

Sont traduits, pour les interfaces tty, curses, X11 et Qt :

- tous les messages du jeu, les menus, l'inventaire, les noms des
  monstres et des objets (avec leur genre et leur pluriel), la ligne
  d'état, la pierre tombale et le tableau des scores ;
- le tutoriel, les textes des quêtes de chaque rôle, les rumeurs, les
  oracles, les gravures et les épitaphes ;
- l'encyclopédie (`;` puis `?`), les fichiers d'aide (`?`) et
  l'écran `#version` ;
- le mode magicien et les messages d'erreur internes ; le `paniclog`
  garde le texte anglais pour les rapports de bugs ;
- le guide du joueur, en texte (`doc/Guidebook-fr.txt`) et en PDF
  (`make Guidebook-fr.pdf` dans `doc/`, il faut pdflatex), les pages de
  manuel (`doc/nethack-fr.txt`, et `recover-fr.txt`, `dlb-fr.txt`,
  `makedefs-fr.txt` pour les outils) et un aide-mémoire des commandes
  (`doc/commandes_nethack.md`).

Restent en anglais : les citations de Terry
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
| `doc/Guidebook-fr.mn`, `doc/*-fr.6` | Guide du joueur et pages de manuel en français |
| `doc/Guidebook-fr.tex`, `doc/mn2tex-fr.py` | Guide du joueur en LaTeX, produit à partir de `Guidebook-fr.mn` |
| `doc/commandes_nethack.md` | Aide-mémoire des principales commandes, en français  |
| `TODO.md`          | État de la traduction et ce qui reste à faire                     |
| `STAT_TRANSLATION.md` | Résumé de l'état de la traduction                              |

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
