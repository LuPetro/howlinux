# TODO – howlinux

## Offene Release-Schritte auf GitHub

- [ ] V1-Änderungen nach `master` mergen oder pushen und alle CI-Jobs abwarten.
- [ ] Branch-Protection für `master` und private Security-Meldungen aktivieren.
- [ ] Repository-Beschreibung und Topics (`linux`, `cli`, `cpp`, `offline`) setzen.
- [ ] Nach grüner CI den Tag `v1.0.0` erstellen; der Release-Workflow veröffentlicht Archiv und Prüfsumme.

## Technische V1 abschließen

- [x] Bash-/WSL-Umgebung reparieren und prüfen, dass `howlinux` aus Bash, WSL und PowerShell gestartet werden kann.
- [x] Fehler in `/home/lukam/.profile` beheben: `DOTNET_ROOT` korrekt setzen.
- [x] Profil mit `bash -n ~/.profile` prüfen und anschließend neu laden.
- [x] Clean-Install in ein leeres Verzeichnis testen.
- [x] Installiertes Binary ohne Repository-Arbeitsverzeichnis starten.
- [x] Automatisierte Smoke-Tests für die wichtigsten CLI-Aufrufe ergänzen.
- [x] Tests für ANSI-freie JSON-Ausgabe ergänzen.
- [x] Tests für die wichtigsten Exitcodes ergänzen.

## Plattformen und Release

- [x] CI-Pipeline für Configure, Build und CTest einrichten.
- [x] CI auf mindestens Ubuntu/Debian testen.
- [x] Debug- und Release-Build in CI prüfen.
- [x] Installation und Start des installierten Binaries in CI prüfen.
- [x] Reproduzierbares Release-Archiv erstellen.
- [x] Release-Struktur mit `bin/howlinux`, `share/howlinux/knowledge` und Dokumentation prüfen.
- [x] Release-Version auf `1.0.0` festlegen.
- [x] Installations- und Deinstallationsanleitung dokumentieren.
- [x] Debian-Paketierung für V1 als nicht erforderlich einstufen; das TGZ-Archiv ist der erste Distributionsweg.

## Bedienung und Distribution

- [x] Bash-Completion ergänzen.
- [x] Zsh-Completion ergänzen.
- [x] Fish-Completion ergänzen.
- [x] Manpage ergänzen.
- [x] Separates `release.sh` für V1 als nicht erforderlich einstufen; der getaggte GitHub-Workflow übernimmt den Release.
- [x] Dokumentieren, wie `--knowledge` und `HOWLINUX_KNOWLEDGE` verwendet werden.
- [x] Dokumentieren, wie das Programm aus Bash, WSL und PowerShell gestartet wird.

## Knowledge-Qualität und spätere Werkzeuge

Die offenen Punkte in diesem Abschnitt sind Verbesserungen nach V1 und keine
Blocker für den ersten Release.

- [x] Knowledge-Validierung mit `howlinux validate` implementieren und über die Smoke-Tests in CI ausführen.
- [ ] Doppelte oder sehr ähnliche Aliase erkennen.
- [ ] Zu allgemeine Keywords erkennen.
- [ ] Unbenutzte Concepts erkennen.
- [x] Veraltete oder fehlende `related`-Referenzen beim Laden prüfen.
- [ ] Markdown-Link- und Codeblock-Prüfung optional ergänzen.
- [ ] Ranking-Evaluation mit erwarteten Top-Ergebnissen aufbauen.
- [ ] Positive, mehrdeutige und negative Queries aufnehmen.
- [ ] Top-1-Genauigkeit und Recall@5 messen.
- [ ] Falsche sichere Treffer messen.
- [ ] Safe- und Margin-Schwellen anhand der Evaluation kalibrieren.

## Knowledge-Base ausbauen

- [ ] Mehr Knowledge-Entries für wichtige Linux-Kommandos schreiben.
- [ ] Wichtige Flags und typische Varianten abdecken.
- [ ] Pro Entry mindestens drei realistische Aliase ergänzen.
- [ ] Fachlich starke Keywords pflegen.
- [ ] Englische und deutsche Suchformulierungen abdecken, sofern gewünscht.
- [ ] Inhalte fachlich prüfen und in einer sicheren Testumgebung verifizieren.
- [ ] Syntax, Voraussetzungen und typische Fehler dokumentieren.
- [ ] Dateinamen mit Leerzeichen korrekt behandeln.
- [ ] Platzhalter in Beispielen deutlich kennzeichnen.
- [ ] Destruktive Befehle unmittelbar und deutlich warnen.
- [ ] `sudo` nicht pauschal empfehlen.
- [ ] Keine Secrets, privaten Hostnamen oder personenbezogenen Daten verwenden.
- [ ] Ähnliche Entries gegeneinander testen.
- [ ] Queries sammeln, bei denen der falsche Entry gewinnt.
- [ ] Concepts und Synonyme laufend erweitern und validieren.
- [ ] `./build/howlinux validate knowledge` nach jeder Knowledge-Änderung ausführen.
- [ ] Repräsentative Suchanfragen mit `--explain` prüfen.

## Nicht erforderlich für V1

- [ ] Persistenter Index erst bei nachweisbarem Performancebedarf planen.
- [ ] Embeddings erst nach einer belastbaren lexikalischen Evaluation prüfen.
- [ ] Datenbank oder externe Vector Database nicht als V1-Abhängigkeit einführen.
- [ ] Keine Textgenerierung oder automatische, ungeprüfte Knowledge-Erstellung einbauen.
