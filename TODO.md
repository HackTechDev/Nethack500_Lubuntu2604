# Traduction française : reste à faire

> **Attention : c'est un travail important.** Environ la moitié des textes
> du jeu restent en anglais. La tâche principale (la grammaire française
> des noms d'objets et de monstres, section 1) demande de modifier en
> profondeur la façon dont NetHack compose ses phrases. Les fichiers de
> données (section 3) représentent à eux seuls plus de 20 000 lignes de
> texte. Il faut compter de nombreuses sessions de travail, et des tests
> en jeu à chaque étape.

État actuel : 4 194 textes traduits sur 7 295 dans `po/fr.po` (dont les
entrées de genre laissées vides, qui valent masculin). Le
fonctionnement de la traduction et la façon d'ajouter des traductions sont
décrits dans la section 7 de [INSTALL.md](INSTALL.md).

## 1. Grammaire française des noms (priorité haute)

C'est ce qui bloque la plupart des messages restants.

- **Fait : noms de monstres.** Les 419 noms de `include/monsters.h` sont
  traduits (contexte `monster`) avec leur genre (contexte `gender` : `f`
  féminin, `e` élision forcée, `n` pas d'élision). `x_monnam()` compose
  l'article et place les adjectifs ; `Monnam()`, `mon_nam()`, `a_monnam()`
  et `y_monnam()` en profitent. Les combats courants sont traduits
  (« Le chacal mord ! », « Vous tuez le triton ! », « Sirius mord le
  triton. »). Restent : les prêtres et les marchands (composés à part),
  les noms hallucinés tirés de `dat/bogusmon.txt`, et les phrases qui
  utilisent le possessif `s_suffix()` (« the fox's »).
- **Accords avec le monstre** : `i18n_mon_fem(mon)` indique si le nom
  traduit est féminin, pour choisir une forme `C_("feminine", ...)` du
  message (fait pour « %s is killed! »). Ailleurs, préférer des tournures
  sans participe (« %s se change en pierre ! »).
- **Fait : noms d'objets.** Les 547 noms, descriptions et motifs de
  `include/objects.h` sont traduits (contexte `object`, avec pluriel ;
  genre dans `objgender`), ainsi que les apparences (« potion
  pétillante », « anneau en bois »). `xname()` et `doname()` composent le
  nom en français (« 4 potions de soins non maudites », « une paire de
  gants de cuir +1 non maudite (portée) »), avec élision (« parchemin
  d'identification »). `an()`, `the()` et `yname()` choisissent l'article
  selon le genre et le nombre. Les couleurs `MENUCOLOR` écrites en anglais
  s'appliquent toujours (jumeau anglais de chaque nom). Restent : les noms
  d'artefacts, le texte des fruits nommés par le joueur, `minimal_xname()`
  et `distant_name()` en partie.
- **Fait : verbes accordés à l'objet.** `otense()`, `vtense()`,
  `aobjnam()`, `yobjnam()` et `Tobjnam()` conjuguent les 92 verbes qu'on
  leur passe (contexte `objverb`, singulier et pluriel) : « Votre épée
  longue brille ». Reste à traduire une bonne partie des phrases qui les
  entourent, souvent composées de morceaux (« %s %s for a moment. »).
- **Possessif** : `s_suffix()` (« the fox's » → « du renard ») ; les
  phrases qui l'utilisent sont à reformuler.
- **Fait : contractions.** Une traduction marque « de » ou « à » devant un
  argument avec `@` (« @de %s ») : une fois le message formaté, les règles
  du catalogue (contexte `grammar`, `contractions`) donnent « du gnome »,
  « de la naine », « d'un orque », « des gnomes », « au pied ».
- **Fait : noms communs avec article.** `i18n_the()` (contexte `noun`) et
  `i18n_the_ctx()` donnent « le sol », « la glace », « l'autel » selon le
  genre (`gender`) ; utilisés pour les surfaces (`surface()`,
  `ceiling()`), les noms de pièges (contexte `trap`) et les parties du
  corps.
- **Fait : parties du corps.** `mbodypart()` renvoie le nom traduit
  (contexte `bodypart`, avec pluriel pour `makeplural()`) ;
  `mbodypart_english()` garde l'anglais pour les causes de mort.
- **Fait : héroïne.** `vpline()` prend la forme `heroine` d'un message
  quand le personnage est féminin (« Vous êtes prise dans une toile »).
  Seuls quelques messages de `trap.c` en ont une pour l'instant.
- **Accord avec l'objet sujet** : `objnam_fmt()` choisit la forme
  `feminine`, `plural` ou `feminine plural` d'un message dont le sujet est
  un objet ; `objnam_adj()` accorde un adjectif. À utiliser ailleurs que
  dans `trap.c`.
- **Conjugaison** : `vtense()` et `otense()` accordent le verbe anglais avec
  le sujet ; il faut une solution pour les verbes français.
- **Vœux** (`readobjnam()` dans `objnam.c`) : comprendre un vœu tapé en
  français (« 2 potions bénies de vitesse »), en gardant l'anglais possible.

## 2. Messages composés

Fait : `trap.c` (pièges, érosion des objets, lévitation, noyade, lave,
désamorçage), `hack.c` (déplacements, rochers, salles spéciales) et
`uhitm.c` (combat du héros, attaques spéciales), `mhitu.c` (attaques des
monstres, engloutissement, séduction), l'empoisonnement (`poisoned()`) et
les questions de fin de partie, les marchands (`shk.c` : accueil, prix,
paiement, ventes, dégâts ; types de boutique traduits avec leur genre,
« au magasin de luminaires d'Izchak »), les prières et sacrifices
(`pray.c`), les potions (`potion.c`) et l'utilisation des outils
(`apply.c` : miroir, sifflets, laisse, bougies, lampes, fouet, pierre de
touche...), les baguettes et rayons (`zap.c` : noms des rayons traduits
avec leur genre, destruction des objets), les objets utilisés par les
monstres (`muse.c`), les parchemins (`read.c` ; lueurs des
enchantements d'armure et d'arme, aussi dans `wield.c`), `do.c`
(escaliers, rochers, évier, autel, objets lâchés), `do_wear.c`
(vêtements et accessoires ; noms d'armures avec leur genre et
`i18n_an_ctx()`), le creusement (`dig.c`), le maniement des armes
(`wield.c`), le ramassage et les conteneurs (`pickup.c`), les monstres
(`mon.c`), les lancers (`dothrow.c`), les repas (`eat.c`), les
artefacts (`artifact.c`), les délais (`timeout.c` : lampes et bougies qui
s'éteignent, œufs, chutes) et les coups de pied (`dokick.c`), sauf quelques
verbes de
`u_locomotion()` et `stagger()` (« float », « slither »...) simplifiés en
français.  Les couleurs de `hcolor()` sont traduites par `hcolor_i18n()`
(accord au féminin).  Restent notamment : `engrave.c`, `music.c`,
`lock.c`, `detect.c`, `steed.c`, `teleport.c`, `invent.c`, `sit.c`,
`sounds.c`, `vault.c`, `spell.c`...
et `attrib.c` (« You feel foolish! »).

`po/msgargs.py` (lancé par `make update-po`) extrait toutes les chaînes
d'une condition passée à `pline()`, `You()`... : xgettext ne prenait que
la première (`You(c ? "a" : "b")`).

- **Messages construits avec `Sprintf`** puis affichés : ils ne sont pas
  trouvés dans le catalogue. Exemples : « Really step onto that falling
  rock trap? », « You already found a monster.  Use 'm' prefix... ».
  Il faut entourer le format de `_()` à l'endroit où il est construit.
- **Morceaux de phrase** assemblés par le code (« You feel %s. » avec un
  adjectif, « %s %s %s » ...) : réécrire en phrases complètes quand c'est
  possible.
- **Verbes de `getobj()` passés par variable** (non traduits) : tremper un
  objet dans une potion (`potion.c`), ouvrir une boîte de conserve
  (`eat.c`).
- **Descriptions d'escaliers** (`stairs_description()` dans `stairs.c`) :
  « staircase down to level 3 », « staircase up out of the dungeon ». Pour
  l'instant, la phrase entière reste en anglais.
- **Simplifications faites pour le français**, à revoir : la question
  « Shall I pick... » ne précise plus ce qui reste à choisir ; la mention
  « adrift » a disparu du message de bienvenue.

## 3. Fichiers de données (`dat/`)

Mécanisme en place : `I18N_FILE(nom)` (nhi18n.c) ouvre `nom.<langue>`
(`dat/help.fr`...) s'il est installé, sinon le fichier anglais.
`make install` copie les `dat/*.<langue>` à côté de `nhdat`.

Fait : `help` (aide détaillée), `hh` (liste des commandes), `keyhelp`
(touches), ainsi que le menu de l'aide `?` et « --More-- », « (end) »
de l'interface tty.

Fait aussi : les rumeurs, l'Oracle, les gravures et les épitaphes.
`makedefs` les compile depuis `dat/rumors-fr.tru` et `.fal`,
`dat/oracles-fr.txt`, `dat/engrave-fr.txt` et `dat/epitaph-fr.txt`
(`MAKEDEFS_LANG=fr`).  `oracles-fr.txt` commence par l'oracle spécial et
garde l'ordre de `oracles.txt` : la sauvegarde retient les positions du
fichier anglais.  Les monstres hallucinés (`bogusmon.txt`) sont traduits
dans `po/fr.po` (contexte `monster` et genre), pas par un fichier.
Restent :

| Fichier | Lignes | Contenu |
|---|---:|---|
| `data.base` | 6 528 | Encyclopédie (commande `;` puis `?`) |
| `tribute` | 9 942 | Citations des romans de Terry Pratchett (livres dans le jeu) |
| `quest.lua` | 3 087 | Textes des quêtes de chaque rôle |
| `history` | 401 | Historique du jeu |
| `opthelp` | 393 | Aide des options |
| `cmdhelp` | 226 | Description de chaque touche (format « touche<TAB>texte ») |

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
