# État de la traduction française (3 octobre 2026)

Hors `dat/tribute` (citations de Terry Pratchett, mises de côté) et
`dat/license` (la licence fait foi en anglais), environ **95 % des textes
du jeu ont une traduction**. Ce qui reste est surtout du texte que le jeu
n'affiche jamais tel quel.

## Messages du jeu (`po/fr.po`, 16 789 entrées)

| Entrées | Nombre | Part |
|---|---:|---:|
| Traduites | 14 916 | 88,8 % |
| Genres vides (voulu, ils valent masculin) | 1 099 | 6,5 % |
| Formats sans texte (`"%s%s"`…) | 42 | 0,3 % |
| Autres entrées vides | 732 | 4,4 % |

Si on ne compte pas les genres et les formats sans texte, qui n'ont rien à
traduire, on arrive à **95,3 %** (14 916 sur 15 648).

Les 732 entrées vides restantes sont presque toutes des formats anglais
que le code remplace par une phrase entière traduite quand le jeu est en
français (branche `i18n_active()`). C'est ce que dit `TODO.md`, mais elles
n'ont pas été revérifiées une à une. Les vrais oublis visibles en jeu
devraient donc être bien moins nombreux, et le taux réel plus proche de
100 %.

## Fichiers de données : 100 %

- Encyclopédie (`dat/data-fr.base`, compilée en `dat/data.fr`), rumeurs,
  oracles, gravures et épitaphes.
- Les neuf fichiers d'aide (`help`, `hh`, `keyhelp`, `cmdhelp`, `opthelp`,
  `history`, `usagehlp`, `optmenu`, `wizhelp`).
- Textes des quêtes et tutoriel.

## Documentation

- Faits : le Guide du joueur (`doc/Guidebook-fr.mn`), la page de manuel
  `doc/nethack-fr.6` et celles des outils (`doc/recover-fr.6`,
  `doc/dlb-fr.6`, `doc/makedefs-fr.6`).
- Pas traduit : `Guidebook.tex`, qui est une autre version du même guide
  (faible priorité dans `TODO.md`).

## Remarque

Ces chiffres comptent ce qui a une traduction, pas ce qui a été vérifié en
jeu. La relecture en jeu reste à faire (section 7 de `TODO.md`).
