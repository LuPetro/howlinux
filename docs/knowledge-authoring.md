# Knowledge für howlinux pflegen

Diese Anleitung beschreibt den täglichen Ablauf zum Anlegen, Prüfen und
Verbessern von Knowledge-Einträgen. Ein neuer Eintrag besteht ausschließlich
aus YAML- und Markdown-Dateien. Eine Änderung am C++-Code oder ein erneuter
Build ist nicht erforderlich.

howlinux generiert keine Antworttexte. Das Programm sucht den passendsten
geprüften Eintrag und gibt dessen Inhalt aus. Deshalb sind fachliche
Korrektheit, realistische Suchformulierungen und eine saubere Validierung
wichtiger als möglichst viel Text.

> **Sicherheitsgrundsatz:** Inhalte dürfen Shell-Befehle erklären und zeigen,
> aber howlinux führt sie niemals aus. Destruktive Befehle müssen im Text
> deutlich gekennzeichnet werden.

## 1. Aufbau der Knowledge Base

Das Standardverzeichnis heißt knowledge. Jeder Eintrag liegt in einer
Kategorie und besitzt genau zwei Dateien:

~~~text
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
~~~

- commands ist für Referenzen zu einzelnen Kommandos gedacht.
- topics ist für Anleitungen und erklärende Themen gedacht.
- Weitere Kategorien sind technisch möglich. Neue Kategorien sollten jedoch
  nur mit einer bewusst festgelegten Taxonomie eingeführt werden.
- Die Entry-ID muss in der gesamten Knowledge Base eindeutig sein.
- Der Verzeichnisname und das Feld id sollten identisch sein.
- concepts.yaml enthält globale Synonyme und ist kein Entry.

Ein anderes Knowledge-Verzeichnis wird über --knowledge ausgewählt:

~~~bash
./build/howlinux --knowledge /pfad/zur/knowledge "rename a directory"
./build/howlinux validate /pfad/zur/knowledge
~~~

## 2. Einen Eintrag anlegen

### Schritt 1: Kategorie und ID wählen

Wähle eine kurze, dauerhafte und sprechende ID. Empfohlen sind
ASCII-Kleinbuchstaben, Zahlen und Bindestriche:

~~~text
rename-folder
chmod-755
extract-tar
~~~

Vermeide wechselnde Versionsnummern, Marketingbegriffe und unnötige
Abkürzungen. Andere Einträge können die ID über related referenzieren; eine
spätere Umbenennung ist deshalb eine inkompatible Inhaltsänderung.

Lege anschließend das Verzeichnis an, zum Beispiel:

~~~bash
mkdir -p knowledge/topics/rename-folder
~~~

### Schritt 2: meta.yaml schreiben

Ein praxistauglicher Eintrag sieht so aus:

~~~yaml
id: rename-folder
title: Rename a folder
type: howto
command: mv

aliases:
  - rename a folder
  - change the name of a directory
  - give a directory a different name

keywords:
  - rename
  - folder
  - directory
  - mv

related:
  - mv

intent:
  - how_to

difficulty: beginner
platforms:
  - linux
  - ubuntu
tags:
  - filesystem
examples:
  - mv -- old-name new-name
~~~

### Felder im Überblick

| Feld | Status | Format | Bedeutung |
| --- | --- | --- | --- |
| id | erforderlich | String | Knowledge-weit eindeutige, stabile ID |
| title | erforderlich | String | Kurzer, verständlicher Ausgabetitel |
| type | erforderlich | String | Üblicherweise command oder howto |
| command | optional | String | Hauptkommando des Eintrags, sofern vorhanden |
| aliases | optional, dringend empfohlen | Liste von Strings | Reale Suchformulierungen und feste Phrasen |
| keywords | optional, empfohlen | Liste von Strings | Fachlich starke Suchbegriffe, Flags und Kommandos |
| related | optional | Liste von IDs | Verweise auf vorhandene, verwandte Entries |
| intent | optional | Liste von Strings | Passende Query-Arten, etwa how_to oder explain |
| difficulty | optional | String | Beispielsweise beginner, intermediate oder advanced |
| platforms | optional | Liste von Strings | Getestete oder relevante Plattformen |
| tags | optional | Liste von Strings | Redaktionelle Gruppierung |
| examples | optional | Liste von Strings | Kurze Beispielbefehle für Metadaten/Tools |

Fehlende optionale Listen werden wie leere Listen behandelt. Bestehende
Entries ohne intent bleiben gültig. Verwende keine frei erfundenen Felder:
Unbekannte Felder werden bei der Validierung als Warnung gemeldet, bleiben
aber für die Suche folgenlos.

YAML-Hinweise:

- Speichere meta.yaml und content.md als UTF-8; content.md muss eine reguläre,
  nicht leere Datei sein. Symlinks werden vom Loader aus Sicherheitsgründen
  abgelehnt.
- Verwende Leerzeichen, keine Tabs.
- Werte mit Doppelpunkten, führenden Sonderzeichen oder YAML-ähnlichen
  Wahrheitswerten sollten in Anführungszeichen stehen.
- aliases, keywords, related, intent, platforms, tags und examples sind Listen.
- Eine related-Angabe enthält Entry-IDs, keine Titel und keine Dateipfade.
- Wiederhole denselben Alias oder dasselbe Keyword nicht künstlich. Doppelte
  Tokens verbessern das Ranking nicht kontrolliert.

### Schritt 3: content.md schreiben

content.md ist die geprüfte Antwort. Die Datei darf Überschriften, Listen,
Inline-Code und Shell-Codeblöcke enthalten. Ihr fachlicher Inhalt wird beim
Rendern nicht umgeschrieben.

Eine bewährte Reihenfolge ist:

1. kurze Erklärung des Kommandos oder Konzepts;
2. allgemeine Syntax;
3. ein einfaches, geprüftes Beispiel;
4. wichtige Varianten oder Flags;
5. typische Fehler, Voraussetzungen und Sicherheitswarnungen;
6. verwandte Entries.

Beispiel:

~~~~markdown
# Rename a folder

The mv command can rename a folder by moving it to a new path.

## Syntax

~~~bash
mv -- OLD_NAME NEW_NAME
~~~

## Example

~~~bash
mv -- "old project" "new project"
~~~

Existing destination paths can change the behavior of mv. Check the target
before running the command.

## Related

- mv
~~~~

Beachte beim Schreiben:

- Prüfe jeden Befehl in einer sicheren Testumgebung.
- Kennzeichne Platzhalter deutlich, zum Beispiel OLD_NAME und NEW_NAME.
- Quote Dateinamen mit Leerzeichen korrekt.
- Erkläre notwendige Rechte, Distributionen, Versionen oder installierte
  Pakete.
- Markiere irreversible oder destruktive Auswirkungen unmittelbar vor dem
  betreffenden Befehl.
- Empfiehl sudo nicht pauschal.
- Kopiere keine ungeprüften Einzeiler aus fremden Quellen.
- Füge keine Geheimnisse, echten Tokens, privaten Hostnamen oder
  personenbezogenen Daten in Beispiele ein.
- Metadaten unter examples ersetzen keine verständlichen Beispiele in
  content.md.

## 3. Gute Aliase schreiben

Aliase sind Formulierungen, die Menschen tatsächlich als Query eingeben. Jeder
Entry sollte mindestens drei unterschiedliche, realistische Formulierungen
abdecken.

Gut:

~~~yaml
aliases:
  - rename a folder
  - change the name of a directory
  - give a directory a different name
~~~

Schlecht:

~~~yaml
aliases:
  - linux help
  - help me
  - folder
  - rename folder
  - rename the folder
  - rename a folder please
~~~

Vermeide:

- sehr allgemeine Phrasen, die zu vielen Entries passen;
- reine Stopword-Varianten;
- zehn nahezu identische Sätze;
- globale Synonyme, die besser in concepts.yaml gepflegt werden;
- Aliase für Sachverhalte, die der Entry inhaltlich nicht beantwortet.

Ein exakter Alias ist ein besonders starkes Ranking-Signal. Ein zu allgemeiner
Alias kann deshalb korrekte Entries verdrängen.

## 4. Gute Keywords wählen

Keywords sind kurze, fachlich wichtige Begriffe. Geeignet sind:

- das Hauptverb oder die Hauptaktion;
- das betroffene Objekt;
- das Linux-Kommando;
- relevante Flags, Dateiformate oder Zahlen;
- etablierte Fachbegriffe.

Beispiel:

~~~yaml
keywords:
  - extract
  - archive
  - tar
  - tar.gz
~~~

Nicht geeignet sind vollständige Fragen, Stopwords und Begriffe wie help,
linux oder command, wenn sie den Entry nicht von anderen Entries unterscheiden.

## 5. Intent und Typ sinnvoll verwenden

type beschreibt die Art des Knowledge-Eintrags. Bestehende und übliche Werte
sind:

- command für eine Kommandoreferenz;
- howto für eine konkrete Anleitung oder Erklärung.

intent beschreibt, für welche Query-Art der Entry besonders passend ist. Die
gängigen Werte entsprechen den erkannten Query-Typen in Kleinschreibung:

- explain
- how_to
- why
- command
- general

Intent ist ein Ranking-Signal und kein Filter. Trage nur Werte ein, die der
Inhalt wirklich beantwortet. Ein fehlendes intent ist zulässig.

## 6. Globale Concepts und Synonyme

Global gültige Synonyme gehören nach knowledge/concepts.yaml:

~~~yaml
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
~~~

Der Schlüssel einer Gruppe ist der kanonische Begriff. Die Liste darf einzelne
Wörter und Mehrwort-Phrasen enthalten. Der kanonische Begriff sollte der
Klarheit halber ebenfalls in seiner Liste stehen.

Regeln:

- Ein Ausdruck darf nicht mehreren Concept-Gruppen zugeordnet werden.
- Kanonische Namen müssen eindeutig sein.
- Verwende keine Gruppen, die sich gegenseitig als Synonym definieren.
- Fasse nur Begriffe zusammen, die im Linux-Kontext tatsächlich austauschbar
  sind.
- Spezifische Formulierungen bleiben lokale Aliase; nur global gültige
  Synonyme gehören in concepts.yaml.
- Eine fehlende concepts.yaml ist erlaubt, reduziert aber die
  Synonymerkennung.

Concepts wirken auf Query und Entry-Metadaten. Die Originalbegriffe bleiben
trotzdem erhalten. Eine Änderung an concepts.yaml kann daher das Ranking
mehrerer Entries beeinflussen und muss mit mehreren Queries geprüft werden.

## 7. Validieren und Suchverhalten prüfen

### Gesamte Knowledge Base validieren

~~~bash
./build/howlinux validate knowledge
echo $?
~~~

Oder für einen anderen Pfad:

~~~bash
./build/howlinux validate /pfad/zur/knowledge
~~~

Die Validierung sollte ohne Warnungen abgeschlossen werden. Sie prüft unter
anderem fehlende Dateien, ungültiges YAML, Feldtypen, doppelte IDs und
fehlerhafte Concept-Definitionen. Ein kaputter Entry wird von der Runtime
übersprungen, darf aber nicht bewusst in der Knowledge Base verbleiben.

### Mehrere Query-Arten testen

Prüfe mindestens:

1. einen direkten Alias;
2. eine natürlich formulierte Frage;
3. eine Formulierung mit globalem Synonym;
4. das Hauptkommando oder ein wichtiges Flag;
5. einen plausiblen Tippfehler;
6. eine irrelevante Query als Negativtest.

~~~bash
./build/howlinux "rename folder"
./build/howlinux "how can i change the name of a directory"
./build/howlinux "mv"
./build/howlinux --explain "change the directory name"
./build/howlinux --json "rename a folder"
~~~

--explain zeigt Alias-, Phrase-, Token-/IDF-, Concept-, Command-, Keyword-,
Titel-, Intent- und Fuzzy-Anteile sowie die konkreten Matchgründe. Nutze diese Ausgabe, um
Ursachen zu korrigieren; erhöhe nicht wahllos die Zahl ähnlicher Aliase.

### Queries mit führenden Bindestrichen

-- beendet die Auswertung von CLI-Optionen. Alle folgenden Argumente gehören
zur Query. Das ist besonders bei Flags erforderlich:

~~~bash
./build/howlinux -- "--recursive"
./build/howlinux --knowledge knowledge -- "what does --recursive mean"
~~~

Ohne den Trenner könnte ein Query-Token wie --recursive als Option der
Anwendung interpretiert werden.

### Exitcodes

| Code | Bedeutung |
| --- | --- |
| 0 | Sicherer Treffer oder erfolgreich ausgeführter Verwaltungsbefehl |
| 1 | Unsicherer Treffer, kein Treffer oder festgestellte Validierungsprobleme |
| 2 | Ungültige CLI-Argumente oder Optionen |
| 3 | Knowledge-Verzeichnis nicht lesbar oder Konfigurationsfehler |

Bei validate bedeutet Code 0 eine erfolgreiche Validierung, Code 1 mindestens
ein übersprungenes/strukturell fehlerhaftes Entry oder eine Warnung und Code 3
ein fehlendes Knowledge-Root oder eine unbrauchbare globale
Concept-Konfiguration. Bei einer normalen Suche bedeutet Code 1, dass Skripte
die Ausgabe nicht als sicheren Treffer behandeln dürfen. In `validate --json`
stehen Diagnosen im Feld `diagnostics`; Suchdiagnosen bleiben auf `stderr`.

## 8. Review-Checkliste

Vor dem Merge eines Entries:

- [ ] Ist die Aussage unter Linux fachlich korrekt?
- [ ] Wurden alle Beispiele sicher getestet oder ihre Voraussetzungen erklärt?
- [ ] Sind Distribution-, Versions- und Paketabhängigkeiten genannt?
- [ ] Sind Dateinamen mit Leerzeichen korrekt quotiert?
- [ ] Sind destruktive oder irreversible Auswirkungen deutlich markiert?
- [ ] Gibt es mindestens drei realistische und unterschiedliche Aliase?
- [ ] Sind Keywords knapp, fachlich stark und frei von Stopwords?
- [ ] Sind id, Verzeichnisname, command und related korrekt geschrieben?
- [ ] Verweisen alle related-Werte auf vorhandene Entry-IDs?
- [ ] Werden globale Synonyme in concepts.yaml statt mehrfach lokal gepflegt?
- [ ] Beantwortet content.md genau die durch Aliase und intent versprochenen
      Fragen?
- [ ] Liefert validate keine Warnungen oder Fehler?
- [ ] Funktionieren direkter Alias, natürliche Frage und Synonymformulierung?
- [ ] Verdrängt der Entry bei Negativtests keinen fachlich besseren Treffer?
- [ ] Enthalten Beispiele keine Zugangsdaten oder privaten Informationen?

## 9. Häufige Probleme

### Der Entry wird nicht geladen

Prüfe:

- Liegen meta.yaml und content.md im selben Entry-Verzeichnis?
- Ist das YAML syntaktisch korrekt und sind Listen wirklich Listen?
- Ist die ID global eindeutig?
- Stimmen Verzeichnisname und id überein?
- Zeigt --knowledge auf das erwartete Root-Verzeichnis?

Führe danach validate aus und beachte Pfad, ID und Ursache der Warnung.

### Der Entry wird nicht gefunden

- Ergänze realistische Aliase, nicht bloß weitere Einzelwörter.
- Prüfe, ob zentrale Objekt- und Aktionsbegriffe als Keywords vorhanden sind.
- Pflege global gültige Synonyme in concepts.yaml.
- Prüfe die normalisierte Query und die Teil-Scores mit --explain.
- Kontrolliere, ob die Query ausschließlich aus Stopwords besteht.

### Ein falscher Entry gewinnt

- Entferne zu allgemeine oder sachlich falsche Aliase.
- Prüfe Concept-Gruppen auf zu weit gefasste Synonyme.
- Verwende spezifischere Keywords.
- Prüfe type und intent.
- Vergleiche die Teil-Scores beider Entries mit --explain.

### Ein Flag wird als Programmoption gelesen

Setze -- vor die Query:

~~~bash
./build/howlinux -- "--help"
~~~

Damit wird nach dem Linux-Flag --help gesucht, statt die Hilfe von howlinux
aufzurufen.
