# Ce qui reste à traduire (état du 2026-10-19)

Bilan après les commits `b383d77` (races), `29aac44` (orques →
corporatistes) et `78fff26` (rôles et jargon du jeu de rôle Cyberpunk),
poussés sur la branche `cybernethack`.

Décision prise depuis : le jeu se déroule à **Night City**, la ville du
jeu de rôle Cyberpunk.  La « Mégalopole » anonyme du thème devient Night
City, et les noms propres du jeu de rôle (quartiers, corporations, gangs),
écartés jusqu'ici, peuvent désormais servir (voir 2 a).

## 1. Le français normal (`po/fr.po`) : presque complet

`msgfmt --statistics po/fr.po` : 15 018 messages traduits, 1 873 non
traduits.  Presque tous les « non traduits » le sont exprès :

| Entrées vides | Nombre | Pourquoi |
|---|---|---|
| msgctxt `gender` | 809 | genre masculin par défaut (vide = masculin) |
| msgctxt `objgender` | 287 | idem pour les objets (vérifié avec `po/check-objgender.py`) |
| msgctxt `feminine`, `monster`, `adjective-position`... | 57 | forme identique au masculin, ou inutile |
| sans contexte | 722 | voir ci-dessous |

Sur les 722 sans contexte, environ 620 sont des formats anglais qui ne
servent jamais quand le jeu est traduit (branche `i18n_active()` à côté,
bloc `#if 0`, `impossible()`), et une cinquantaine sont des formats sans
texte (`"%s %s"`, `"%s."`, `"]"`).  Il reste une dizaine de vrais trous :

- `src/mcastu.c:729` : « You hear someone summoning something, and %s %s. »
- `src/uhitm.c:3151` : « Some writing vanishes from %s head! »
- `src/trap.c:7591` : « %s into flame! » (à vérifier : peut-être un
  fragment déjà couvert par une branche traduite)
- `win/tty/getline.c:320`, `win/curses/cursdial.c:515` : « unknown
  extended command »
- `win/tty/wintty.c:2443`, `2484`, `win/curses/cursmisc.c:673` :
  « Cannot open ... »
- `win/tty/wintty.c:2965` : terminal trop petit pour `perm_invent`

Restes notés lors des sessions précédentes :

- la question « What do you want to name this lance? » (nommer un objet)
  n'est pas traduite ;
- « de Hermes » dans un texte de quête pour le Guérisseur neutre ;
- la série de vérifications des quêtes en jeu a été interrompue (Arc et
  Bar : textes vus seulement en jeu).

Volontairement laissés de côté : `dat/tribute` (mis de côté par
l'utilisateur), `dat/license` (ne jamais traduire).

## 2. Le thème cyberpunk (`po/themes/fr@cyber.po`) : les trous visibles

Le thème ne contient que les entrées qui changent (1 352) ; tout le reste
vient de `po/fr.po`.  Des messages français parlent donc encore de ce que
le thème a renommé.

### a) Night City (priorité 1)

Le niveau principal (Donjons du Destin) s'appelle aujourd'hui « La
Mégalopole » : il devient « Night City ».  Occurrences à reprendre :

- `po/themes/fr@cyber.po` : 15 (nom du donjon ligne 112, textes de quête,
  messages de l'Oracle et des prêtres) ;
- `dat/rumors-fr@cyber.fal` : 4 ; `dat/engrave-fr@cyber.txt` : 2 (dont
  l'en-tête) ; `dat/oracles-fr@cyber.txt` : 1 (« Tout au fond de la
  Mégalopole ») ; à revérifier avec `grep -i mégalopole`.

Points de grammaire : « Night City » n'a pas d'article, donc « dans la
Mégalopole » → « à Night City » (ou « dans Night City » pour les rues),
« de la Mégalopole » → « de Night City ».  Le nom du donjon est inséré
par des formats de `fr.po` (aperçu `#overview`, accomplissements,
« Vous entrez dans %s ») qui supposent un article : les relire tous et
les reprendre dans le thème si « dans Night City » sonne mal.  Vérifier
aussi que `i18n_contract()` laisse « de Night City » intact.

Ce que la ville permet d'ajouter (thème seulement, sans copier les textes
publiés du jeu de rôle) :

- les quartiers (Watson, Westbrook, Heywood, Pacifica, Santo Domingo,
  City Center) et les Badlands autour : décor des rumeurs, épitaphes,
  gravures, textes de quête ; les Bas-fonds peuvent rester un nom
  générique ou être situés dans un quartier (à décider) ;
- les corporations (Arasaka, Militech, Biotechnica...) : les
  corporatistes (ex-orques) et le Corpo en goguette peuvent en dépendre ;
- les gangs (Maelstrom, Tyger Claws, Valentinos, 6th Street, Voodoo
  Boys...) : voyous, cyberpsychos, nouvelles gravures ;
- les forces de l'ordre (NCPD, MaxTac) pour les Keystone Kops et le
  Lawman ; Trauma Team pour le Medtech.

Mettre à jour `CYBERPUNK.md` en conséquence : sa section « Jargon du jeu
de rôle Cyberpunk » dit encore « sans ses noms propres (villes,
corporations, gangs) », et la liste des lieux donne « la Mégalopole ».

### b) L'or (priorité 1)

Environ 90 messages de `fr.po` disent « or » ou « pièces d'or » alors que
la bourse affiche des eddies : « %s ramasse l'or », « %s attrape l'or »,
marchands (`shk.c`), gardes du coffre-fort (`vault.c`), prêtres
(`priest.c`), Croesus, pièces lancées, fontaines...  À reprendre dans le
thème avec « eddies » (« ramasse les eddies »).

Pour retrouver la liste : entrées de `fr.po` dont le msgstr contient
`pièces? d'or|\bor\b|zorkmid`, absentes du thème, hors `gender` et
`objgender`.

### c) Les Mines (priorité 1)

Le thème appelle les Mines des gnomes « les Bas-fonds », mais 6 entrées
gardent l'ancien nom : accomplissements (« entered the Gnomish Mines »,
« completed the Gnomish Mines »), « You have entered the Gnomish Mines. »,
`#overview`.

### d) Monstres pas encore renommés (priorité 2)

Elfes (le thème garde « elfe » pour les monstres, alors que la race du
héros est « réplicant ») : Elfe sylvain, Elfe vert, Elfe gris, Haut-elfe,
noble/seigneur/dame elfe, Roi/Reine/monarque elfe, zombie elfe, elfe.
Décider : réplicants partout, ou garder les elfes comme peuple distinct.

Autres : gnome mannequin, orque martyr, orque mortier, Blork l'orque,
gnome lauréat, lapin-garou, moustache-garou, ornithorynque-garou (en
partie des noms de monstres hallucinés ou rares) ; Orcus et orcus
cosmicus peuvent rester.

### e) Objets des races (priorité 2)

25 objets portent encore l'adjectif de l'ancienne race :

- nains (5) : cape naine, heaume de fer nain, bouclier rond nain, épée
  courte naine, lance naine → « de mineur » ;
- elfiques (10) : flèche, bottes, arc, épée large, cape, dague, heaume de
  cuir, bouclier, épée courte, lance → « de réplicant » ou un autre mot ;
- orques (10) : flèche, arc, cotte de mailles, cape, dague, heaume,
  armure d'anneaux, bouclier, épée courte, lance → « de corporatiste »,
  « corpo ».

Penser aux genres (`objgender`) et aux formes plurielles.

### f) Divers

- Le thème ne renomme qu'une partie des 3 000 messages ; il faut des
  parties de test pour repérer le reste (comme pour les quêtes).
- Les sections historiques de `CYBERPUNK.md` gardent les anciens noms
  des rôles (Récupérateur, Cogneur...) : à laisser, ou à mettre à jour.
- Messages de cyberpsychose (`You feel feverish.`, `You feel purified.`)
  et nouvelles rumeurs : pas encore vus en jeu.

## 3. Ordre proposé pour la suite

1. Mégalopole → Night City (thème, rumeurs, gravures, Oracle) et mise à
   jour de `CYBERPUNK.md`.
2. L'or → eddies et les Mines → Bas-fonds dans le thème.
3. Les noms propres de Night City (quartiers, corporations, gangs) dans
   les rumeurs, épitaphes, gravures et monstres.
4. Monstres et objets des races (choisir d'abord le sort des elfes).
5. La dizaine de vrais trous de `fr.po`, la lance à nommer, « de Hermes ».
6. Parties de test dans le thème pour trouver le reste.

## 4. Les mêmes tâches, de la plus simple à la plus difficile

Fait le 2026-10-20 : points 1 à 5 (commits `d0c2b5e`, `ff8042a`,
`a975e8a`, `2fed821`, puis celui des trous de `fr.po`).  Pour le point 5,
seuls 4 des « trous » servaient vraiment (feu de lave sur un objet porté,
commande étendue inconnue, fichiers introuvables, terminal trop petit
pour `perm_invent`) ; l'invocation entendue et l'écriture du golem
d'argile avaient déjà leur branche traduite.

Puis points 6 à 10 : questions pour nommer (`f6a7929`, avec l'élision
« d'Athéna », « d'Hermes » dans les quêtes), or → eddies (`d754bee`),
objets des races (`b59f8d5`), elfes → réplicants (`750970b`), noms
propres de Night City (NCPD, Militech, nouvelles rumeurs, épitaphes et
graffitis).  Reste le point 11 : les vérifications en jeu.

Puis (2026-10-20) : la magie devient du code (`50e4164` : sorts →
programmes, magie → piratage, 143 entrées), aides `help`, `hh`,
`cmdhelp`, `opthelp`, `usagehlp`, `wizhelp` en version `.fr@cyber` ;
l'historique de NetHack reste tel quel et l'encyclopédie (`data.fr`,
plus de 7 000 lignes) n'a pas de version cyberpunk.  Des 448 textes de
quête laissés au français, un seul jurait (résumé de l'antre de
l'Exécuteur, `1ff6350`).

Puis : derniers restes de magie (`a42f2b8`), noms des dieux traduits avec
accord des déesses (`483cafe`, « La Dame », « Tu m'as mise en colère »),
artefacts traduits en français et renommés dans le thème (Fléau-des-Hackers,
Antivirus, Scanner de Détection, Optiques du Surmonde...).

1. Mines → Bas-fonds : 6 entrées à reprendre dans le thème.
2. Mégalopole → Night City dans les fichiers de données : 7 occurrences
   (4 rumeurs, 2 gravures, 1 oracle), texte libre sans format.
3. Mettre à jour `CYBERPUNK.md` : lieux, « sans ses noms propres »,
   éventuellement les anciens noms des rôles.
4. Mégalopole → Night City dans `po/themes/fr@cyber.po` : 15 entrées,
   plus la relecture des formats de `fr.po` qui supposent un article.
5. La dizaine de vrais trous de `fr.po` : vérifier d'abord dans le code
   qu'ils servent vraiment.
6. « What do you want to name this lance? » et « de Hermes » : trouver
   d'abord d'où vient chaque texte.
7. Or → eddies : environ 90 messages ; mécanique mais long, l'accord
   change (« l'or » → « les eddies »).
8. Objets des races : 25 noms à choisir, avec genres (`objgender`) et
   pluriels.
9. Monstres pas encore renommés : décider d'abord du sort des elfes, puis
   une quinzaine de noms.
10. Noms propres de Night City (quartiers, corporations, gangs, NCPD)
    dans de nouvelles rumeurs, épitaphes, gravures et noms de monstres :
    textes à inventer ; situer ou non les Bas-fonds dans un quartier.
11. Vérifications en jeu : cyberpsychose, nouvelles rumeurs, quêtes Arc et
    Bar, puis parties de test pour trouver le reste des messages.

## Rappels pratiques

- Vérifier le thème : `python3 po/check-theme.py po/themes/fr@cyber.po`.
- Compiler et installer : `./install.sh` (construit toutes les
  interfaces ; un `make all` sans les options d'interfaces échoue à
  l'édition des liens).  Il remplace `~/.nethackrc` par
  `config/nethackrc` : comparer les deux avant.
- Pour un simple changement de traduction : `make all` puis copier
  `dat/*.fr@cyber` et `dat/fr@cyber.mo` dans
  `~/nh/install/games/lib/nethackdir/`.
- Test rapide : nom de joueur `x-Rog-Hum-Fem` (rôle, race, sexe) avec
  `OPTIONS=language:fr@cyber`.
