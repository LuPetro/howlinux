# howlinux v0.01

Offline CLI-Tool: erklärt Linux-Befehle basierend auf einer lokalen,
textbasierten Knowledge Base (YAML + Markdown). Keine AI, keine
Embeddings, keine Datenbank, kein Internet.

## Stand v0.01

Umgesetzt (entspricht "Schritt 6" aus dem Plan):

- CMake + Buildsystem
- Knowledge-Format (`meta.yaml` + `content.md` pro Eintrag)
- Loader, der `knowledge/commands/*` und `knowledge/topics/*` automatisch einliest
- Einfache Suche: Normalisierung (lowercase, Satzzeichen weg, Stopwörter raus) +
  Scoring (exact alias match, keyword match, command match, title match)
- Terminal-Renderer inkl. "Did you mean" bei schwachen Treffern
- 5 Beispiel-Einträge (mv, chmod, rename-folder, chmod-755, extract-tar)

Noch NICHT umgesetzt (kommt in v0.02+, siehe Notizen):

- Concepts/Synonyme (`concepts.yaml`)
- Inverted Index (aktuell wird bei jeder Query linear über alle Einträge iteriert –
  bei 5-20 Einträgen völlig egal, erst bei hunderten relevant)
- Fuzzy Matching (Tippfehler wie `renmae` werden noch nicht erkannt)
- CLI11 (aktuell werden Argumente einfach manuell zusammengefügt)
- Tests (Catch2)

## Setup in WSL / Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential cmake libyaml-cpp-dev
```

## Bauen

Im Projekt-Root (`howlinux/`):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Das Binary liegt danach unter `build/howlinux`.

## Ausführen

WICHTIG: `howlinux` sucht den Ordner `knowledge/` relativ zum aktuellen
Arbeitsverzeichnis. Du musst es also aus dem Projekt-Root heraus starten:

```bash
./build/howlinux rename folder
./build/howlinux mv
./build/howlinux chmod 755
./build/howlinux extract tar.gz
```

Falls du es systemweit als `howlinux` verfügbar machen willst, kannst du es
später z.B. nach `/usr/local/bin` kopieren – dann müsstest du aber auch den
`knowledge/`-Ordner an einen festen Pfad legen und den Pfad im Code fest
verdrahten statt `"knowledge"` relativ zu verwenden. Für v0.01 reicht es,
es aus dem Projektordner heraus zu starten.

## Neuen Knowledge-Eintrag hinzufügen (ohne Code-Änderung)

1. Neuen Ordner anlegen, z.B. `knowledge/topics/delete-file/`
2. Darin `meta.yaml` anlegen (siehe bestehende Einträge als Vorlage)
3. Darin `content.md` anlegen mit dem eigentlichen Erklärtext
4. Neu bauen ist NICHT nötig – `howlinux` liest den Ordner beim Start neu ein.

## Projektstruktur

```text
howlinux/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp        Einstiegspunkt, Argumente einlesen, Ablauf steuern
│   ├── knowledge.cpp   Lädt meta.yaml + content.md aus knowledge/
│   ├── search.cpp      Normalisierung + Scoring
│   └── render.cpp      Terminal-Ausgabe
├── include/
│   ├── knowledge.hpp
│   ├── search.hpp
│   └── render.hpp
└── knowledge/
    ├── commands/
    │   ├── mv/
    │   └── chmod/
    └── topics/
        ├── rename-folder/
        ├── chmod-755/
        └── extract-tar/
```

## Nächste Schritte (laut Plan)

1. `concepts.yaml` (Synonyme wie folder/directory/dir)
2. Inverted Index statt linearem Scan
3. Fuzzy Matching (Levenshtein) für Tippfehler
4. Mehr Knowledge-Einträge (Richtung 75-100)
5. Catch2 Tests
