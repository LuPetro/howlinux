# howlinux 1.0.0

`howlinux` ist eine schnelle, lokale CLI-Suche für redaktionell gepflegtes
Linux-Wissen. Sie wählt den passendsten Eintrag aus einer Knowledge Base aus
YAML und Markdown und gibt dessen geprüften Inhalt aus.

Das Programm arbeitet vollständig offline. Es generiert keine Texte, verwendet
keine Cloud-API, sendet keine Telemetrie und führt weder Suchanfragen noch die
in Antworten gezeigten Shell-Befehle aus.

## Funktionen der V1

- robuster, rekursiver Loader mit Schema- und Duplikatvalidierung;
- frei erweiterbare Kategorien unter `knowledge/`;
- globale Einzelwort- und Mehrwort-Synonyme aus `concepts.yaml`;
- testbare Normalisierung, Query-Typen und Erhalt von Linux-Tokens wie
  `tar.gz`, `755`, `-r`, `--recursive` und `2>`;
- In-Memory-Inverted-Index mit deterministischer IDF-Gewichtung;
- erklärbares Ranking für Aliase, Phrasen, Commands, Keywords, Concepts,
  Intent, Titel, Tokens und Tippfehler;
- begrenztes Damerau-Levenshtein-Fuzzy-Matching;
- sichere, unsichere und leere Result-Policy mit stabilen Exitcodes;
- `search`, `list`, `show`, `validate`, `--explain` und ANSI-freies JSON;
- Debug-/Release-Build, CTest-Suite und Installationsregeln.

Die vollständige technische Spezifikation steht in [requirements.md](requirements.md).
Für die tägliche Inhaltspflege gibt es
[docs/knowledge-authoring.md](docs/knowledge-authoring.md). Geplante Arbeiten
nach V1 stehen ausschließlich in [docs/future-features.md](docs/future-features.md).

## Voraussetzungen

Ubuntu oder Debian:

```bash
sudo apt update
sudo apt install -y build-essential cmake libyaml-cpp-dev
```

Benötigt werden ein C++20-Compiler, CMake ab 3.16 und `yaml-cpp`. Zur Laufzeit
bestehen keine Netzwerk- oder Datenbankabhängigkeiten.

## Bauen und testen

Debug-Build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Separater Release-Build:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel
ctest --test-dir build-release --output-on-failure
```

Das Binary liegt je nach Build-Verzeichnis unter `build/howlinux` oder
`build-release/howlinux`.

Optional installieren:

```bash
cmake --install build-release --prefix "$HOME/.local"
```

Dabei wird das Binary nach `bin/` und die Knowledge Base nach
`share/howlinux/knowledge/` unterhalb des Prefix installiert.
Falls `$HOME/.local/bin` nicht im `PATH` liegt, starte es mit
`$HOME/.local/bin/howlinux` oder ergänze dieses Verzeichnis im Shell-Profil.

## Schnellstart

```bash
./build/howlinux rename folder
./build/howlinux "how can i change the name of a directory"
./build/howlinux "what does chmod 755 mean"
./build/howlinux "renmae fodler"
./build/howlinux --explain "change the directory name"
./build/howlinux --json "extract tar.gz"
```

Ein sicherer Treffer rendert den vollständigen, unveränderten Inhalt der
zugehörigen `content.md`. Bei zu geringem Score oder zu kleinem Abstand zum
zweiten Treffer erscheinen stattdessen Vorschläge. Eine irrelevante Query
liefert keinen künstlich erzeugten Text.

## CLI-Referenz

```text
howlinux [options] <query...>
howlinux [options] search <query...>
howlinux [options] list
howlinux [options] show <entry-id>
howlinux [options] validate [path]
```

Optionen:

| Option | Bedeutung |
| --- | --- |
| `-h`, `--help` | Hilfe anzeigen, ohne Knowledge zu laden |
| `-V`, `--version` | Programmversion anzeigen |
| `--knowledge <path>` oder `--knowledge=<path>` | Knowledge-Verzeichnis explizit auswählen |
| `--limit <n>` oder `--limit=<n>` | Höchstens 1 bis 100 Ergebnisse; Standard ist 5 (nur Suche) |
| `--explain` | Bei der Suche Query-Typ, Concepts, Teil-Scores und Matchgründe zeigen |
| `--json` | Stabile, maschinenlesbare und ANSI-freie Ausgabe |
| `--` | Optionsauswertung beenden; der Rest ist Query-Text |

Verwaltungsbefehle:

- `list` zeigt alle gültigen Entries deterministisch nach ID sortiert.
- `show <entry-id>` rendert ausschließlich eine bereits geladene ID. Der Wert
  wird nie als Dateipfad interpretiert.
- `validate [path]` verwendet exakt denselben Loader wie die Runtime und prüft
  Entries, Feldtypen, IDs, Referenzen und Concepts.

Da Linux-Fragen selbst mit einem Bindestrich beginnen können, wird vor solchen
Query-Argumenten `--` verwendet:

```bash
./build/howlinux -- "--recursive"
./build/howlinux --knowledge knowledge search -- "tar -xzf"
```

### Exitcodes

| Code | Bedeutung |
| --- | --- |
| `0` | sicherer Treffer oder erfolgreicher Verwaltungsbefehl |
| `1` | unsicherer Treffer, kein Treffer, unbekannte ID oder Validierungsproblem |
| `2` | ungültige CLI-Argumente oder Optionen |
| `3` | Knowledge-Verzeichnis nicht lesbar oder globale Konfiguration ungültig |

Bei normaler Suche werden Loader-/Konfigurationsdiagnosen auf `stderr`
geschrieben, damit `stdout` eine einzelne JSON-Nutzlast bleibt. `validate
--json` bettet seine Diagnosen dagegen bewusst in das Feld `diagnostics` ein.
CLI-Fehler sowie Help/Version bleiben menschenlesbar; Konfigurationsfehler
geben zusätzlich eine stabile JSON-Fehlernutzlast aus, wenn `--json` aktiv ist.

## Knowledge-Pfad und portable Nutzung

Die Pfadauflösung verwendet die erste passende Quelle in dieser Reihenfolge:

1. `--knowledge <path>`;
2. Umgebungsvariable `HOWLINUX_KNOWLEDGE`;
3. Verzeichnis `knowledge/` neben dem Binary;
4. installierter Pfad `../share/howlinux/knowledge/` relativ zum Binary;
5. `knowledge/` im aktuellen Arbeitsverzeichnis.

Relative explizite Pfade und die Umgebungsvariable werden relativ zum aktuellen
Arbeitsverzeichnis aufgelöst. Für Skripte und Dienste ist ein absoluter
`--knowledge`-Pfad am eindeutigsten.

```bash
HOWLINUX_KNOWLEDGE=/srv/howlinux/knowledge howlinux mv
howlinux --knowledge /srv/howlinux/knowledge validate
```

Ein fehlender Pfad ist ein Konfigurationsfehler. Ein vorhandenes, aber leeres
Verzeichnis ist gültig und wird als Knowledge Base mit null Entries gemeldet.

## Knowledge Base erweitern

Ein Eintrag besteht aus genau einem Verzeichnis mit `meta.yaml` und
`content.md`:

```text
knowledge/
├── concepts.yaml
├── commands/
│   └── mv/
│       ├── meta.yaml
│       └── content.md
└── topics/
    └── rename-folder/
        ├── meta.yaml
        └── content.md
```

Weitere Kategorien sind ohne C++-Änderung möglich. IDs sind über alle
Kategorien hinweg eindeutig. Ein minimaler Eintrag:

```yaml
id: rename-folder
title: Rename a folder
type: howto
command: mv

aliases:
  - rename folder
  - rename directory
  - change folder name

keywords:
  - rename
  - folder
  - directory
  - name

related:
  - mv

intent:
  - how_to
```

Erforderlich sind `id`, `title`, `type` und eine reguläre, lesbare, nicht leere
`content.md` (Symlinks werden aus Sicherheitsgründen abgelehnt). `command`, alle Listen, `difficulty`, `platforms`, `tags` und
`examples` sind optional. Der Markdown-Inhalt darf Überschriften, Listen,
Inline-Code und Codeblöcke enthalten und wird nicht semantisch umgeschrieben.

Nach dem Hinzufügen ist kein Rebuild erforderlich:

```bash
./build/howlinux validate knowledge
./build/howlinux --explain "eine realistische Suchformulierung"
```

Der vollständige Workflow, Qualitätsregeln und eine Review-Checkliste stehen
in [docs/knowledge-authoring.md](docs/knowledge-authoring.md).

## Concepts und Synonyme

Globale, fachlich gleichwertige Formulierungen stehen in
`knowledge/concepts.yaml`:

```yaml
concepts:
  folder:
    - folder
    - directory
    - dir
  rename:
    - rename
    - change name
    - give another name
```

Der Schlüssel ist der kanonische Begriff. Einzelwörter und Mehrwort-Phrasen
werden auf Queries und indexierte Entry-Felder angewendet; die Originaltokens
bleiben parallel erhalten. Ein Ausdruck darf nicht zwei Gruppen zugeordnet
sein. Eine fehlende Datei ist erlaubt und deaktiviert nur die
Concept-Erweiterung; eine widersprüchliche Datei ist ein Konfigurationsfehler.

## Suche und Ranking

Beim Start baut howlinux aus den geladenen Metadaten einen In-Memory-Index.
Queries werden nicht gegen jedes vollständige Markdown-Dokument gescannt.
Seltene Tokens erhalten gemäß ihrer Dokumenthäufigkeit mehr Gewicht als
häufige. Das Ranking kombiniert folgende getrennte Signale:

1. exakter Alias und exakte Phrase;
2. Command, gewichtete Keywords und Concepts;
3. Intent, Titel und allgemeine Token-Überlappung;
4. begrenztes Fuzzy-Matching als schwaches Rettungsnetz.

`--explain` zeigt die Teilwerte. Wiederholte Query-Tokens erhöhen einen Score
nicht mehrfach. Ein Query-Typ ist nur ein Bonus und nie ein harter Filter.
Gleichstände werden über Exact/Phrase, Intent und anschließend alphabetische
Entry-ID reproduzierbar aufgelöst.

## JSON-Ausgabe

`--json` erzeugt eine UTF-8-JSON-Nutzlast ohne ANSI-Sequenzen. Das Suchschema
enthält immer `status` (`confident`, `uncertain` oder `no_match`), `query`,
`query_type`, `concepts`, `results` und `entry`. Ein Resultat enthält `id`,
`title`, `score`, `fuzzy_used` und `match_reasons`; mit `--explain` kommt
`breakdown` mit `exact_alias`, `phrase`, `command`, `keywords`, `concepts`,
`intent`, `title`, `token_idf`, `fuzzy` und `total` hinzu. `entry` ist nur bei
`confident` ein vollständiges Entry-Objekt, sonst `null`.

`show` liefert `{status: "ok", entry}` oder `{status: "not_found", id}`;
`list` liefert `{status: "ok", count, entries[]}`; `validate` liefert
`status` (`valid`, `invalid`, `error`), Knowledge-/Concept-Zähler und
`diagnostics[]` mit `scope`, `severity`, `path`, `entry_id` und `message`.
Help, Version und Parserfehler bleiben bewusst menschenlesbar.

Beispiel:

```bash
./build/howlinux --json --explain "chmod 755" | jq .
```

## Projektstruktur

```text
howlinux/
├── CMakeLists.txt
├── README.md
├── requirements.md
├── docs/
│   ├── knowledge-authoring.md
│   └── future-features.md
├── include/
│   ├── app.hpp
│   ├── cli.hpp
│   ├── concepts.hpp
│   ├── config.hpp
│   ├── diagnostics.hpp
│   ├── index.hpp
│   ├── knowledge.hpp
│   ├── query.hpp
│   ├── render.hpp
│   └── search.hpp
├── src/
├── tests/
└── knowledge/
```

`howlinux_core` kapselt Loader, Query Processor, Index, Ranker, Policy, CLI und
Renderer. Das Executable enthält nur die Prozessgrenze. Die Tests linken
dieselbe Core-Library wie das Programm.

## Fehlerbehebung

### `Knowledge directory ...` ist nicht lesbar

Prüfe den aufgelösten Pfad mit einem expliziten Aufruf:

```bash
./build/howlinux --knowledge "$(pwd)/knowledge" validate
```

Der Pfad muss ein lesbares Verzeichnis sein. `meta.yaml` und `content.md`
müssen reguläre Dateien sein.

### Ein Entry wird übersprungen

```bash
./build/howlinux validate knowledge
```

Jede Diagnose nennt Pfad, soweit vorhanden die Entry-ID und die konkrete
Ursache. Ein kaputter einzelner Entry verhindert nicht, dass gültige Entries
geladen werden.

### Eine Query mit `-r` oder `--recursive` wird als Option gelesen

Setze `--` vor die Query:

```bash
./build/howlinux -- "what does --recursive mean"
```

### Ein Treffer bleibt unsicher

Vergleiche die Kandidaten mit `--explain`. Pflege präzise Aliase und Keywords
oder ein global gültiges Synonym; vermeide generische Alias-Sätze. Die
Confidence-Schwellen sind bewusst konservativ, damit howlinux keine schwache
Übereinstimmung als sichere Antwort ausgibt.

### Build findet `yaml-cpp` nicht

Installiere unter Ubuntu/Debian `libyaml-cpp-dev` und konfiguriere den
CMake-Build danach neu. Bei einer benutzerdefinierten Installation kann
`CMAKE_PREFIX_PATH` auf deren Prefix zeigen.

## Sicherheits- und Vertrauensmodell

Die Knowledge Base ist redaktionell vertrauenswürdig und die einzige Quelle
für Antworten. howlinux öffnet nur Knowledge-Metadaten und Markdown, führt
keinen darin enthaltenen Text aus und interpoliert Query-Text nicht in
Shell-Kommandos. `show` arbeitet ausschließlich mit geladenen IDs. Für
kuratierte Inhalte gelten dennoch die üblichen Review-Regeln: destruktive
Befehle kennzeichnen, Beispiele testen, Pfade korrekt quotieren und keine
Geheimnisse eintragen.
