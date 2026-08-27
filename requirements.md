# howlinux - vollständige Projektanforderungen

## 1. Zweck und Ziel

`howlinux` ist ein schnelles, lokales CLI-Programm, das Linux-Fragen aus einer von Menschen gepflegten Knowledge Base beantwortet. Es ist keine generative KI und kein Chatbot: Das Programm findet den passendsten geprüften Eintrag und rendert dessen Inhalt unverändert.

Das System muss offline, deterministisch, ressourcenschonend und auf schwachen Linux-Systemen nutzbar sein. Ein neuer Knowledge-Eintrag darf keine C++-Änderung benötigen.

Diese Datei ist die Arbeitsgrundlage für einen Implementierungs-Agenten. Der Agent soll das gesamte technische System von Build bis Tests umsetzen. Die fachlichen Knowledge-Einträge selbst werden anschließend manuell ergänzt.

## 2. Ausgangslage

Das Repository enthält bereits eine funktionierende v0.01-Basis:

- CMake und C++20
- `yaml-cpp` zum Laden von `meta.yaml`
- Markdown-Inhalte in `content.md`
- Einträge unter `knowledge/commands/<id>/` und `knowledge/topics/<id>/`
- lineare Suche mit Normalisierung, Stopwords und einfachem Scoring
- Terminal-Renderer

Die bestehende Struktur und die vorhandenen Einträge müssen erhalten bleiben. Vorhandene öffentliche APIs dürfen geändert werden, wenn dadurch die Zielarchitektur sauber erreicht wird; unnötige inkompatible Änderungen sind zu vermeiden.

## 3. Nicht-Ziele

Folgende Funktionen gehören nicht zu Version 1:

- Textgenerierung oder ein eigenes LLM
- Netzwerkzugriff, Cloud-API oder Telemetrie
- Embeddings oder eine externe Vector Database
- automatische Erstellung fachlicher Knowledge-Einträge
- Ausführung der in der Antwort angezeigten Shell-Befehle
- Shell-Kommandos aus Benutzereingaben ausführen
- Volltextsuche über beliebige Binärdateien
- Übersetzungsautomatik

Embeddings können später als austauschbare Ranking-Stufe ergänzt werden, dürfen aber die erste Implementierung nicht voraussetzen.

## 4. Zielarchitektur

Die Anwendung soll in klar getrennte Komponenten gegliedert sein:

```text
Knowledge-Dateien
  -> Loader/Validator
  -> Query Processor
  -> Candidate Generator
  -> Ranker
  -> Result Policy
  -> Renderer
```

### 4.1 Knowledge Loader

Der Loader liest rekursiv alle gültigen Einträge unter einem konfigurierten Knowledge-Verzeichnis. Ein Eintrag besteht aus:

```text
<category>/<entry-id>/meta.yaml
<category>/<entry-id>/content.md
```

Unterstützte Kategorien sind mindestens `commands` und `topics`; weitere Kategorien müssen ohne C++-Änderung möglich sein.

Der Loader muss:

- `id`, `title`, `type`, `command`, `aliases`, `keywords`, `related` laden
- optionale Felder mit sinnvollen Defaults behandeln
- UTF-8-Text in Markdown unverändert laden
- fehlende oder ungültige einzelne Einträge mit einer verständlichen Warnung überspringen
- bei fehlendem Knowledge-Verzeichnis einen klaren Fehler liefern
- doppelte IDs erkennen und nicht still überschreiben
- Pfad, ID und konkrete Validierungsursache in Warnungen nennen
- deterministische Reihenfolge liefern, vorzugsweise sortiert nach ID

Ein kaputter Eintrag darf nicht den Start der gesamten Anwendung verhindern. Ein komplett leeres Verzeichnis ist ein gültiger Zustand, aber die CLI muss darauf hinweisen.

### 4.2 Query Processor

Die rohe Query wird in eine `QueryContext`-Struktur umgewandelt. Diese soll mindestens enthalten:

- Originaltext
- normalisierte Token
- normalisierte zusammenhängende Query
- erkannte Phrasen
- erkannte Concepts/Kanonbegriffe
- erkannter Query-Typ

Die Normalisierung muss:

1. UTF-8-freundlich mit ASCII-Linux-Vokabular umgehen
2. Groß-/Kleinschreibung ignorieren
3. Satzzeichen und überflüssige Leerzeichen behandeln
4. Bindestriche, Unterstriche und typische Shell-Trennzeichen sinnvoll als Wortgrenzen behandeln
5. Stopwords entfernen
6. wiederholte Tokens deduplizieren, ohne die Phrase-Reihenfolge zu verlieren
7. Zahlen, Flags und Versionen wie `755`, `tar.gz`, `-r`, `--recursive` und `2>` nicht zerstören

Die Normalisierung muss aus einer einzelnen Funktion bzw. testbaren Klasse bestehen. Die Roh-Query bleibt für die Ausgabe erhalten.

### 4.3 Stopwords

Stopwords werden aus einer Datei oder einer klar zentralisierten Konfiguration geladen. Mindestens die bisher verwendeten englischen Wörter müssen abgedeckt werden, zum Beispiel `a`, `an`, `the`, `i`, `do`, `does`, `can`, `how`, `please`, `to`, `of`, `is`, `for`, `me`, `my`.

Ein relevantes Linux-Token wie `mv`, `cp`, `rm`, `chmod`, `tar`, `ssh` oder `sudo` darf niemals als Stopword behandelt werden. Leere Queries nach der Normalisierung müssen sauber behandelt werden.

### 4.4 Concepts und Synonyme

Die Knowledge Base bekommt eine globale Datei:

```text
knowledge/concepts.yaml
```

Beispiel:

```yaml
concepts:
  folder:
    - folder
    - directory
    - dir
  delete:
    - delete
    - remove
    - erase
  rename:
    - rename
    - change name
    - give another name
  permissions:
    - permission
    - permissions
    - rights
    - access rights
  extract:
    - extract
    - unpack
    - decompress
```

Anforderungen:

- Jede Gruppe hat genau einen kanonischen Namen.
- Einzelwörter und Mehrwort-Phrasen werden unterstützt.
- Concepts werden sowohl auf Queries als auch auf Eintragsfeldern angewendet.
- Die Originaltokens bleiben zusätzlich verfügbar, damit `directory` und `folder` nicht ununterscheidbar in der Ausgabe werden.
- Zirkuläre oder doppelte Definitionen werden validiert und verständlich gemeldet.
- Eine fehlende `concepts.yaml` ist erlaubt und bedeutet: keine Synonym-Erweiterung.
- Die Datei darf später erweitert werden, ohne C++-Code zu ändern.

### 4.5 Query-Typen

Der Query Processor erkennt mindestens:

```text
EXPLAIN:  what is X, what does X mean, explain X
HOW_TO:   how do I X, how can I X, X a folder
WHY:      why does X, why can't X, why is X
COMMAND:  direkte Kommandos oder Flags wie mv, chmod 755
GENERAL:  alles andere
```

Die Erkennung ist heuristisch und darf nie dazu führen, dass ein ansonsten guter Treffer verworfen wird. Der Query-Typ ist ein Ranking-Signal, kein harter Filter.

Die Metadaten unterstützen dafür mindestens:

```yaml
type: howto
intent:
  - how_to
  - explain
```

Die bisherige Zeichenkette `type: howto` und bestehende Einträge müssen weiterhin funktionieren.

## 5. Suche und Ranking

### 5.1 Kandidaten-Generierung

Die Suchmaschine soll nicht mehr bei jeder Query alle Einträge vollständig vergleichen. Beim Laden wird ein Inverted Index im RAM aufgebaut:

```text
token -> Entry IDs
```

Indexiert werden mindestens:

- ID und Titel
- Alias-Tokens und Alias-Phrasen
- Keywords
- Command
- kanonische Concept-Tokens

Für wenige Einträge darf ein vollständiger Fallback bestehen. Der Index muss aber die primäre Implementierung sein.

Kandidaten entstehen aus Alias-, Token-, Concept-, Command- und optional Fuzzy-Treffern. Gibt es keine Index-Kandidaten, darf ein begrenzter Fuzzy-Fallback verwendet werden.

### 5.2 Score

Der Score muss aus nachvollziehbaren Teilwerten bestehen. Die konkreten Konstanten dürfen kalibriert werden, aber die folgende Priorität ist verbindlich:

```text
exact alias match       sehr hoch
exact phrase match      hoch
token/concept overlap    mittel bis hoch
wichtige Keywords        mittel
Command match            hoch
Intent match             mittel
Titelmatch               niedrig bis mittel
BM25-ähnliche Seltenheit mittel
Fuzzy match              niedrig
```

Ein möglicher Startpunkt:

```text
exact alias              +100
phrase match              +40
command match             +30
gewichtetes Keyword       +20 je relevanter Übereinstimmung
Concept/Synonym            +15 je Übereinstimmung
Intent match               +20
Titelmatch                 +10 je Token
Fuzzy match                +10 bis maximal +15 je Token
```

Die Implementierung soll Teil-Scores in `SearchResult` oder einer Debug-Struktur speichern, damit `--explain` den Rang nachvollziehbar machen kann. Mehrfaches Auftreten desselben Tokens darf den Score nicht unkontrolliert aufblasen.

### 5.3 Token-Gewichtung

Häufige, wenig aussagekräftige Tokens sollen weniger zählen als seltene, fachlich starke Tokens. Für v1 reicht eine deterministische IDF-ähnliche Gewichtung:

```text
idf(token) = log((N + 1) / (document_frequency(token) + 1)) + 1
```

Die Gewichtung muss beim Indexaufbau aus der geladenen Knowledge Base berechnet werden. Ein explizites Gewicht im YAML darf später unterstützt werden, ist aber für v1 optional.

### 5.4 Fuzzy Matching

Fuzzy Matching ist ein Rettungsnetz, kein Primärsignal. Implementiert wird mindestens Levenshtein-Distanz oder eine gleichwertige edit-distance-basierte Methode.

Regeln:

- nur für ausreichend lange Tokens, standardmäßig ab vier Zeichen
- maximale Distanz abhängig von der Tokenlänge, zum Beispiel 1 bei 4-6 und 2 ab 7 Zeichen
- Zahlen, Flags und Kommandos nicht aggressiv fuzzy matchen
- Fuzzy-Scores deutlich unter Exact-, Phrase- und Concept-Scores halten
- im Ergebnis kenntlich machen, wenn ein Fuzzy-Match verwendet wurde

Beispiel: `renmae fodler` muss `rename folder` als starken Kandidaten finden, darf aber keinen unverbundenen Treffer dominieren.

### 5.5 Sortierung und Gleichstände

Ergebnisse werden absteigend nach Gesamtscore sortiert. Gleichstände werden deterministisch nach folgenden Kriterien aufgelöst:

1. höherer Exact-/Phrase-Teilscore
2. höherer Intent-Teilscore
3. alphabetische Entry-ID

Die API muss die besten Ergebnisse und optional ein konfigurierbares Limit liefern. Standardmäßig werden höchstens fünf Vorschläge gerendert.

## 6. Result Policy und Benutzerverhalten

Die Anwendung unterscheidet drei Fälle:

1. **Sicherer Treffer:** Der beste Treffer überschreitet eine konfigurierbare Schwelle und liegt ausreichend vor dem zweitbesten Treffer. Der vollständige Eintrag wird gerendert.
2. **Unsicherer Treffer:** Es gibt Kandidaten, aber keine ausreichende Sicherheit. Es werden Titel, IDs, Score oder kurze Match-Informationen als Vorschläge angezeigt.
3. **Kein Treffer:** Es gibt keine sinnvollen Kandidaten. Die Ausgabe erklärt kurz, dass nichts gefunden wurde, und zeigt eine Beispielsyntax.

Schwellenwerte gehören in eine zentrale Konfiguration und nicht als verstreute Magic Numbers in `main.cpp`.

Die CLI soll mindestens unterstützen:

```text
howlinux <query...>
howlinux search <query...>
howlinux --help
howlinux --version
howlinux --knowledge <path> <query...>
howlinux --limit <n> <query...>
howlinux --explain <query...>
```

Zusätzlich empfohlen:

```text
howlinux list
howlinux show <entry-id>
howlinux validate [path]
howlinux --json <query...>
```

Die exakte Syntax darf an die vorhandene CLI angepasst werden, aber Hilfe, Version, Knowledge-Pfad, Limit, Validierung und Debug-Ranking müssen vorhanden sein.

Exit-Codes:

- `0`: Antwort oder valide Vorschläge erfolgreich erzeugt
- `1`: kein Treffer oder unsicherer Treffer, falls für Skripte sinnvoll dokumentiert
- `2`: ungültige CLI-Argumente
- `3`: Knowledge-Verzeichnis nicht lesbar oder Konfigurationsfehler

Die Ausgabe für Menschen bleibt standardmäßig gut lesbares Terminal-Markdown/ANSI. JSON darf keine ANSI-Sequenzen enthalten und muss eine stabile Struktur besitzen.

## 7. Entry-Datenformat

Bestehende Einträge wie `knowledge/topics/rename-folder/` bleiben gültig. Das Minimalformat:

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

Zusätzlich mögliche optionale Felder:

```yaml
difficulty: beginner
platforms:
  - linux
  - ubuntu
tags:
  - filesystem
examples:
  - mv old-name new-name
```

Unbekannte optionale Felder sollen entweder ignoriert oder als Warnung gemeldet werden; sie dürfen den Loader nicht zum Absturz bringen.

`content.md` muss der geprüfte Antworttext sein. Er darf Shell-Codeblöcke, Überschriften, Listen und Inline-Code enthalten. Der Renderer darf Markdown nicht inhaltlich verändern.

## 8. Entry-Autor-Anleitung

Diese Anleitung gehört später auch in die README oder eine eigene Datei `docs/knowledge-authoring.md`.

### 8.1 Einen Eintrag anlegen

1. Einen sprechenden, stabilen Slug unter `knowledge/topics/<id>/` oder `knowledge/commands/<id>/` anlegen.
2. `meta.yaml` und `content.md` erstellen.
3. Eine eindeutige `id` und einen lesbaren `title` vergeben.
4. Aliase als echte Nutzerfragen und alternative Formulierungen ergänzen.
5. Keywords auf die wichtigsten Linux-Begriffe begrenzen.
6. Das relevante Kommando und verwandte Entry-IDs eintragen.
7. Den Inhalt mit überprüften Befehlen und Beispielen schreiben.
8. `howlinux validate knowledge` und die Suche mit mehreren Formulierungen ausführen.

### 8.2 Gute Aliase

Aliase decken Formulierungen ab, die Benutzer tatsächlich eingeben würden:

```yaml
aliases:
  - rename a folder
  - change the name of a directory
  - give a directory a different name
```

Nicht hilfreich sind generische Aliase wie `linux help` oder zehn fast identische Varianten. Synonyme wie `directory`, `dir` und `folder` gehören bevorzugt in `concepts.yaml`, wenn sie global gelten.

### 8.3 Gute Keywords

Keywords sind kurze, fachlich wichtige Begriffe. Dazu gehören Verben, Objekte, Kommandos und relevante Optionen. Stopwords, vollständige Sätze und sehr allgemeine Wörter sollen vermieden werden.

```yaml
keywords:
  - rename
  - folder
  - directory
  - mv
```

### 8.4 Gute Inhalte

Jeder Eintrag sollte möglichst diese Reihenfolge verwenden:

1. kurze Erklärung, was das Kommando oder Konzept tut
2. allgemeine Syntax
3. ein einfaches Beispiel
4. wichtige Varianten oder Flags
5. typische Fehler und Sicherheitswarnungen
6. verwandte Einträge

Keine Befehle ungeprüft kopieren. Destruktive Befehle müssen ausdrücklich markiert werden. Platzhalter wie `OLD_NAME` und `NEW_NAME` müssen klar erkennbar sein. Die Einträge erklären und zeigen Befehle; `howlinux` führt sie niemals automatisch aus.

### 8.5 Review-Checkliste für Einträge

- Ist die Aussage für Linux korrekt?
- Funktionieren die Beispiele oder ist ihre Voraussetzung erklärt?
- Sind Dateinamen mit Leerzeichen korrekt quotiert?
- Sind destruktive Auswirkungen genannt?
- Gibt es mindestens drei realistische Suchformulierungen?
- Sind `id`, `related` und Commands korrekt geschrieben?
- Liefert die Validierung keine Warnungen?
- Liefert die Suche sowohl direkte als auch synonymische Formulierungen?

## 9. Persistenter Index und Index-Compiler

Für v1 muss der In-Memory-Index beim Start zuverlässig funktionieren. Zusätzlich soll die Architektur einen optionalen Index-Compiler vorsehen:

```text
howlinux-index knowledge/ -o howlinux.idx
howlinux --index howlinux.idx "rename folder"
```

Der Compiler liest exakt dieselben YAML/Markdown-Dateien und erzeugt ein versioniertes, dokumentiertes Format. Das Format muss enthalten:

- Formatversion
- Knowledge-Signatur oder Änderungszeitpunkte
- Einträge und Inhalte beziehungsweise Verweise darauf
- Inverted Index
- Concept-Daten
- IDF-Statistiken

Der Runtime-Loader prüft die Formatversion und lehnt inkompatible Indizes mit einer klaren Meldung ab. Ein veralteter Index darf nicht still falsche Antworten liefern. Der Index-Compiler ist nach Möglichkeit ein separates Executable oder eine klar getrennte Library-Komponente. Er darf die manuelle Pflege von YAML/Markdown nicht ersetzen.

## 10. Projektstruktur

Die bestehende Struktur soll in etwa zu folgender Zielstruktur wachsen:

```text
howlinux/
├── CMakeLists.txt
├── README.md
├── requirements.md
├── docs/
│   └── knowledge-authoring.md
├── include/
│   ├── knowledge.hpp
│   ├── query.hpp
│   ├── concepts.hpp
│   ├── index.hpp
│   ├── search.hpp
│   ├── cli.hpp
│   └── render.hpp
├── src/
│   ├── main.cpp
│   ├── cli.cpp
│   ├── knowledge.cpp
│   ├── query.cpp
│   ├── concepts.cpp
│   ├── index.cpp
│   ├── search.cpp
│   └── render.cpp
├── tests/
│   ├── test_query.cpp
│   ├── test_concepts.cpp
│   ├── test_search.cpp
│   ├── test_knowledge.cpp
│   └── fixtures/
└── knowledge/
    ├── concepts.yaml
    ├── commands/
    └── topics/
```

Die Aufteilung ist ein Zielbild, keine Aufforderung zu blindem Umbenennen. Der Agent soll kleine, testbare Module bilden und den aktuellen Code schrittweise migrieren.

## 11. Build und Abhängigkeiten

- C++20 bleibt Standard.
- CMake bleibt das Buildsystem.
- `yaml-cpp` bleibt die YAML-Abhängigkeit.
- Eine kleine CLI- oder Testbibliothek darf ergänzt werden, wenn sie sauber über CMake eingebunden wird.
- Keine Netzwerkabhängigkeiten zur Laufzeit.
- Debug- und Release-Build müssen funktionieren.
- Compiler-Warnungen sollen auf einem sinnvollen Niveau aktiviert werden.
- Der Build muss auf Ubuntu/Debian mit dokumentierten Paketen reproduzierbar sein.

Empfohlene Befehle:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

## 12. Tests

Tests sind Pflicht. Mindestens abzudecken:

### Query und Normalisierung

- lowercase und Satzzeichen
- Stopwords
- leere Query
- Mehrwort-Phrasen
- `tar.gz`, Zahlen und Flags
- deterministische Token-Reihenfolge

### Concepts

- Einzelwort-Synonyme
- Mehrwort-Synonyme
- fehlende Datei
- doppelte und ungültige Definitionen
- Mapping auf denselben kanonischen Begriff

### Loader

- gültiger Eintrag
- fehlendes `meta.yaml`
- fehlendes `content.md`
- ungültiges YAML
- doppelte IDs
- unbekannte optionale Felder
- deterministische Reihenfolge

### Ranking

- exakter Alias gewinnt
- `change the directory name` findet `rename-folder`
- `remove directory` kann über Concepts einen Delete-F Treffer finden
- `renmae fodler` findet `rename-folder`, aber Fuzzy bleibt schwächer als exact
- Command-Match für `chmod 755` und `mv`
- Intent-Match beeinflusst, ersetzt aber nicht den Inhaltsscore
- Gleichstände sind stabil
- keine Ergebnisse bei irrelevanter Query

### CLI und Renderer

- Help und Version
- ungültige Optionen
- Knowledge-Pfad
- sichere Antwort, Vorschläge und kein Treffer
- JSON enthält gültige, ANSI-freie Ausgabe
- Exit-Codes sind dokumentiert und stabil

Die Tests sollen mit kleinen temporären Fixtures arbeiten und nicht vom aktuellen Arbeitsverzeichnis abhängen.

## 13. Performance und Robustheit

Bei 10.000 Einträgen muss eine Query ohne Netzwerk typischerweise deutlich unter 100 ms bleiben, nachdem die Knowledge Base geladen wurde. Der Start muss nicht vorzeitig für eine unrealistische Benchmark optimiert werden.

- Indexzugriff statt vollständigem Feldvergleich pro Query
- keine unnötigen Kopien großer Markdown-Inhalte während der Suche
- Fuzzy-Vergleiche nur gegen begrenzte Kandidaten oder passende Token
- keine Abstürze bei leerem Input, kaputtem YAML oder fehlenden Dateien
- keine Shell-Ausführung aus Suchtext oder Metadaten
- Pfade dürfen nicht unkontrolliert aus User-Input geöffnet werden; `show` löst nur geladene IDs auf

## 14. Dokumentation

README und Dokumentation müssen aktualisiert werden und mindestens enthalten:

- Installation
- Debug- und Release-Build
- CLI-Beispiele
- Knowledge-Pfad und portable Nutzung
- alle verfügbaren Optionen
- Entry-Format
- Concepts/Synonyme
- Validierung
- Tests
- Fehlerbehebung
- klare Aussage: offline, keine KI-Textgenerierung, keine Befehlsausführung

Die `requirements.md` bleibt die übergeordnete technische Spezifikation. `docs/knowledge-authoring.md` ist die kurze tägliche Anleitung für die manuelle Inhaltspflege.

## 15. Umsetzungsreihenfolge für den Folge-Agenten

1. Bestehenden Build und aktuelle Tests bzw. Beispielaufrufe verifizieren.
2. Datenmodelle und Loader robuster machen, ohne vorhandene Einträge zu brechen.
3. Query Processor inklusive Query-Typen und Tests implementieren.
4. `concepts.yaml` laden und Concept-Normalisierung testen.
5. Inverted Index und IDF-ähnliche Gewichtung implementieren.
6. Ranking mit Teil-Scores, stabiler Sortierung und Result Policy ersetzen.
7. Levenshtein-Fallback ergänzen und gegen Fehlmatches testen.
8. CLI-Parser mit Help, Version, Pfad, Limit, Validate und Explain bauen.
9. Renderer für sichere Treffer, Vorschläge, Fehler und optional JSON vervollständigen.
10. Test-Suite, CTest und Fixture-Isolation fertigstellen.
11. README und `docs/knowledge-authoring.md` schreiben.
12. Optional den persistenten Index-Compiler als separaten, gut abgegrenzten Schritt umsetzen.
13. Erst danach große Mengen neuer Knowledge-Einträge manuell ergänzen.

Nach jedem Schritt muss der Agent den kleinsten passenden Test ausführen. Am Ende müssen Build, Tests, Validierung der Beispiel-Knowledge Base und die wichtigsten CLI-Beispiele erfolgreich sein.

## 16. Abnahmekriterien

Das Projekt gilt als technisch fertig, wenn:

- ein sauberer Debug- und Release-Build funktioniert
- `ctest` ohne Fehler durchläuft
- `howlinux "how can i change the name of a directory"` den Rename-Eintrag sicher findet
- `howlinux "renmae fodler"` einen sinnvollen Rename-Treffer oder Vorschlag liefert
- `howlinux "what does chmod 755 mean"` den passenden Explain-Eintrag bevorzugt
- Synonyme aus `concepts.yaml` ohne Codeänderung wirken
- ein neuer gültiger Entry nur durch neue YAML/Markdown-Dateien hinzugefügt werden kann
- kaputte einzelne Entries verständliche Warnungen erzeugen und andere Entries weiter funktionieren
- Ranking mit `--explain` nachvollziehbar ist
- keine Runtime-Netzwerkverbindung und keine Kommandoausführung stattfindet
- Dokumentation erklärt, wie der Betreiber ausschließlich durch neue Knowledge-Einträge Wissen erweitert

## 17. Hinweise für den Implementierungs-Agenten

Arbeite inkrementell und bewahre die vorhandenen Einträge. Beginne nicht mit Embeddings oder einem eigenen Sprachmodell. Halte Loader, Query-Verarbeitung, Index, Ranking, CLI und Renderer unabhängig testbar. Verwende strukturierte YAML-APIs statt eigener Stringparser. Jede neue Entscheidung, die das Datenformat oder die CLI verändert, muss in README und Tests sichtbar werden.
