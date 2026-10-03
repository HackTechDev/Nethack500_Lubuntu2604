#!/bin/bash
# Compile et installe NetHack 5.0.0 sous Lubuntu 26.04, avec les
# interfaces tty, curses, X11 et Qt (ou seulement tty et curses avec
# --console).  Voir INSTALL.md pour le détail des étapes.
set -euo pipefail

cd "$(dirname "$(readlink -f "$0")")"

graphique=1
for arg in "$@"; do
    case "$arg" in
    --console) graphique=0 ;;
    -h|--aide|--help)
        echo "Usage : $0 [--console]"
        echo "  sans option : interfaces tty, curses, X11 et Qt"
        echo "  --console   : interfaces tty et curses seulement"
        exit 0 ;;
    *) echo "Option inconnue : $arg (voir $0 --aide)" >&2; exit 1 ;;
    esac
done

PAQUETS="gcc make curl gettext libncurses-dev"
WIN="WANT_WIN_TTY=1 WANT_WIN_CURSES=1"
if [ "$graphique" = 1 ]; then
    # X11 : widgets Athena ; Qt 6 : bibliothèques, moc et sons
    PAQUETS="$PAQUETS libxaw7-dev qt6-base-dev qt6-base-dev-tools"
    PAQUETS="$PAQUETS qt6-multimedia-dev"
    WIN="WANT_WIN_ALL=1 WANT_WIN_QT6=1 QT6MANUAL=1 HOSTTYPE=x86_64"
fi
HACKDIR="$HOME/nh/install/games/lib/nethackdir"
# fichiers du joueur conservés d'une installation à l'autre
A_GARDER="save record logfile xlogfile livelog perm"

etape() { printf '\n\033[1m==> %s\033[0m\n' "$*"; }

etape "Paquets nécessaires"
manquants=""
for p in $PAQUETS; do
    dpkg-query -W -f='${Status}' "$p" 2>/dev/null | grep -q "ok installed" \
        || manquants="$manquants $p"
done
if [ -n "$manquants" ]; then
    echo "Installation de :$manquants"
    sudo apt-get install -y $manquants
else
    echo "Déjà installés."
fi

etape "Configuration (hints/linux.500)"
(cd sys/unix && sh setup.sh hints/linux.500)

if [ ! -d lib/lua ]; then
    etape "Téléchargement de Lua"
    make fetch-lua
fi

# les objets compilés pour d'autres interfaces ne se lient pas avec
# celles-ci : on les supprime quand le choix d'interfaces change
if [ "$(cat src/.interfaces 2>/dev/null)" != "$WIN" ]; then
    rm -f src/*.o
    echo "$WIN" > src/.interfaces
fi

etape "Compilation ($WIN)"
make -j"$(nproc)" $WIN

sauvegarde=""
if [ -d "$HACKDIR" ]; then
    etape "Sauvegarde des parties et des scores"
    sauvegarde="$HOME/nh/backup-$(date +%Y%m%d-%H%M%S)"
    mkdir -p "$sauvegarde"
    for f in $A_GARDER; do
        if [ -e "$HACKDIR/$f" ]; then
            cp -a "$HACKDIR/$f" "$sauvegarde/"
        fi
    done
    echo "Copie dans $sauvegarde"
fi

etape "Installation"
make install $WIN

if [ -n "$sauvegarde" ]; then
    etape "Restauration des parties et des scores"
    for f in $A_GARDER; do
        [ -e "$sauvegarde/$f" ] || continue
        rm -rf "${HACKDIR:?}/$f"
        cp -a "$sauvegarde/$f" "$HACKDIR/"
    done
    echo "Restauré. La copie reste dans $sauvegarde"
fi

etape "Configuration du joueur (~/.nethackrc)"
if [ -e "$HOME/.nethackrc" ] && ! cmp -s config/nethackrc "$HOME/.nethackrc"; then
    ancien="$HOME/.nethackrc.$(date +%Y%m%d-%H%M%S)"
    mv "$HOME/.nethackrc" "$ancien"
    echo "Ancien fichier conservé dans $ancien"
fi
cp config/nethackrc "$HOME/.nethackrc"
echo "config/nethackrc copié dans ~/.nethackrc"

etape "Terminé"
echo "Lancer le jeu avec : ~/nh/install/games/nethack"
if [ "$graphique" = 1 ]; then
    echo "  interface X11 : ~/nh/install/games/nethack -wX11"
    echo "  interface Qt  : ~/nh/install/games/nethack -wQt"
fi
