# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

NetHack 5.0.0 — a C99 roguelike (descendant of NetHack 3.6/3.7). Much of the upstream docs and `$NHDT-Branch` tags still say "3.7"; treat 3.7 and 5.0 as the same development line.

## Build (Linux / macOS, hints system)

There is no top-level Makefile until the tree is configured. `setup.sh` generates `Makefile`, `src/Makefile`, `util/Makefile`, `dat/Makefile`, `doc/Makefile` from `sys/unix/Makefile.*` plus a hints file.

```sh
cd sys/unix && sh setup.sh hints/linux.500 && cd ../..   # or hints/macOS.500, hints/linux-minimal
make fetch-lua          # one-time: downloads Lua sources into lib/ (needed before first build)
make all                # tty only by default
make WANT_WIN_ALL=1 all # tty + curses + X11 + Qt (needs ncurses, Xaw, Qt dev packages)
make install            # WARNING: deletes and recreates the install dir; save record/logfile/sysconf first
make spotless           # remove all generated files; do this before retrying a failed build
```

- Interfaces are selected with `WANT_WIN_TTY=1`, `WANT_WIN_CURSES=1`, `WANT_WIN_X11=1`, `WANT_WIN_QT=1`, `WANT_DEFAULT=...` (see `sys/unix/NewInstall.unx`).
- Default install (linux.500): `~/nh/install/games/lib/nethackdir`, launcher in `~/nh/install/games`. With `WANT_SOURCE_INSTALL=1` it installs into `./playground` in the source tree.
- The hints files use `#-INCLUDE` directives (not comments) pulling from `sys/unix/hints/include/*.500`; compiler flags live in `compiler.500`.
- CI (`azure-pipelines.yml`) is upstream's and references `hints/linux.370`/`macos.370`; in this tree the equivalents are `linux.500`/`macOS.500`.
- Other platforms: Windows (`sys/windows/`, VS solution or `GNUmakefile` for MinGW), MS-DOS cross-compile (`CROSS_TO_MSDOS=1`, see `Cross-compiling`), Amiga, VMS.
- Docs: `make Guidebook`, `make Guidebook.txt`, `make Guidebook.pdf`.

## Tests

There is no automated test runner. `test/*.lua` are Lua scripts run inside the game:

1. Build **without DLB** (linux.500 adds `-DDLB` to `NHCFLAGS`; remove it) and install.
2. Copy the `test/*.lua` files into the installed playground dir (HACKDIR).
3. Start in wizard mode (`nethack -D`; the user must be listed in `WIZARDS` in `sysconf`).
4. Run `#wizloadlua` and give a test file name (e.g. `test_src.lua`, `test_des.lua`).

Tests exercise the `nh.*` / `des.*` / `obj.*` / `selection.*` Lua bindings and fail via Lua `error()`.

## Architecture

- **`src/`** — game core. `allmain.c` holds the main loop (`moveloop`); `cmd.c` holds the command table (`extcmdlist[]`) and key dispatch; wizard-mode commands are in `wizcmds.c`.
- **`include/extern.h`** — prototypes for every cross-file function, grouped by source file. Add new non-static functions here under their file's section.
- **Global state** lives in lettered instance structs declared in `include/decl.h` and defined in `src/decl.c`: `ga`..`gz` for transient globals and `svb`, `svc`, … (`instance_globals_saved_*`) for state that is saved/restored. A variable named `foo` lives in `gf.foo`/`svf.foo` according to its first letter. Other key globals: `u` (hero, `you.h`), `flags`/`iflags` (`flag.h`), `svl.level` (current level map, `rm.h`).
- **Data tables via X-macros**: monsters (`include/monsters.h`), objects (`include/objects.h`), artifacts (`artilist.h`), map symbols (`defsym.h`), options (`optlist.h`), sound effects (`seffects.h`). These headers are included multiple times with different macro definitions to generate enums, arrays and names — edit the list, not the generated uses.
- **Lua integration**: level design, the dungeon layout (`dat/dungeon.lua`), quest text (`dat/*-{strt,loca,goal,fila,filb}.lua`, `quest.lua`), and many special levels (`dat/*.lua`) are Lua loaded at runtime (no more yacc/lex level compiler). C bindings: `src/nhlua.c` (core `nh.*`), `src/sp_lev.c` (`des.*` level builder), `src/nhlobj.c` (objects), `src/nhlsel.c` (selections). Lua sources come from `make fetch-lua` into `lib/` (or the `submodules/lua` git submodule).
- **Window ports**: the core talks to the UI only through `struct window_procs` (`include/winprocs.h`, dispatch in `src/windows.c`). Implementations: `win/tty`, `win/curses`, `win/X11`, `win/Qt` (C++), `win/win32`, `win/shim` / `win/chain` (for embedding/tracing). Sound follows the same pattern (`include/sndprocs.h`, `sound/`).
- **Platform layer**: `sys/unix` (`unixmain.c` is the entry point), `sys/share` (shared helpers; exactly one of `posixregex.c`/`cppregex.cpp`/`pmatchregex.c` is linked), `sys/windows`, `sys/msdos`, `sys/libnh` (NetHack as a library/wasm).
- **`util/`** — build-time tools: `makedefs` (generates headers/data files such as `date.h`, `rumors`, `oracles`), `dlb` (data librarian archive `nhdat`), `recover`, `sfctool` (save-file conversion).
- **Save files**: serialization in `src/sfbase.c`/`sfstruct.c` with macros in `include/sfmacros.h`/`sfprocs.h`. Changing saved structures requires bumping `EDITLEVEL` in `include/patchlevel.h` (invalidates old saves/bones).
- **Translation (fork addition)**: `src/nhi18n.c` loads a GNU `.mo` catalog chosen by `OPTIONS=language:<lang>` (`optfn_language` in `options.c`; enabled by `NHI18N` in `unixconf.h`). `vpline()` translates every message format (after `You()`/`pline_The()`&c add their "You "/"The " prefix, so msgids are whole sentences; `po/update-pot.sh` adds those prefixes when extracting) and the argument of a bare `"%s"`; `yn_function()`, `getlin()` and `paranoid_ynq()` translate their prompt. MSGTYPE rules are matched against the English text too. Other text (built with `Sprintf`, menu entries, window text) needs `_()` (`include/nhi18n.h`); `nh_gettext()` has `format_arg`, so `pline(_("..."), ...)` is still format-checked. `po/<lang>.po` → `dat/<lang>.mo` (msgfmt) → HACKDIR; `make update-po` re-extracts `po/nethack.pot` and merges (no fuzzy matching). Verbs passed to `getobj()` are marked `NC_("verb", ...)` and shown with `C_("verb", word)` while the English `word` keeps driving its logic. At run time a translation whose printf conversions don't match the msgid is ignored (dropping trailing arguments is allowed). `C_("ctx", s)` translates with a msgctxt (`NC_` marks it in tables); `gendered_word(word, gend)` (role.c) gives the `"feminine"` form for a female hero. Monster names: `x_monnam()` hands over to `x_monnam_i18n()` (do_name.c) when translating; names use msgctxt `monster`, their grammatical gender msgctxt `gender` (`f`/`e`/`n`, extracted from `monsters.h` by `update-pot.sh`); `i18n_mon_fem(mon)` selects `C_("feminine", ...)` message forms. Never make a message format out of an English verb fragment (`You("%s %s!", verb, ...)`): write one sentence per verb so each can be translated. Never translate the data tables themselves (role/race names, `hu_stat[]`, `conditions[]`...): they are matched against option values and `hilite_status` rules; mark them `N_()` and translate where displayed. Where the English text is composed from fragments, branch on `i18n_active()` and use a whole translatable format instead. Translations can be longer than English: use `Snprintf`/`copynchars` into fixed buffers. Don't use `_()` in files linked into `util/` programs (`alloc.c`, `dlb.c`, `hacklib.c`, `objects.c`, `monst.c`, `date.c`, save-file code).
- **Config**: compile-time feature switches are in `include/config.h`, `global.h`, `unixconf.h`/`windconf.h`; runtime system config in `sys/unix/sysconf`.

## Conventions (from `DEVEL/code_style.txt`)

- 4-space indentation, no tabs, lines ≤ 78 columns. Function return type on its own line, name at column 0, opening brace on its own line; ANSI prototypes. K&R braces for control statements; `case` labels not indented; mark fall-throughs explicitly with a comment (`/*FALLTHRU*/` or `/* fall-through */`).
- No declarations inside `for (...)` initializers or conditions; wrap assignments used as conditions in extra parens: `if ((p = f()))`.
- Space after casts `(char *) s`; `sizeof (type)` with a space, `sizeof expr` without parens.
- Use `staticfn` (not `static`) for file-local functions in `src/*.c`; never for data. Hence static function names must be unique across files.
- Avoid identifiers `near`/`far`. Many files end with a `/*filename*/` comment.
- Record user-visible changes and fixes as a new entry in `doc/fixes5-0-0.txt` (continuation lines indented with a tab).
- Behavior changes that should alert players can use the `FEATURE_NOTICE_VER` mechanism (`DEVEL/code_features.txt`).
