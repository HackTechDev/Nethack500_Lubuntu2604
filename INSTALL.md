# Installation de NetHack 5.0.0

Installation effectuée sous **Lubuntu 26.04** (Ubuntu 26.04.1 LTS), avec
GCC 15.2.0, en utilisant le système de « hints » (`sys/unix/hints/linux.500`).
Les interfaces tty et curses sont compilées.

Pour les autres systèmes ou interfaces, voir `sys/unix/NewInstall.unx`.

## 1. Dépendances

Les fichiers d'en-tête de ncurses sont indispensables, même pour l'interface
tty seule. Sans eux, la compilation échoue sur
`fatal error: curses.h: Aucun fichier ou dossier de ce nom`.

```sh
sudo apt install build-essential libncurses-dev
```

## 2. Configuration

Depuis le dossier racine des sources :

```sh
cd sys/unix
sh setup.sh hints/linux.500
cd ../..
make fetch-lua
```

`make fetch-lua` télécharge les sources de Lua 5.4.8 dans `lib/`. Il suffit de
le faire une seule fois.

## 3. Compilation

```sh
make WANT_WIN_TTY=1 WANT_WIN_CURSES=1
```

Sans ces variables, seule l'interface tty est compilée. Pour passer d'une
compilation tty seule à tty + curses, nettoyer d'abord avec
`make clean-keep-lib` (conserve Lua dans `lib/`).

Le binaire obtenu est `src/nethack` et l'archive des données `dat/nhdat`.

Pendant la génération du Guidebook, groff peut afficher
`troff: cannot load font 'S' for emboldening`. Cet avertissement ne concerne
que la documentation et n'empêche pas la compilation.

En cas d'échec, nettoyer avec `make spotless` avant de recommencer à partir
de l'étape 2.

## 4. Installation

```sh
make install WANT_WIN_TTY=1 WANT_WIN_CURSES=1
```

Passer les mêmes variables qu'à la compilation : `make install` dépend de la
cible de compilation et doit voir la même configuration d'interfaces.

Le jeu est installé dans le dossier personnel, sans droits root :

| Élément                       | Emplacement                                 |
|-------------------------------|---------------------------------------------|
| Script de lancement           | `~/nh/install/games/nethack`                |
| Données, sauvegardes, scores  | `~/nh/install/games/lib/nethackdir/`        |
| Configuration système         | `~/nh/install/games/lib/nethackdir/sysconf` |
| Configuration du joueur       | `~/.nethackrc`                              |

**Attention :** `make install` supprime puis recrée
`~/nh/install/games/lib/nethackdir`. Avant de réinstaller, mettre de côté
les sauvegardes (`save/`), le tableau des scores (`record`, `logfile`,
`xlogfile`) et tout `sysconf` modifié à la main.

## 5. Configuration du joueur

Les options de jeu (interface, couleurs, `MENUCOLOR`, `MSGTYPE`,
`AUTOPICKUP_EXCEPTION`, etc.) vont dans `~/.nethackrc`, et non dans le
`sysconf` : celui-ci est réservé aux réglages système (`WIZARDS`, etc.) et
il est écrasé à chaque `make install`.

Le fichier utilisé est l'exemple de la section « 7/ Fichier de
configuration » de
<https://labo.hacktech.dev/jeu-libre/installation_roguelike_evilhack>.
Il active l'interface curses (`OPTIONS=windowtype:curses`), d'où la
compilation avec `WANT_WIN_CURSES=1`.

## 6. Lancement

```sh
~/nh/install/games/nethack
```

### Mode debug (wizard mode)

Le fichier `sys/unix/sysconf` de ce dépôt autorise l'utilisateur `util01` :

```
WIZARDS=root games util01
```

Ce fichier est recopié à chaque `make install`. Pour un autre compte,
ajouter son nom à cette ligne avant d'installer (ou modifier directement le
`sysconf` installé). Le mode debug se lance avec :

```sh
~/nh/install/games/nethack -D
```
