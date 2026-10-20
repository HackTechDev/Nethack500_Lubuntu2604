# NetHack en thème cyberpunk : analyse

Comment donner au jeu un habillage cyberpunk, et ce que coûte chaque
niveau d'ambition.  Du moins coûteux au plus coûteux.

## 1. Un catalogue « cyberpunk » (le meilleur rapport effort/résultat)

Le mécanisme de traduction du jeu (`src/nhi18n.c`, `po/fr.po`) remplace à
l'affichage chaque message, nom de monstre, nom d'objet et texte d'aide.
Il suffit d'ajouter une « langue » de plus, choisie par une option, sans
toucher aux règles du jeu :

- objets : potions → stims, parchemins → puces de données, baguettes →
  implants ou gadgets, grimoires → logiciels, anneaux → modules neuronaux,
  armures → combinaisons ou blindages ;
- monstres : gnomes → squatteurs des sous-sols, orques → corporatistes, liches →
  IA corrompues, golems → drones, démons → entités du Réseau ;
- lieux et rôles, à l'affichage seulement : Donjons du Destin →
  Night City, Mines des gnomes → Bas-fonds, Sokoban → Entrepôt automatisé,
  le Magicien → Netrunner, la Valkyrie → Solo, le Samouraï → Ronin
  urbain, le Touriste → Corpo en goguette.

Avantages : le français reste disponible à côté, et la partie technique
est déjà en place.  Le travail est surtout de l'écriture : environ 15 000
entrées au total, mais on peut commencer par les noms (environ 1 400) et
les messages les plus fréquents.

## 2. Les textes d'ambiance

Il faudrait réécrire, avec notre propre plume :

- les rumeurs, l'Oracle (« la Courtière »), les gravures (graffitis) et les
  épitaphes ;
- les textes de quête de chaque rôle, l'encyclopédie et le Guide.

Beaucoup de texte, mais faisable morceau par morceau, comme la
traduction.

## 3. L'apparence

- Couleurs et symboles : un jeu de symboles (`symset`) et des couleurs
  néon dans la configuration, sans rien compiler.
- Tuiles graphiques (X11, Qt, curses) : il faudrait redessiner les images
  de `win/share/*.txt`.  C'est long, et c'est un travail de graphisme.

## 4. Changer les règles du jeu (le plus coûteux)

De vraies armes à feu avec munitions, du piratage, des cyber-implants
avec leurs effets, de nouveaux monstres : cela demande de modifier
`include/objects.h`, `include/monsters.h` et le code du combat, de
rééquilibrer tout le jeu, et cela rend les anciennes sauvegardes
incompatibles.  C'est le travail d'une variante complète, de l'ordre de
SLASH'EM ou dNetHack.

## Recommandation

Commencer par le niveau 1 avec les couleurs du niveau 3, et un premier
lot limité aux noms des objets, des monstres, des rôles et des lieux.
Jouer ensuite pour juger de l'effet avant d'aller plus loin.

## Mécanique (en place)

- **Choix du thème** : `OPTIONS=language:fr@cyber` dans la configuration,
  ou l'option `language` des options avancées en cours de partie.  La
  forme `<langue>@<thème>` suit les locales POSIX (`src/nhi18n.c` :
  thème de 1 à 8 minuscules).
- **Thème par défaut** : sans option `language`, le jeu démarre en
  `fr@cyber` (`NHI18N_DEFAULT_LANG` dans `include/unixconf.h`, à
  définir autrement ou vide avec `-DNHI18N_DEFAULT_LANG=...`) ; en
  anglais si le catalogue manque.  `language:fr` ou `language:en` dans
  la configuration pour en sortir.
- **Catalogue** : `po/themes/fr@cyber.po` ne contient que les entrées qui
  changent.  `make` le fusionne avec `po/fr.po` (`msgcat --use-first`, le
  thème d'abord) en `dat/fr@cyber.mo`, installé à côté de `fr.mo`
  (`sys/unix/Makefile.top`, variable `THEMEMO`).  Tout ce que le thème ne
  redéfinit pas reste en français.
- **Fichiers de données** : le jeu cherche `help.fr@cyber`, puis
  `help.fr`, puis `help` (`i18n_datafile()`).  On peut donc ajouter plus
  tard des fichiers `dat/<nom>.fr@cyber` (aide, rumeurs...).
- **Genres** : un objet ou un monstre renommé doit avoir son genre dans le
  thème (`objgender` pour les objets, `gender` pour les monstres).  Le
  masculin s'écrit `m` : une entrée vide serait remplacée par celle de
  `fr.po` (« potion » est féminin, « stim » ne l'est pas).
- **Contrôle** : `python3 po/check-theme.py po/themes/fr@cyber.po` signale
  les entrées absentes de `po/nethack.pot` (faute de frappe, texte disparu
  du jeu) et les renommages sans genre explicite.
- Les règles du jeu ne changent pas, ni les sauvegardes : on peut passer
  d'une partie à l'autre entre `fr`, `fr@cyber` et `en`.

## Premier lot de vocabulaire (en place)

| Anglais | Français | Cyberpunk |
|---|---|---|
| potion (de soins) | potion | stim (de soins), masculin |
| scroll | parchemin | puce (de données), féminin |
| wand | baguette | implant, masculin |
| ring | anneau | module, masculin |
| spellbook | livre de sorts | logiciel, masculin |
| gold piece | pièce d'or | eddie |
| credit card, expensive camera, food ration | carte de crédit... | puce de crédit, holocaméra, ration synthétique |
| catégories de l'inventaire | Parchemins, Potions... | Puces, Stims, Modules, Implants, Logiciels, Eddies |

Rôles (d'après ceux du jeu de rôle Cyberpunk quand l'un correspond) :
Archéologue → Techie, Barbare → Nomade, Homme/Femme des cavernes →
Sauvageon/Sauvageonne, Guérisseur → Medtech, Chevalier → Lawman, Moine → Moine
augmenté/Moniale augmentée, Prêtre/Prêtresse →
Technoprêtre/Technoprêtresse, Rôdeur → Traqueur/Traqueuse, Voleur →
Fixer/Fixeuse, Samouraï → Ronin urbain(e), Touriste → Corpo
en goguette, Valkyrie → Solo, Magicien → Netrunner/Netrunneuse.

Races : elfe → réplicant/réplicante, nain → mineur/mineuse, gnome →
squatteur/squatteuse, orque → corporatiste ; humain reste humain.

Lieux : Donjons du Destin → Night City (d'abord « la Mégalopole »), Mines
des gnomes → les Bas-fonds, Sokoban → les Entrepôts automatisés, la Quête → le Contrat,
Fort Ludios → le Coffre-fort de Ludios, Géhennom → le Réseau profond, Plans
élémentaires → les Couches du Réseau.  (Éviter les noms en « L' » : seuls
« Le », « La », « Les » passent en minuscule dans les phrases.)

Monstres : triton → rat de labo, chacal → chien errant, insecte de grille
→ bug réseau, œil flottant → caméra flottante, kobold → voyou, gnome →
squatteur, seigneur gnome → chef squatteur, nain → mineur, orque des
collines → corporatiste, orque du Mordor → corporatiste du Mordor,
Uruk-hai → gros bras, liche → IA corrompue, golem de papier → drone de
papier, golem de fer → drone de combat, diablotin → virus,
fourmi soldat → nanobot soldat,
abeille tueuse → drone tueur, marchand → vendeur, garde → vigile,
capitaine de la garde → chef des vigiles, Oracle → la Courtière, chaton,
petit chien, poney → chaton-robot, chien-robot, moto-poney.

Vérifié en jeu (Magicien elfe) : « Vous êtes Netrunner elfe chaotique »,
« une puce d'identification non maudite », « un stim de soins non
maudit », « Vous êtes dans la Mégalopole », « chaton-robot apprivoisé ».

## Deuxième lot : les rangs (en place)

Neuf rangs par rôle, au masculin et au féminin (ligne d'état, `^X`, fin
de partie).  Les rangs qui reprennent le nom du rôle gardent celui du
thème.  Le trône s'adresse au héros par « Patron » ou « Patronne »
(« Sire » et « Dame », `sit.c`).

| Rôle | Rangs (niveaux 1, 3, 6, 10, 14, 18, 22, 26, 30) |
|---|---|
| Techie | Fouineur, Ferrailleur, Fouilleur de décharges, Pilleur d'épaves, Démonteur, Spéléo urbain, Archéotechnicien, Collectionneur, Conservateur |
| Nomade | Casseur, Pilleur, Racketteur, Braqueur, Pillard, Ravageur, Tueur, Chef de gang, Seigneur de guerre |
| Sauvageon | Troglo, Rat de tunnel, Errant, Clodo, Routard, Rôdeur des friches, Bourlingueur, Écumeur, Pionnier |
| Medtech | Herboriste de rue, Rafistoleur, Embaumeur, Infirmier, Rebouteux, Chimiste de rue, Doc, Médecin de l'ombre, Chirurgien cybernétique |
| Lawman | Recrue, Vigile, Agent, Sergent, Lawman, Chef d'escouade, Chevalier corpo, Patron, Paladin corpo |
| Moine augmenté | Candidat, Novice, Initié, Élève du Silicium, du Flux, du Chrome, des Ondes, du Plasma, Maître |
| Technoprêtre | Aspirant, Acolyte, Adepte, Technoprêtre, Desservant du Code, Chanoine du Réseau, Gourou, Patriarche du Réseau, Grand Technoprêtre |
| Traqueur | Bleu, Guetteur, Éclaireur, Ombre, Pisteur, Arbalétrier, Tireur, Tireur d'élite, Sniper |
| Fixer | Pickpocket, Coupe-bourse, Fixer, Chapardeur, Braqueur, Cambrioleur, Escroc, Arnaqueur, Maître voleur |
| Ronin urbain | Recrue du clan, Ronin des rues, Ninja/Kunoichi, Garde du corps, Chef de quartier, Chef de district, Oyabun, Conseiller zaibatsu, Shogun corpo |
| Corpo en goguette | Badaud, Visiteur, Excursionniste, Globe-trotteur, Voyageur, Baroudeur, Grand voyageur, Explorateur, Aventurier |
| Solo | Gamin, Escarmoucheur, Combattant, Homme d'armes, Guerrier, Bretteur, Héros, Champion, Seigneur des rues |
| Netrunner | Script-kiddie, Bidouilleur, Codeur, Hacker, Cracker, Sorcier du code, Nécromancien des données, Netrunner, Légende du Réseau |

Vérifié en jeu : « le Script-kiddie », puis « le Hacker » au niveau 10
(Netrunner) ; « la Gamine », puis « la Femme d'armes » (Valkyrie).

## Troisième lot : descriptions des classes (en place)

Ce que montrent `;` et `/` (msgctxt `symbol` sans article, `symbol a`
avec).  Seules changent les classes liées aux renommages et quelques
classes à forte couleur ; les autres restent en français.

- Monstres : nanobot ou insecte, blob de déchets, caméra ou sphère, virus
  ou démon mineur, gelée chimique, voyou, corporatiste, vortex de données,
  hologramme, IA angélique, drone volant ou oiseau, squatteur, monstre
  furtif, flic de choc, IA corrompue, cyborg momifié, pouding ou boue
  toxique, spectre numérique, fantôme du Réseau, drone ou golem, entité du
  Réseau.
- Objets : combinaison ou pièce de blindage, module, stim, puce,
  logiciel, implant, tas de crédits.

Vérifié en jeu avec `/` : « un corporatiste », « un squatteur », « un stim »,
« une puce », « une entité du Réseau », « un drone ou golem ».

## Quatrième lot : le reste des monstres et des objets (en place)

Une sélection : 133 monstres et 67 objets de plus ; les animaux, dragons,
géants, démons nommés, chefs de quête et armes d'hast restent en
français.  `i18n_mon_plural()` met chaque mot au pluriel ; les exceptions
sont dans le msgctxt `monster-plural` (« chevaux de Troie », « IA
corrompues »), que `check-theme.py` accepte.

- Monstres : chien-robot de garde, molosse-robot, chat-robot,
  moto-cheval ; mineurs (chef mineur, contremaître, contremaîtresse),
  squatteurs (caïd, caïde, parrain, bidouilleur squatteur), voyous et
  gangs (petite frappe, gros dur, membre de gang, capitaine de gang) ;
  pirate neuronal, ver informatique, cheval de Troie, racketteur, leurres,
  voleuses des parcs, des quais, des toits ; drones (chauve-souris,
  corbeau, cryogénique, incendiaire, de paille, de verre, biodrone),
  mine gazeuse, alarme ambulante, hologrammes aveuglant et psychédélique,
  tempête de données, nuage de smog ; flics de choc, IA démente, maîtresse,
  suprême, momies et zombies des nouvelles races ; technoprêtres,
  soldats corpo, infirmière-androïde, Archihacker de Yendor, démon de
  messagerie ; rôles joués par les monstres et gardiens des quêtes
  (stagiaire, sensei, apprenti hacker...).
- Objets : poignard, cutter, cran d'arrêt, machette, monokatana, hache
  d'incendie, matraque, bâton télescopique, arc à poulies, perceuse,
  marteau-piqueur ; blindage lourd, maille de kevlar, blouson de cuir,
  trenchs (de protection, à camouflage optique, à blindage EM,
  holographique), bouclier anti-émeute, gantelets hydrauliques, bottes à
  servomoteurs, à ressorts, antigravité ; sac hyperspatial, frigo
  portable, lampe torche, holosphère, lentilles augmentées, scanner
  médical, graveur de puces, canon cryogénique, lance-flammes, barre
  protéinée, gaufrette énergétique.

Vérifié en jeu : « 2 poignards », « un trench à blindage EM », « un
graveur de puces », « Le cheval de Troie boit un stim bleu ciel ! », « La
contremaîtresse manie une épée courte large ! », « Le flic de choc
frappe ! ».

## Cinquième lot : messages et monnaie (en place)

- Les 113 messages qui nommaient potions, parchemins, baguettes, anneaux,
  livres de sorts ou pièces d'or parlent de stims, puces, implants,
  modules, logiciels et crédits (« Pendant que vous lisez la puce, elle
  grille. », « Le stim explose ! », « Vous ne portez aucun module. »),
  avec l'accord en genre (« Cet %s est un implant de creusement ! »), y
  compris les causes de mort (« un stim d'acide ») et l'aide des
  commandes.  Le mot « magie » reste : le Réseau a sa magie.
- La monnaie : `currency()` (invent.c) traduit son résultat (msgctxt
  `currency`).  En français il reste « zorkmid », le thème dit « crédit »
  : « Votre bourse contient 672 crédits. », prix des boutiques, Oracle...
- Le fruit par défaut « slime mold » est traduit quand le joueur n'a pas
  choisi de nom (objnam.c) : « moisissure visqueuse », « pâte d'algues »
  dans le thème.

## Sixième lot : les textes d'ambiance (en place)

Écrits pour le thème (pas traduits du français), compilés par `makedefs`
avec `MAKEDEFS_LANG=fr@cyber` (`set_datlang()` accepte maintenant le
« @ ») en `dat/<nom>.fr@cyber`, installés à côté de `fr@cyber.mo` :

- `dat/rumors-fr@cyber.tru` et `.fal` : 113 vraies rumeurs (des conseils
  exacts, avec le vocabulaire du thème) et 107 fausses (publicités,
  slogans des corpos, folklore du Réseau) ;
- `dat/oracles-fr@cyber.txt` : les 21 consultations de la Fixeuse, dans
  l'ordre de `oracles.txt` (le contenu de chaque conseil est gardé) ;
- `dat/engrave-fr@cyber.txt` : 53 graffitis (tags de gangs, mots de
  passe, slogans ; « Elbereth » reste) ;
- `dat/epitaph-fr@cyber.txt` : 98 épitaphes (« Déconnecté », « Garantie
  expirée », « Téléchargée vers un monde meilleur »...).

Les messages de l'Oracle disent « la Fixeuse » (« La Fixeuse consulte ses
écrans un instant, puis déclare : »), aussi dans l'aperçu du donjon et
les exploits.

Vérifié en jeu : `#wizrumorcheck` (rumeurs, gravures, épitaphes),
consultations mineure et majeure de la Fixeuse.

## Septième lot : les quêtes, textes communs et Netrunner (en place)

Les 931 textes de quête (msgctxt `quest`) se réécrivent rôle par rôle avec
`po/quest-theme.py` : `dump <rôle>` les liste avec leur traduction
française, numérotés ; `gen <rôle> <fichier>` produit les entrées du
thème à partir des textes réécrits, et signale les codes `%` qui ne
correspondent pas.  Un texte partagé par plusieurs rôles n'a qu'une
traduction : il doit rester neutre.

- Textes communs : jurons des anges (« Ta licence de vie est révoquée par
  %D ! ») et des démons, bannissement (« Votre contrat est résilié. »),
  légende du début (« Il est écrit dans le Code source de %d... Moloch
  déroba à Marduk le Programmeur... »), message du portail (« Vous recevez
  un message chiffré, très brouillé... »).
- Netrunner (Magicien) : Néferet la Verte, mentore, dans son data center
  isolé (« la Tour solitaire ») ; le Ténébreux dans la Tour Obsidienne ;
  apprentis hackers, pare-feu, botnet, Archihacker de Yendor.

Vérifié en jeu : arrivée (« C'est le data center de votre ancienne
mentore, Néferet la Verte. »), retour (« Vous voici de nouveau au data
center isolé. »), répliques des apprentis (« Les attaques du Ténébreux
étaient trop puissantes pour nos pare-feu. »).  Le dialogue avec la
mentore n'a été contrôlé que dans le catalogue.

## Huitième lot : quêtes du Récupérateur et du Cogneur (en place)

Seuls les textes qui changent sont dans le thème (94 entrées) ; ceux
que partage déjà la quête du Netrunner gardent sa version.

- Récupérateur (Archéologue) : le Campus d'archéotechnique, menacé par un
  plan social de ses actionnaires ; le Bunker des rois toltèques derrière
  une friche industrielle ; stagiaires, cyborgs momifiés.
- Cogneur (Barbare) : le Squat de la tribu Duali, la Station de pompage
  des Duali ; Thoth Amon en techno-sorcier, son convoi blindé touranien,
  le Codex de Skelos ; la rue, le gang, les bastons sur le bitume.

Vérifié en jeu : arrivée dans la quête du Récupérateur (« ou que le
Campus d'archéotechnique a été assiégé ») ; le reste dans le catalogue.

## Neuvième lot : quêtes du Sauvageon et du Charcudoc (en place)

73 entrées (textes changés et noms de lieux).

- Sauvageon (Homme des cavernes) : les Tunnels des Ancêtres, vieilles
  rames de métro éclairées de braseros ; le Collecteur du Dragon, au
  bout d'égouts géants ; pluies acides, sortie vers la surface coupée.
- Charcudoc (Guérisseur) : la Clinique d'Épidaure et ses cuves de
  culture, le Laboratoire de Coéos au-delà des marais toxiques ; le
  cyclope répand un virus, nanobots-sangsues, vaccin, rejet de greffe.
  Les noms grecs (Chiron, Hadès, Zeus) restent.

Vérifié en jeu : arrivée dans la quête du Charcudoc (« Quel bug vous a
ramené à la Clinique d'Épidaure ? ») ; le reste dans le catalogue.

## Dixième lot : quêtes de l'Exécuteur et du Moine augmenté (en place)

- Exécuteur (Chevalier) : la Tour Camelot, siège d'une corporation dont
  le roi Arthur préside le conseil (le « Nous » est gardé) ; Merlin
  devient l'IA de sécurité qu'il faut remettre en ligne ; la Plateforme
  de Verre au-delà des marais toxiques ; écuyers corpo, stock-options.
- Moine augmenté : le Dojo de Chan-Sune, la Forteresse du Seigneur de la
  Terre ; méditation → synchronisation, illumination → mise à jour
  suprême ; Maître Kaen a sauvegardé son esprit ailleurs.

Vérifié en jeu : arrivée dans la quête de l'Exécuteur (« Vous vous
matérialisez dans l'ombre de la Tour Camelot. ») ; le reste dans le
catalogue.

## Onzième lot : quêtes du Technoprêtre et du Traqueur (en place)

- Technoprêtre (Prêtre) : le Grand Temple du Code et son autel-serveur
  corrompu, le Sanctuaire de Nalzok derrière une décharge de serveurs
  morts ; Grandes Compilations, Moloch qui tonne dans les haut-parleurs.
  Beaucoup de répliques sont communes avec le Moine augmenté (même texte
  anglais) : elles gardent la version du lot 10, rendue neutre
  (« Quittez ce sanctuaire »).
- Traqueur (Rôdeur) : le Poste avancé d'Orion dans le dernier parc
  sauvage de Night City, défendu par des champs de force ; le
  Labyrinthe du wumpus, « comme dans un très vieux jeu vidéo » ; le
  bunker de Scorpius ; l'arc qui imprime lui-même ses flèches.

Vérifié en jeu : arrivée dans la quête du Traqueur (« le dernier parc
sauvage de Night City ») ; le reste dans le catalogue.

## Douzième lot : quêtes du Voleur de données et du Ronin urbain (en place)

- Voleur de données (Voleur) : Ransmannsby devient un quartier, le code
  de la guilde se tape sur un terminal de poignet ; le QG de la Guilde
  des Voleurs et celui de la Guilde des Assassins (le premier sert aussi
  à la quête du Corpo en goguette) ; crédits, puces de données, passe
  électronique, virus qui change les puces en ferraille ; la clé ouvre
  « toutes les serrures et tous les pare-feu de Night City ».
- Ronin urbain (Samouraï) : le Siège du clan Taro et la Tour du Shogun ;
  le daimyo devient directeur, l'empereur le Président du zaibatsu ;
  yakuzas des bas quartiers, monokatana, hologramme du teki, saké de
  synthèse ; les mots japonais restent.

Vérifié en jeu : arrivée dans la quête du Ronin urbain (« le kami du
Siège du clan Taro ») ; le reste dans le catalogue.

## Treizième lot : quêtes du Corpo en goguette et de la Mercenaire (en place)

- Corpo en goguette (Touriste) : Ankh-Morpork garde son nom, ses néons
  éteints et ses aérotaxis cloués au sol ; le maire financé par les
  corpos s'enfuit avec la caisse municipale vers le QG de la Guilde des
  Voleurs (nom du lot 12) ; crédits, holocartes postales des Bas-fonds,
  hôtel-capsule, visa annulé, code PIN de la carte.
- Mercenaire (Valkyrie) : le Relais du Destin, hérissé d'antennes, sur
  une colline de neige artificielle ; la Fonderie de Surt, haut fourneau
  entouré de lave ; la Norne calcule l'avenir au lieu de le lire, l'orbe
  devient un flux de simulations ; solde, contrats, Valhalla gardé.

Vérifié en jeu : arrivées dans les deux quêtes (« le décor familier
d'Ankh-Morpork », « le Relais du Destin, hérissé d'antennes ») ; le
reste dans le catalogue.

## Quatorzième lot : configuration aux couleurs néon (en place)

`doc/cyberpunk.nethackrc` est un exemple de configuration, à copier dans
`~/.nethackrc` ou à lancer avec `nethack -nethackrc=doc/cyberpunk.nethackrc` :

- `language:fr@cyber`, `symset:Enhanced1` ;
- carte : murs en cyan néon, sol violet, couloirs gris acier, portes rose
  vif, escaliers jaunes, autels magenta, lave orange, pièges rose vif
  (`glyph:S_<symbole>:U+nnnn/R-G-B`) ;
- menus (`MENUCOLOR`) : béni, maudit, porté, en main... Les motifs se
  comparent au nom anglais des objets, même en jeu traduit ;
- ligne d'état (`hilite_status`) : points de vie du vert acide au rouge
  clignotant, énergie en cyan, crédits, rang en magenta, faim, états.

Les couleurs à deux mots s'écrivent avec un tiret (`bright-green`) :
avec une espace, `hilite_status` les refuse.

Vérifié en jeu : la configuration se charge sans erreur ; murs et sol
sortent en cyan et violet (palette 256 couleurs du terminal).

## Prochains lots

Tous les lots prévus sont faits ; les pistes pour plus tard sont dans
[CYBERPUNK_lot.md](CYBERPUNK_lot.md).

## Jargon du jeu de rôle Cyberpunk (en place)

Les noms et termes génériques du jeu de rôle (ses noms propres sont venus
ensuite, voir « Night City » ci-dessous) :

- rôles : Techie, Nomade, Medtech, Lawman, Fixer/Fixeuse, Solo (voir
  plus haut) ; le rang « Nomade » des Sauvageons devient « Bourlingueur »
  pour ne pas faire doublon, et l'Oracle « la Fixeuse » devient « la
  Courtière » ;
- la monnaie : eddies (« Votre bourse contient 978 eddies. ») ; le
  crédit des boutiques et le crédit au sens figuré restent des crédits ;
- la lycanthropie devient la cyberpsychose : rat-garou, chacal-garou,
  loup-garou → cyberpsycho des égouts, des ruelles, alpha ; « Votre chrome
  grésille : la cyberpsychose vous gagne. » ;
- l'argot (chrome, choom, preem, gonk, edgerunner, flatliné) dans de
  nouvelles rumeurs, épitaphes et gravures.

Les sections précédentes gardent les anciens noms des rôles.

## Night City

Le jeu se déroule à Night City, la ville du jeu de rôle Cyberpunk : la
« Mégalopole » anonyme des premiers lots devient Night City (nom du
donjon, textes de quête, rumeurs, gravures, Fixeuse et prêtres), et les
noms propres du jeu de rôle peuvent servir, sans en copier les textes
publiés :

- les quartiers (Watson, Westbrook, Heywood, Pacifica, Santo Domingo,
  City Center) et les Badlands autour ;
- les corporations (Arasaka, Militech, Biotechnica...), auxquelles
  peuvent appartenir les corporatistes et le Corpo en goguette ;
- les gangs (Maelstrom, Tyger Claws, Valentinos, 6th Street, Voodoo
  Boys...) ;
- les forces de l'ordre (NCPD, MaxTac) et Trauma Team.

« Night City » n'a pas d'article : « à Night City », « de Night City »
(pas de contraction), « dans les rues de Night City ».  Les Bas-fonds
gardent leur nom générique.

En place (2026-10-20) : les flics de choc sont ceux du NCPD, les soldats
corpo ceux de Militech ; 11 rumeurs vraies (de vrais conseils situés dans
les quartiers ou chez les gangs), 12 fausses, 12 épitaphes et 12
graffitis citent les quartiers, les corporations et les gangs.  Les
elfes sont des réplicants, comme la race du héros, et les objets des
races renommées sont « de mineur », « de réplicant » et « corpo ».  Les
messages sur l'or parlent d'eddies.
