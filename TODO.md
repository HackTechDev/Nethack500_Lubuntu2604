# Traduction française : reste à faire

> **Où en est-on ?** Les messages du jeu écrits en C sont presque tous
> traduits, ainsi que le tutoriel, les textes des quêtes et la
> documentation (section 6) et l'encyclopédie (`dat/data.base`).
> Il reste quelques textes isolés (voir le tableau ci-dessous).
> Les citations de Terry Pratchett (`dat/tribute`) sont mises de côté.

État actuel (3 octobre 2026) : 13 639 textes traduits dans `po/fr.po`,
1 887 vides. Parmi ces vides, 1 099 sont des entrées de genre
(`gender`, `objgender`) : vides, elles valent masculin, c'est voulu.
Presque tous les autres sont des formats anglais remplacés à l'exécution
par une phrase entière traduite (branche `i18n_active()`) : ils ne sont
jamais cherchés dans le catalogue. Ce qui reste vraiment non traduit est
surtout fait de messages de débogage et de formats sans texte
(`"%s%s"`) ; les entrées `adjective-position` vides valent « après le
nom », c'est voulu. Les commandes du mode magicien sont traduites, ainsi que
les diagnostics internes : `impossible()` et `panic()` affichent leur
message traduit, mais le `paniclog`, le rapport de plantage et le core
dump gardent le texte anglais pour les rapports de bugs. Seuls le vidage
des glyphes de `#wizcustom` et les mots-clés des drapeaux de niveau
(`noTport`...) restent en anglais, ce sont des identifiants.

Le fonctionnement de la traduction et la façon d'ajouter des traductions
sont décrits dans la section 7 de [INSTALL.md](INSTALL.md).

## 0. Fichiers restant à traduire

Par ordre de priorité (visibilité en jeu) :

| Fichier | Lignes | Contenu | Remarque |
|---|---:|---|---|
| `dat/quest.lua` | 3 087 | Textes des quêtes de chaque rôle (arrivée, chef, ennemi, artefact) | **Traduit** (octobre 2026) : 872 textes, msgctxt `quest` dans `po/fr.po`, extraits par `po/questtext.py` ; restent à tester en jeu les dialogues avec les chefs et ennemis |
| `dat/data.base` | 6 528 | Encyclopédie (`;` puis `?`, `/`) | Fait : `dat/data-fr.base` (clés anglaises, compilé en `dat/data.fr` par `MAKEDEFS_LANG=fr makedefs -d`) |
| `dat/tribute` | 9 942 | Citations de Terry Pratchett (livres du jeu) | Ouvert sans `I18N_FILE()` (`files.c`, `TRIBUTEFILE`). **Mis de côté** (3 octobre 2026) ; si on le reprend, à brancher d'abord |
| `dat/options` | 37 | Options de compilation (`#version`) | Fait : sous Unix, `#version` construit ce texte à l'exécution (`build_options()`, `src/mdlib.c`) ; il est traduit là, sauf la licence de Lua. Le fichier produit par `makedefs` reste en anglais |
| `dat/license` | 95 | Licence | Écarté : la licence fait foi en anglais, on ne la traduit pas |

Déjà traduits : `help`, `hh`, `keyhelp`, `cmdhelp`, `opthelp`,
`history`, `usagehlp`, `optmenu`, `wizhelp` (fichiers `dat/*.fr`),
rumeurs, oracles, gravures, épitaphes (section 3), monstres hallucinés
(`bogusmon.txt`, dans `po/fr.po`).

Restes dans le code (`po/fr.po`, environ 85 textes) : messages de
débogage de `mkmaze.c`, `display.c`, `objnam.c`, `trap.c`, `dig.c`,
`wizcmds.c`, `restore.c`, `sfstruct.c` ; quelques textes des interfaces
`win/tty` (8) et `win/curses` (4) ; les menus du mode magicien (`^V`,
`^O`...). Pour les retrouver, chercher dans `po/fr.po` les entrées vides
qui ne sont ni des genres ni des formats remplacés par une branche
`i18n_active()`.

Interfaces non extraites : `win/X11` et `win/Qt` (`po/update-pot.sh` ne
lit que `win/tty` et `win/curses`).

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
s'éteignent, œufs, chutes), les coups de pied (`dokick.c`), les
gravures (`engrave.c`), la musique (`music.c`), les serrures
(`lock.c`), la détection (`detect.c`), les montures (`steed.c`), la
téléportation (`teleport.c`), l'inventaire (`invent.c`), s'asseoir
(`sit.c`), le menu d'actions sur un objet (`iactions.c`) et les menus de
catégories et d'objets (`query_category()`, `query_objlist()`), les cris
et répliques des monstres (`sounds.c`), les gardes du coffre (`vault.c`)
et les sorts (`spell.c`, menu des sorts aligné par caractères),
l'équipement des monstres (`worn.c`), les combats entre monstres
(`mhitm.c`), leurs sorts (`mcastu.c`), les reflets (`mon_reflects()`) et
l'occupation interrompue (« Vous cessez de chercher »), la polymorphie
(`polyself.c`), les projectiles et souffles des monstres (`mthrowu.c`),
les fontaines et éviers (`fountain.c`), le reste des pièges (`trap.c`),
les déplacements des monstres (`monmove.c`), `pager.c`, le pont-levis
(`dbridge.c`), les armes et compétences (`weapon.c`), les démons
(`minion.c`), les vols (`steal.c`), les caractéristiques (`attrib.c`),
l'écriture (`write.c`), les familiers (`dog.c`, `dogmove.c`), les
explosions (`explode.c`), `mkobj.c`, le Magicien de Yendor (`wizard.c`),
les nuages (`region.c`), les prêtres (`priest.c`), le boulet (`ball.c`),
les vers longs (`worm.c`), le choix d'une position (`getpos.c`, hors
fenêtre d'aide), le courrier (`mail.c`), les lignes d'état du stéthoscope
(`insight.c`), `quest.c`, le menu simple des options `O` avec les
descriptions des options (extraites de `include/optlist.h`), les messages
de changement d'option et le ramassage automatique (`@`), la description
d'un point de la carte (`;`, `/`, description automatique du curseur,
`#lookaround` ; `pager.c` décrit le point deux fois, en anglais pour la
recherche dans l'encyclopédie `data.base`, puis traduit pour l'affichage ;
symboles en msgctxt `symbol`, `symbol a`, `symbol the`), les directions
d'accessibilité (« (4sud,6ouest) »), le génocide (pluriels des monstres
par `i18n_mon_plural()`, msgctxt `monster-plural` pour les exceptions),
les vœux, le nommage (`do_name.c`), `#lookaround`, les erreurs du fichier
de configuration, les messages de `options.c`, `files.c`, `cfgfiles.c`,
`bones.c` et des commandes du mode magicien (`wizcmds.c`), et les lignes
« deux armes » de `^X`.

Exceptions : les noms de rayons hallucinatoires (`hallublasts[]`) restent
en anglais ; quelques verbes de `u_locomotion()` et `stagger()`
(« float », « slither »...) sont simplifiés en français.  Les couleurs de
`hcolor()` sont traduites par `hcolor_i18n()` (accord au féminin).

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
(touches), `cmdhelp` (touche `&`, mêmes touches et directives `&`),
`opthelp` (aide des options), `history`, `usagehlp` (ligne de commande),
`optmenu` (menu des options) et `wizhelp` (mode magicien), ainsi que le
menu de l'aide `?` et « --More-- », « (end) » de l'interface tty.
`usagehlp` reste en anglais pour `nethack --usage` lancé hors du jeu,
avant la lecture de l'option `language`.

Fait aussi : les rumeurs, l'Oracle, les gravures et les épitaphes.
`makedefs` les compile depuis `dat/rumors-fr.tru` et `.fal`,
`dat/oracles-fr.txt`, `dat/engrave-fr.txt` et `dat/epitaph-fr.txt`
(`MAKEDEFS_LANG=fr`).  `oracles-fr.txt` commence par l'oracle spécial et
garde l'ordre de `oracles.txt` : la sauvegarde retient les positions du
fichier anglais.  Les monstres hallucinés (`bogusmon.txt`) sont traduits
dans `po/fr.po` (contexte `monster` et genre), pas par un fichier.
Les fichiers restants sont listés dans la section 0.

Les messages d'arrivée des niveaux spéciaux (`des.message()` des plans
élémentaires et du Plan astral) sont traduits par `deliver_by_pline()`
avant le remplacement de leurs codes `%` (liste pour xgettext dans
`questpgr.c`).  Le tutoriel (`tut-1.lua`, `tut-2.lua`, messages de
`nhlib.lua`) est traduit : ses textes passent par `tr(format, ...)`
(`nhlib.lua`), qui formate la traduction rendue par `nh.gettext()` ;
`po/update-pot.sh` extrait les textes donnés à `tr()` dans les fichiers
Lua.  Les textes des quêtes (`quest.lua`) sont traduits aussi.

## 4. Interface

- **Menus et fenêtres** : les textes passés à `add_menu()`, `putstr()`,
  `end_menu()` ne sont pas traduits automatiquement (menu complet des
  options `#optionsfull` (titres et descriptions traduits, noms d'options
  en anglais)). L'aide du curseur, la pierre tombale et le tableau des
  scores sont traduits à l'affichage (les causes de mort composées
  dynamiquement restent en anglais). Le menu simple `O`, les
  descriptions des commandes étendues, le résumé de fin de partie,
  `#conduct`, les exploits, `#overview` et l'illumination `^X` sont
  traduits (pour `^X`, les lignes du mode magicien `from_what()` restent
  composées de fragments, parfois en anglais).
- **Noms des commandes étendues** : à garder en anglais pour la saisie ;
  seules leurs descriptions sont à traduire.
- **Alignement des colonnes** : une lettre accentuée compte pour deux
  caractères dans les calculs de largeur (tty et curses). Des colonnes
  sont décalées d'un caractère, par exemple « Pièces  ('$') » dans
  l'inventaire. Il faudrait compter les caractères UTF-8 et non les octets.
- **Pierre tombale et tableau des scores** (`rip.c`, `topten.c`) : la cause
  de la mort (« killed by a goblin ») reste en anglais dans `logfile`,
  `xlogfile` et `record` ; elle est traduite à l'affichage quand le nom
  du tueur figure dans le catalogue (msgctxt `monster`, `killer`,
  `killer a`, suffixe « , while … » en msgctxt `while`).

## 5. Laissé volontairement en anglais

- Messages techniques internes (`impossible()`, `panic()`, traces de
  débogage).
- Fichiers `livelog`, `dumplog` et `xlogfile`, lus par des outils externes.
- Noms des options et valeurs du fichier de configuration.

## 6. Documentation

- Guide du joueur : `doc/Guidebook-fr.mn` (traduction complète de
  `doc/Guidebook.mn`), texte UTF-8 dans `doc/Guidebook-fr.txt` produit par
  `make Guidebook-fr.txt` dans `doc/` (il faut groff pour `preconv`).
- Page de manuel : `doc/nethack-fr.6`, texte dans `doc/nethack-fr.txt`
  (`make nethack-fr.txt`).
- À reporter dans les traductions quand `Guidebook.mn` ou `nethack.6`
  changent (chaque fichier traduit garde la révision de l'original).
- Pas encore traduits : `Guidebook.tex` (version LaTeX), `recover.6`,
  `dlb.6`, `makedefs.6` (outils techniques, faible priorité).
