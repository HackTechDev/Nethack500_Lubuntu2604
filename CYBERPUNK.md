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
- monstres : gnomes → squatteurs des sous-sols, orques → gangers, liches →
  IA corrompues, golems → drones, démons → entités du Réseau ;
- lieux et rôles, à l'affichage seulement : Donjons du Destin →
  Mégalopole, Mines des gnomes → Bas-fonds, Sokoban → Entrepôt automatisé,
  le Magicien → Netrunner, la Valkyrie → Mercenaire, le Samouraï → Ronin
  urbain, le Touriste → Corpo en goguette.

Avantages : le français reste disponible à côté, et la partie technique
est déjà en place.  Le travail est surtout de l'écriture : environ 15 000
entrées au total, mais on peut commencer par les noms (environ 1 400) et
les messages les plus fréquents.

## 2. Les textes d'ambiance

Il faudrait réécrire, avec notre propre plume :

- les rumeurs, l'Oracle (« la Fixeuse »), les gravures (graffitis) et les
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
| gold piece | pièce d'or | crédit |
| credit card, expensive camera, food ration | carte de crédit... | puce de crédit, holocaméra, ration synthétique |
| catégories de l'inventaire | Parchemins, Potions... | Puces, Stims, Modules, Implants, Logiciels, Crédits |

Rôles : Archéologue → Récupérateur/Récupératrice, Barbare →
Cogneur/Cogneuse, Homme/Femme des cavernes → Sauvageon/Sauvageonne,
Guérisseur → Charcudoc, Chevalier → Exécuteur/Exécutrice, Moine → Moine
augmenté/Moniale augmentée, Prêtre/Prêtresse →
Technoprêtre/Technoprêtresse, Rôdeur → Traqueur/Traqueuse, Voleur →
Voleur/Voleuse de données, Samouraï → Ronin urbain(e), Touriste → Corpo
en goguette, Valkyrie → Mercenaire, Magicien → Netrunner/Netrunneuse.

Lieux : Donjons du Destin → la Mégalopole, Mines des gnomes → les
Bas-fonds, Sokoban → les Entrepôts automatisés, la Quête → le Contrat,
Fort Ludios → le Coffre-fort de Ludios, Géhennom → le Réseau profond, Plans
élémentaires → les Couches du Réseau.  (Éviter les noms en « L' » : seuls
« Le », « La », « Les » passent en minuscule dans les phrases.)

Monstres : triton → rat de labo, chacal → chien errant, insecte de grille
→ bug réseau, œil flottant → caméra flottante, kobold → voyou, gnome →
squatteur, seigneur gnome → chef squatteur, nain → mineur, orque des
collines → ganger, orque du Mordor → ganger du Mordor, Uruk-hai → gros
bras, liche → IA corrompue, golem de papier → drone de papier, golem de
fer → drone de combat, diablotin → virus, fourmi soldat → nanobot soldat,
abeille tueuse → drone tueur, marchand → vendeur, garde → vigile,
capitaine de la garde → chef des vigiles, Oracle → la Fixeuse, chaton,
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
| Récupérateur | Fouineur, Ferrailleur, Fouilleur de décharges, Pilleur d'épaves, Démonteur, Spéléo urbain, Archéotechnicien, Collectionneur, Conservateur |
| Cogneur | Casseur, Pilleur, Racketteur, Braqueur, Pillard, Ravageur, Tueur, Chef de gang, Seigneur de guerre |
| Sauvageon | Troglo, Rat de tunnel, Errant, Clodo, Routard, Rôdeur des friches, Nomade, Écumeur, Pionnier |
| Charcudoc | Herboriste de rue, Rafistoleur, Embaumeur, Infirmier, Rebouteux, Chimiste de rue, Doc, Médecin de l'ombre, Chirurgien cybernétique |
| Exécuteur | Recrue, Vigile, Agent, Sergent, Exécuteur, Chef d'escouade, Chevalier corpo, Patron, Paladin corpo |
| Moine augmenté | Candidat, Novice, Initié, Élève du Silicium, du Flux, du Chrome, des Ondes, du Plasma, Maître |
| Technoprêtre | Aspirant, Acolyte, Adepte, Technoprêtre, Desservant du Code, Chanoine du Réseau, Gourou, Patriarche du Réseau, Grand Technoprêtre |
| Traqueur | Bleu, Guetteur, Éclaireur, Ombre, Pisteur, Arbalétrier, Tireur, Tireur d'élite, Sniper |
| Voleur de données | Pickpocket, Coupe-bourse, Voleur de données, Chapardeur, Braqueur, Cambrioleur, Escroc, Arnaqueur, Maître voleur |
| Ronin urbain | Recrue du clan, Ronin des rues, Ninja/Kunoichi, Garde du corps, Chef de quartier, Chef de district, Oyabun, Conseiller zaibatsu, Shogun corpo |
| Corpo en goguette | Badaud, Visiteur, Excursionniste, Globe-trotteur, Voyageur, Baroudeur, Grand voyageur, Explorateur, Aventurier |
| Mercenaire | Gamin, Escarmoucheur, Combattant, Homme d'armes, Guerrier, Bretteur, Héros, Champion, Seigneur des rues |
| Netrunner | Script-kiddie, Bidouilleur, Codeur, Hacker, Cracker, Sorcier du code, Nécromancien des données, Netrunner, Légende du Réseau |

Vérifié en jeu : « le Script-kiddie », puis « le Hacker » au niveau 10
(Netrunner) ; « la Gamine », puis « la Femme d'armes » (Mercenaire).

## Troisième lot : descriptions des classes (en place)

Ce que montrent `;` et `/` (msgctxt `symbol` sans article, `symbol a`
avec).  Seules changent les classes liées aux renommages et quelques
classes à forte couleur ; les autres restent en français.

- Monstres : nanobot ou insecte, blob de déchets, caméra ou sphère, virus
  ou démon mineur, gelée chimique, voyou, ganger, vortex de données,
  hologramme, IA angélique, drone volant ou oiseau, squatteur, monstre
  furtif, flic de choc, IA corrompue, cyborg momifié, pouding ou boue
  toxique, spectre numérique, fantôme du Réseau, drone ou golem, entité du
  Réseau.
- Objets : combinaison ou pièce de blindage, module, stim, puce,
  logiciel, implant, tas de crédits.

Vérifié en jeu avec `/` : « un ganger », « un squatteur », « un stim »,
« une puce », « une entité du Réseau », « un drone ou golem ».

## Prochains lots

1. Le reste des monstres (environ 380) et des objets (armes, armures,
   outils, nourriture), avec leurs genres.
2. Les messages fréquents qui parlent de magie (lire, boire, lancer un
   sort...), la monnaie des marchands (« zorkmids »).
3. Les textes d'ambiance (rumeurs, Oracle, gravures, épitaphes) en
   fichiers `dat/*-fr@cyber.*`, puis les quêtes.
4. Un exemple de configuration aux couleurs néon (`MENUCOLOR`, `symset`).
