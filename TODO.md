# Traduction française : reste à faire

> **Attention : c'est un travail important.** Environ la moitié des textes
> du jeu restent en anglais. La tâche principale (la grammaire française
> des noms d'objets et de monstres, section 1) demande de modifier en
> profondeur la façon dont NetHack compose ses phrases. Les fichiers de
> données (section 3) représentent à eux seuls plus de 20 000 lignes de
> texte. Il faut compter de nombreuses sessions de travail, et des tests
> en jeu à chaque étape.

État actuel : 2 217 textes traduits sur 4 734 dans `po/fr.po`. Le
fonctionnement de la traduction et la façon d'ajouter des traductions sont
décrits dans la section 7 de [INSTALL.md](INSTALL.md).

## 1. Grammaire française des noms (priorité haute)

C'est ce qui bloque la plupart des messages restants, dont tous les
combats (« The fox bites! », « You kill the newt! »).

- **Noms de monstres** : 396 entrées dans `include/monsters.h`, à traduire
  avec leur genre (le renard, la vipère). Ne pas traduire la table
  elle-même : les noms anglais servent aux options, aux vœux et aux
  fichiers de sauvegarde. Les marquer `N_()` et les traduire à l'affichage.
- **Noms d'objets** : 380 entrées dans `include/objects.h`, plus les
  descriptions des objets non identifiés (« bubbly potion », « scroll
  labeled FOOBIE BLETCH »), les matériaux et les appellations données par
  le joueur.
- **Composition des noms** (`src/objnam.c`) : `xname()`, `doname()`,
  `an()`, `the()`, `makeplural()`, et l'ordre des adjectifs
  (« uncursed +0 long sword » → « épée longue +0 non maudite »), avec les
  accords en genre et en nombre.
- **Noms de monstres dans les phrases** (`src/do_name.c`) : `Monnam()`,
  `mon_nam()`, `a_monnam()`, `x_monnam()`, et le possessif
  `s_suffix()` (« the fox's » → « du renard »).
- **Articles et contractions** : le/la/l'/les, de + le = du,
  à + le = au, élision devant voyelle ou h muet.
- **Conjugaison** : `vtense()` et `otense()` accordent le verbe anglais avec
  le sujet ; il faut une solution pour les verbes français.
- **Vœux** (`readobjnam()` dans `objnam.c`) : comprendre un vœu tapé en
  français (« 2 potions bénies de vitesse »), en gardant l'anglais possible.

## 2. Messages composés

- **Messages construits avec `Sprintf`** puis affichés : ils ne sont pas
  trouvés dans le catalogue. Exemples : « Really step onto that falling
  rock trap? », « You already found a monster.  Use 'm' prefix... ».
  Il faut entourer le format de `_()` à l'endroit où il est construit.
- **Morceaux de phrase** assemblés par le code (« You feel %s. » avec un
  adjectif, « %s %s %s » ...) : réécrire en phrases complètes quand c'est
  possible.
- **Verbes de `getobj()` passés par variable** (non traduits) : frotter une
  pierre de touche (`apply.c`, `stonebuf`), tremper un objet dans une
  potion (`potion.c`), ouvrir une boîte de conserve (`eat.c`).
- **Descriptions d'escaliers** (`stairs_description()` dans `stairs.c`) :
  « staircase down to level 3 », « staircase up out of the dungeon ». Pour
  l'instant, la phrase entière reste en anglais.
- **Simplifications faites pour le français**, à revoir : la question
  « Shall I pick... » ne précise plus ce qui reste à choisir ; la mention
  « adrift » a disparu du message de bienvenue.

## 3. Fichiers de données (`dat/`)

Ces textes ne passent pas par `po/fr.po` : il faut un mécanisme pour
charger une version française de chaque fichier (par exemple `help.fr`,
choisi selon l'option `language`).

| Fichier | Lignes | Contenu |
|---|---:|---|
| `data.base` | 6 528 | Encyclopédie (commande `;` puis `?`) |
| `tribute` | 9 942 | Citations des romans de Terry Pratchett (livres dans le jeu) |
| `quest.lua` | 3 087 | Textes des quêtes de chaque rôle |
| `bogusmon.txt` | 562 | Noms de monstres hallucinés |
| `history`, `epitaph.txt` | 401 chacun | Historique, épitaphes |
| `rumors.tru`, `rumors.fal` | environ 390 chacun | Rumeurs (biscuits de fortune) |
| `opthelp` | 393 | Aide des options |
| `cmdhelp`, `help`, `hh`, `keyhelp` | 658 au total | Aide des commandes et des touches |
| `oracles.txt` | 105 | Consultations de l'Oracle |
| `engrave.txt` | 93 | Inscriptions au sol |

Il faut aussi traduire les messages des niveaux spéciaux, écrits en Lua
dans `dat/*.lua` (Sokoban, Mines, Oracle, tutoriel...).

## 4. Interface

- **Menus et fenêtres** : les textes passés à `add_menu()`, `putstr()`,
  `end_menu()` ne sont pas traduits automatiquement (menu des options `O`,
  aide `?`, `#overview`, `#conduct`, fin de partie, liste des commandes
  étendues `#`).
- **Noms des commandes étendues** : à garder en anglais pour la saisie ;
  seules leurs descriptions sont à traduire.
- **Alignement des colonnes** : une lettre accentuée compte pour deux
  caractères dans les calculs de largeur (tty et curses). Des colonnes
  sont décalées d'un caractère, par exemple « Pièces  ('$') » dans
  l'inventaire. Il faudrait compter les caractères UTF-8 et non les octets.
- **Pierre tombale et tableau des scores** (`rip.c`, `topten.c`) : la cause
  de la mort (« killed by a goblin ») est enregistrée dans `logfile`,
  `xlogfile` et `record`. Elle doit rester en anglais dans ces fichiers,
  mais pourrait être traduite à l'affichage.

## 5. Laissé volontairement en anglais

- Commandes de débogage (`wizcmds.c`) et messages techniques internes
  (`impossible()`, `panic()`, erreurs de fichiers).
- Fichiers `livelog`, `dumplog` et `xlogfile`, lus par des outils externes.
- Noms des options et valeurs du fichier de configuration.

## 6. Documentation

- Guidebook (`doc/Guidebook.mn`) : le projet nethack-fr (SourceForge,
  abandonné en 2014) en proposait une traduction française, pour une
  version ancienne ; elle pourrait servir de point de départ.
- Pages de manuel (`doc/nethack.6`...).
