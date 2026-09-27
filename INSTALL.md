# Installation de NetHack 5.0.0

Installation effectuée sous **Lubuntu 26.04** (Ubuntu 26.04.1 LTS), avec
GCC 15.2.0, en utilisant le système de « hints » (`sys/unix/hints/linux.500`).
Les interfaces tty et curses sont compilées.

Pour les autres systèmes ou interfaces, voir `sys/unix/NewInstall.unx`.

## Installation automatique

Le script `install.sh` enchaîne les étapes 1 à 5 ci-dessous :

```sh
./install.sh
```

1. installe les paquets manquants (`gcc`, `make`, `curl`, `gettext`,
   `libncurses-dev`) avec `sudo apt-get` ;
2. configure avec `hints/linux.500` et télécharge Lua si `lib/lua` est absent ;
3. compile avec `WANT_WIN_TTY=1 WANT_WIN_CURSES=1` ;
4. si le jeu est déjà installé, copie les parties et les scores (`save`,
   `record`, `logfile`, `xlogfile`, `livelog`, `perm`) dans
   `~/nh/backup-AAAAMMJJ-HHMMSS`, lance `make install`, puis les restaure ;
5. copie `config/nethackrc` dans `~/.nethackrc` ; un `~/.nethackrc`
   différent est d'abord renommé en `~/.nethackrc.AAAAMMJJ-HHMMSS`.

Les copies de sauvegarde dans `~/nh/` ne sont pas supprimées ; les effacer
une fois le jeu vérifié.

Les sections suivantes décrivent les mêmes étapes à la main.

## 1. Dépendances

Les fichiers d'en-tête de ncurses sont indispensables, même pour l'interface
tty seule. Sans eux, la compilation échoue sur
`fatal error: curses.h: Aucun fichier ou dossier de ce nom`.

Le paquet `gettext` fournit `msgfmt`, qui compile la traduction française
(voir « 7. Traduction française »).

```sh
sudo apt install build-essential gettext libncurses-dev
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

Un exemple est fourni dans `config/nethackrc`. Pour l'installer :

```sh
cp config/nethackrc ~/.nethackrc
```

Si un `~/.nethackrc` existe déjà, il est remplacé : le sauvegarder avant si
besoin.

Cet exemple reprend la section « 7/ Fichier de configuration » de
<https://labo.hacktech.dev/jeu-libre/installation_roguelike_evilhack>.
Il active l'interface curses (`OPTIONS=windowtype:curses`), d'où la
compilation avec `WANT_WIN_CURSES=1`.

## 6. Lancement

```sh
~/nh/install/games/nethack
```

### Déplacements

Avec l'interface curses et `OPTIONS=number_pad:0` (réglage de
`config/nethackrc`), les touches fléchées fonctionnent en plus des touches
vi, sans configuration supplémentaire :

| Direction          | Lettre | Touche      |
|--------------------|--------|-------------|
| Gauche             | `h`    | ←           |
| Bas                | `j`    | ↓           |
| Haut               | `k`    | ↑           |
| Droite             | `l`    | →           |
| Haut-gauche        | `y`    | Début       |
| Haut-droite        | `u`    | Page préc.  |
| Bas-gauche         | `b`    | Fin         |
| Bas-droite         | `n`    | Page suiv.  |

Si les flèches ne déplacent pas le personnage, vérifier que le jeu s'affiche
bien en curses (fenêtres avec bordures) : sinon, `~/.nethackrc` n'est pas lu.

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

## 7. Traduction française

La traduction est en cours (environ 940 textes sur 4 700). Sont traduits :
la création du personnage, le message de bienvenue, la ligne de statut
(avec accord au féminin des rangs et de l'alignement), les catégories de
l'inventaire, les invites de choix d'objet (« Que voulez-vous manger ? »),
les questions pour quitter ou sauvegarder, et plusieurs centaines de
messages courants (portes, déplacements, nourriture, gravure, détection).
Les noms d'objets et de monstres, et donc les messages qui les contiennent
(combats notamment), restent en anglais. Pour l'activer, ajouter dans
`~/.nethackrc` :

```
OPTIONS=language:fr
```

Les traductions sont dans `po/fr.po`. `make` le compile en `dat/fr.mo`, que
`make install` copie dans le dossier du jeu. Si le catalogue est absent, le
jeu affiche « No message catalog for language 'fr' » au démarrage et reste
en anglais.

Pour traduire de nouveaux messages :

1. les messages passés à `pline()`, `You()`, `pline_The()`, `yn_function()`,
   etc. sont traduits automatiquement ; un texte construit autrement
   (`Sprintf`, menus...) doit être entouré de `_()` :
   `Sprintf(buf, _("There is %s here."), ...)` ;
2. extraire les textes et mettre à jour `po/fr.po` : `make update-po`
   (script `po/update-pot.sh`) ;
3. traduire les entrées vides (`msgstr ""`) de `po/fr.po` ;
4. recompiler et réinstaller (`./install.sh`).

Les `%s`, `%d`, etc. de la traduction doivent correspondre à ceux du texte
anglais (même ordre, ou ordre changé avec `%1$s`, `%2$s` ; les derniers
peuvent être omis). Sinon, `msgfmt --check` refuse le catalogue, et le jeu
ignore de toute façon une traduction incohérente et affiche le texte
anglais.

Les règles `MSGTYPE` et `hilite_status` de la configuration peuvent rester
en anglais : elles s'appliquent aussi aux messages traduits.

Les entrées `msgctxt "feminine"` donnent la forme féminine d'un mot (rang,
race, alignement) pour une héroïne ; sans elle, la forme masculine est
utilisée. Une entrée marquée `#, fuzzy` par `make update-po` est une
traduction devinée, ignorée par le jeu tant qu'elle n'a pas été vérifiée
et le marqueur retiré (`make update-po` n'en crée pas).
