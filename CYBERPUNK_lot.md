# Thème cyberpunk : liste des lots

Suivi des lots du thème `fr@cyber` (le détail de chaque lot est dans
[CYBERPUNK.md](CYBERPUNK.md)).  Branche `cybernethack`.

## Lots faits

| Lot | Contenu | Commit |
|---|---|---|
| 1 | Mécanique des thèmes (`language:fr@cyber`, `po/themes/`, `check-theme.py`), premier vocabulaire : classes d'objets (stim, puce, implant, module, logiciel, crédit), rôles, lieux, 25 monstres | `96dd52b` |
| 2 | Les rangs : 9 rangs × 13 rôles, au masculin et au féminin ; « Patron », « Patronne » | `95b9d4a` |
| 3 | Descriptions des classes de monstres et d'objets (`;` et `/`) : 28 classes | `cf6af93` |
| 4 | Le reste des monstres et des objets (sélection) : 133 monstres, 67 objets, pluriels irréguliers | `9601c8a` |
| 5 | Messages sur les stims, puces, implants, modules, logiciels (113) ; monnaie « crédit » (`currency()`) ; fruit par défaut traduit | `56b51ae` |
| 6 | Textes d'ambiance : rumeurs (113 vraies, 107 fausses), consultations de la Fixeuse, 53 graffitis, 98 épitaphes ; `makedefs` accepte `fr@cyber` | `be076ce` |
| 7 | Quêtes : textes communs et quête du Netrunner ; outil `po/quest-theme.py` | `2dd4ddb` |
| 8 | Quêtes du Récupérateur (Archéologue) et du Cogneur (Barbare) | `52452fb` |
| 9 | Quêtes du Sauvageon (Homme des cavernes) et du Charcudoc (Guérisseur) | `27f262f` |
| 10 | Quêtes de l'Exécuteur (Chevalier) et du Moine augmenté | `ea8a0fd` |
| 11 | Quêtes du Technoprêtre (Prêtre) et du Traqueur (Rôdeur) | `3db8b45` |
| 12 | Quêtes du Voleur de données (Voleur) et du Ronin urbain (Samouraï) | (ce commit) |

## Lots à faire

Les quêtes se réécrivent avec `po/quest-theme.py` (`dump`, puis `gen`),
environ 70 textes par rôle, avec les noms de leurs lieux (`%H`, `%i`).

| Lot | Contenu |
|---|---|
| 13 | Quêtes du Corpo en goguette (Touriste, 70) et de la Mercenaire (Valkyrie, 68) |
| 14 | Un exemple de configuration aux couleurs néon (`MENUCOLOR`, `symset`, couleurs de la ligne d'état) |

## Pistes pour plus tard

- Les messages qui parlent de magie (« magie », sorts) : gardés pour
  l'instant, le Réseau a sa magie.
- Les artefacts (noms gardés pour l'instant) et les chefs de quête
  (noms propres gardés).
- Les textes d'aide en `dat/<fichier>.fr@cyber` (aide, encyclopédie).
- Hors thème, à corriger dans la traduction française : « Vous êtes
  complètement guéri. » n'a pas de forme féminine (msgctxt `heroine`).
