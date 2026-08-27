# Post-V1-Roadmap

Dieses Dokument sammelt mögliche Erweiterungen nach der stabilen V1. Es ist
eine priorisierte technische Roadmap, keine Zusage für einen bestimmten
Release. Ein Feature wird erst eingeplant, wenn Nutzen, Wartungsaufwand,
Offline-Verhalten und messbare Abnahmekriterien geklärt sind.

Die V1 bleibt die Referenz:

- lokal und ohne Runtime-Netzwerk;
- deterministisch und ressourcenschonend;
- redaktionell geprüfte Antworten statt Textgenerierung;
- keine Ausführung angezeigter Befehle;
- YAML und Markdown bleiben die autoritative Knowledge-Quelle;
- lexikalische Suche funktioniert ohne optionale Zusatzkomponenten.

## Prioritätsmodell

- **P1:** naheliegende nächste Arbeit mit hohem praktischem Nutzen;
- **P2:** mittelfristige Erweiterung nach belastbaren V1-Erfahrungen;
- **P3:** experimentell; nur hinter klaren Schnittstellen und nie als
  Voraussetzung für die Basissuche.

## P1 – Naheliegende Erweiterungen

### 1. Persistenter, versionierter Index

Der persistente Index gehört ausdrücklich **nicht zur V1**. Er ist der erste
naheliegende Skalierungsschritt, wenn Startzeit oder Knowledge-Größe dies
rechtfertigen.

Geplanter Umfang:

- separates Programm howlinux-index oder eine klar getrennte
  Compiler-Komponente;
- Aufruf nach dem Muster
  howlinux-index knowledge/ -o howlinux.idx;
- Runtime-Nutzung über howlinux --index howlinux.idx "query";
- exakt derselbe Loader und dieselbe Validierung wie für YAML/Markdown;
- dokumentierte Formatversion;
- Knowledge-Signatur, Hashes oder ausreichend genaue Änderungsinformationen;
- Entries und Inhalte beziehungsweise sichere Referenzen darauf;
- Inverted Index, Concept-Daten und IDF-Statistiken;
- atomare Erstellung, damit ein abgebrochener Build keinen gültigen Index
  überschreibt;
- klare Ablehnung inkompatibler oder veralteter Indizes.

Abnahmekriterien:

- Ein Index liefert dieselben Suchresultate und Tie-Breaks wie der
  In-Memory-Aufbau derselben Knowledge Base.
- Ein veralteter oder beschädigter Index führt nie still zu falschen
  Antworten.
- Ohne Index bleibt die normale V1-Suche vollständig funktionsfähig.
- Formatmigration und Fehlermeldungen sind getestet und dokumentiert.

### 2. Distributions- und Installationsausbau

Die V1 enthält bereits CMake-Installationsregeln, einen relocatable
`share/howlinux/knowledge`-Fallback und die explizite Option `--knowledge`.
Mögliche nächste Schritte sind:

- XDG-kompatible Benutzer- und Systempfade mit dokumentierter Priorität;
- Debian-Paket und reproduzierbares Release-Archiv;
- Manpage und Shell-Completions für Bash, Zsh und Fish;
- getrennte, versionierte Knowledge-Pakete mit Upgrade-/Deinstallations-Doku.

Die explizite Option `--knowledge` muss weiterhin jede automatische
Pfadauflösung übersteuern.

### 3. Authoring- und Lint-Werkzeuge

Die manuelle Pflege soll sicherer werden, ohne Inhalte automatisch zu
erfinden.

Mögliche Funktionen:

- howlinux init-entry zum Erzeugen einer minimalen Vorlage;
- strikter Validator-Modus für CI;
- versionierte JSON-Schema-Datei für die bereits vorhandenen Payloads;
- CI-Ausgabe und Filter für die bereits vorhandenen Validator-Diagnosen;
- Erkennung doppelter oder sehr ähnlicher Aliase;
- Hinweise auf zu allgemeine Keywords und unbenutzte Concepts;
- weitergehende Lints für Alias-Ähnlichkeit, ungenutzte Concepts und
  redaktionelle ID-Konventionen (die Basisprüfung ist bereits V1);
- optionaler Markdown-Link- und Codeblock-Check;
- übersichtlicher Diff-Bericht über Ranking-Auswirkungen einer Änderung.

Tools dürfen redaktionelle Entscheidungen unterstützen, aber keine
fachlichen Inhalte ungeprüft erzeugen.

### 4. Reproduzierbare Ranking-Evaluation

Vor weiterer Ranking-Komplexität sollte ein kuratierter Evaluationskorpus
entstehen.

Geplanter Umfang:

- versionierte Queries mit erwarteten Top-Ergebnissen;
- getrennte positive, mehrdeutige und negative Fälle;
- Kennzahlen wie Top-1-Genauigkeit, Recall@5 und Quote falscher sicherer
  Treffer;
- Performance-Datensatz mit mindestens 10.000 Entries;
- Vergleichsberichte für Änderungen an Normalisierung, Concepts und Scores;
- kalibrierte Safe- und Margin-Schwellen anhand der Testdaten.

Die Tests bleiben offline und enthalten keine aus Telemetrie gewonnenen
Nutzerdaten.

## P2 – Mittelfristige Erweiterungen

### 1. Versionierte Knowledge-Pakete

Binary und Knowledge können getrennte Releasezyklen erhalten:

- Manifest mit Paketversion, Sprache, unterstützten Plattformen und
  Mindestversion der Engine;
- deterministische Paketarchive;
- Integritätsprüfung und optional signierte Releases;
- expliziter, separater Download-/Updatevorgang;
- parallele lokale Pakete, auswählbar über CLI oder Konfiguration.

Die Runtime selbst bleibt offline. Ein Updatewerkzeug darf niemals unbemerkt
Netzwerkzugriffe ausführen.

### 2. Präzisere lexikalische Suche

Ausbau der bestehenden Suche, bevor komplexere Verfahren nötig werden:

- konfigurierbare Feldgewichte;
- optionale explizite Keyword-Gewichte im YAML;
- feldbezogene BM25-Varianten;
- bessere Behandlung zusammengesetzter Wörter und Unicode;
- sprachspezifische Stopwords und vorsichtiges Stemming;
- section-aware Indexierung ausgewählter Markdown-Inhalte;
- begrenzte, nachvollziehbare Textausschnitte in Vorschlägen.

Alle neuen Teil-Scores müssen über --explain sichtbar und deterministisch
bleiben. Binärdateien werden weiterhin nicht indexiert.

### 3. Mehrsprachige, redaktionell gepflegte Knowledge Base

Möglicher Aufbau:

- Sprache im Paketmanifest oder Entry;
- getrennte, von Menschen geprüfte Inhalte pro Sprache;
- sprachabhängige Normalisierung, Stopwords und Concepts;
- definierte Fallback-Sprache;
- Validierung auf fehlende oder veraltete Übersetzungen.

Automatische Übersetzung darf höchstens einen redaktionellen Entwurf liefern.
Ungeprüfter maschineller Text darf nicht als autoritative Antwort erscheinen.

### 4. Wiederverwendbare Library und weitere Oberflächen

Loader, Query Processor, Index und Ranker könnten als stabile C++-Library
bereitgestellt werden. Darauf aufbauend wären möglich:

- interaktiver Terminalmodus zur Auswahl unsicherer Treffer;
- Read-only-TUI;
- lokale Editorintegration;
- dokumentierte API für Distributionstools;
- Renderer für weitere strukturierte Ausgabeformate.

Keine Oberfläche darf Shell-Befehle ohne einen separaten, ausdrücklich
bestätigten Sicherheitsentwurf ausführen. Die Standardfunktion bleibt reine
Anzeige.

### 5. Sicherer lokaler Reload

Für sehr große oder häufig aktualisierte Knowledge Bases:

- Änderungserkennung ohne vollständigen Prozessneustart;
- inkrementeller Indexaufbau;
- konsistente Snapshots während eines Reloads;
- Rückfall auf den letzten gültigen Zustand bei Validierungsfehlern;
- messbare Speicher- und Latenzgrenzen.

Ein teilweise geladener Datenbestand darf niemals sichtbar werden.

## P3 – Experimentelle Erweiterungen

### 1. Austauschbare Embedding-Rankingstufe

Embeddings sind optional, experimentell und **niemals Voraussetzung** für
howlinux. Sie dürfen nur als austauschbare Rankingstufe hinter einer stabilen
Schnittstelle ergänzt werden.

Leitplanken:

- lexikalischer Inverted Index und Fuzzy-Fallback bleiben verfügbar;
- Aktivierung ausschließlich explizit per Build-/Konfigurationsoption;
- bevorzugt lokales, offline nutzbares Modell;
- keine Cloud-API und keine zwingende externe Vector Database;
- Modellname, Version und Vektordimension sind Teil der Indexmetadaten;
- ein fehlendes Modell führt kontrolliert zur lexikalischen Suche zurück;
- semantische Teil-Scores erscheinen in --explain;
- exakte Command-, Flag- und Alias-Treffer dürfen nicht von einem diffusen
  semantischen Match verdrängt werden;
- Evaluation misst besonders falsche sichere Treffer und Ressourcenverbrauch.

Embeddings wählen weiterhin nur geprüfte Entries aus. Sie generieren oder
verändern keinen Antworttext.

### 2. Gelerntes hybrides Ranking

Ein lernender Ranker könnte lexikalische, Concept-, Intent- und optionale
semantische Signale kombinieren. Voraussetzung sind:

- ausreichend großer, kuratierter und versionierter Evaluationsdatensatz;
- reproduzierbares Training ohne Nutzerdaten-Telemetrie;
- exportiertes, versioniertes Modell;
- verständliche Teil-Scores und ein deterministischer Fallback;
- nachweisbarer Vorteil gegenüber der konfigurierten V1-Heuristik.

Ohne messbaren Qualitätsgewinn bleibt der regelbasierte Ranker maßgeblich.

### 3. Knowledge-Graph für verwandte Themen

related und Concepts könnten zu einem validierten Graphen ausgebaut werden:

- gerichtete und typisierte Beziehungen;
- Prüfung auf verwaiste Knoten und unerwünschte Zyklen;
- Vorschläge für benachbarte Themen;
- Navigation in einer TUI oder strukturierten Ausgabe;
- begrenzter Ranking-Bonus für fachlich nahe Entries.

Graphbeziehungen dürfen einen inhaltlich unpassenden Treffer nicht zu einer
sicheren Antwort machen.

### 4. Lokale, datenschutzfreundliche Qualitätsmessung

Als Experiment denkbar ist ein ausschließlich lokaler Diagnosemodus, der
anonyme Aggregationen für den Betreiber erzeugt, etwa häufige No-Match-Queries.

Voraussetzungen:

- standardmäßig deaktiviert;
- kein Netzwerk und keine automatische Übertragung;
- klare Speicherorte und Löschfunktion;
- Redaction potenziell sensibler Query-Bestandteile;
- ausdrückliche Dokumentation und Einwilligung.

Ohne überzeugendes Datenschutzkonzept wird dieses Feature nicht umgesetzt.

## Bewusst außerhalb der Roadmap

Folgende Änderungen würden den Kernvertrag von howlinux verletzen und sind
nicht geplant:

- ungeprüfte generative Antworten als Standardverhalten;
- automatische Ausführung angezeigter Shell-Befehle;
- Ausführung von Query- oder YAML-Inhalten;
- verpflichtende Cloud-Dienste oder Runtime-Netzwerkzugriffe;
- versteckte Telemetrie;
- eine externe Vector Database als Voraussetzung;
- Ersetzung der YAML-/Markdown-Quellen durch ein undurchsichtiges Binärformat.

Jede spätere Erweiterung muss weiterhin zeigen, dass ein neuer geprüfter
Knowledge-Eintrag ohne C++-Änderung ergänzt werden kann.
