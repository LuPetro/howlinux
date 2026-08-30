# TODO – howlinux

## Technische V1 abschließen

- [ ] Bash-/WSL-Umgebung reparieren und prüfen, dass `howlinux` aus Bash, WSL und PowerShell gestartet werden kann.
- [ ] Fehler in `/home/lukam/.profile` beheben: `DOTNET_ROOT` korrekt setzen.
- [ ] Profil mit `bash -n ~/.profile` prüfen und anschließend neu laden.
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
- [ ] Release-Version auf `1.0.0` festlegen und Git-Tag erstellen.
- [x] Installations- und Deinstallationsanleitung dokumentieren.
- [ ] Prüfen, ob eine Debian-Paketierung benötigt wird.

## Bedienung und Distribution

- [x] Bash-Completion ergänzen.
- [x] Zsh-Completion ergänzen.
- [x] Fish-Completion ergänzen.
- [x] Manpage ergänzen.
- [ ] Optionales Release-Skript wie `release.sh` ergänzen.
- [x] Dokumentieren, wie `--knowledge` und `HOWLINUX_KNOWLEDGE` verwendet werden.
- [x] Dokumentieren, wie das Programm aus Bash, WSL und PowerShell gestartet wird.

## Knowledge-Qualität und Werkzeuge

- [ ] Knowledge-Linter für CI vorbereiten.
- [ ] Doppelte oder sehr ähnliche Aliase erkennen.
- [ ] Zu allgemeine Keywords erkennen.
- [ ] Unbenutzte Concepts erkennen.
- [ ] Veraltete oder fehlende `related`-Referenzen prüfen.
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
