# UMBRA firmware — gjeldende oversikt 24. oktober 2026

Dette dokumentet erstatter tidligere statusoversikter fra oppryddingen. Prosjektene nedenfor er separate: RNode er radio/modemfirmware, Transport Node er QD-applikasjonen, og microReticulum er protokollbiblioteket QD bruker.

## Gjeldende stabile og installerte referanser

| Prosjekt / GitHub-fork | Hovedgren | Eksakt kildecommit | Varig referanse |
|---|---|---|---|
| [UMBRA RNode Firmware](https://github.com/JAR4x4/UMBRA_RNode_Firmware) | main | fbfb168bab5f502e5f24c670a4928cfdffb842a2 | umbra-rnode-current-stable-2026-10-24 |
| [UMBRA Transport Node](https://github.com/JAR4x4/UMBRA_Transport_Node) | main | 9b67cec70aa5c3de36348201ad08247d9d9a75d1 | qd-installed-2026-10-24 |
| [microReticulum](https://github.com/JAR4x4/microReticulum) | master | 9ce2fbaaa1335b2d8f16050195233cd6ee414c86 | qd-installed-2026-10-24 |

Hovedgrenene inneholder samme firmwarekode som referansene, med senere dokumentasjonscommits. HEAD alene identifiserer derfor ikke installerte bytes. Ingen nye firmwareversjoner er bygget eller flashet ved denne dokumentasjonsoppdateringen.

RNode fbfb168 er bekreftet av eier som stabil i over en måned. Historisk merke xiao-rnode-1.86-battery-stable-2026-08-09 er beholdt. Denne opplysningen identifiserer ikke automatisk firmware på hver enkelt radio; ingen enheter er avlest under oppryddingen.

QD ble oppdatert 24. oktober ca.02:35 med bevart identitet og innstillinger. Miljøet umbra_xiao_nrf52840_lockup_diag har biblioteket låst til full commit9ce2fbaaa1335b2d8f16050195233cd6ee414c86. Andre byggemiljøer kan ha andre avhengigheter og er ikke samme installasjonsreferanse.

## Verifiserte pakker og tester

Pakker er bevart lokalt under /home/jar/UMBRAreticulum/firmware/artifacts/; rå USB- og EEPROM-logger publiseres ikke.

| Pakke | Lokal undermappe | SHA256 for original ZIP |
|---|---|---|
| Stabil RNode fbfb168 | rnode-stable-fbfb168/RNode_Firmware.ino.zip | 64daa70bd8bf318f25032be3ac947b66d8c07b24989b996e5e560286ea95d1b6 |
| Installert QD | qd-current-9ce2fba/UMBRA-QD-rapid-response-admission-unflashed.zip | ee8b7e7d9e5ceb914770e633b837431e2a6386f3876cc20007e6c5f3fb93ce0b |

RNode-pakken ble sammenlignet med en isolert nybygging av eksakt fbfb168 med installert verktøykjede: HEX og alle ZIP-medlemmer (BIN, DAT, manifest) var identiske. ZIP-arkivets egen hash kan avvike på grunn av metadata. BIN-SHA256 er33e3432e5ada170686392a274b1fd384664135b57df790222a4439ec09dc31d0. Byggingen lastet ikke opp noe. Verifikasjonsrapport ligger sammen med den bevarte pakken og i RNode-repoets docs/rnode/2026-10-24/.

QD-pakkens filnavn «unflashed» og opprinnelige manifeststatus er historiske fra før opplasting; de endres ikke retrospektivt. Tidligere QD-pakke d3b0d31 og septemberpakken er bevarte tilbakeføringsreferanser, ikke gjeldende firmware.

To cachefrie NomadNet-stressrunder og nattlogging har vist opprydding av ekstra forbindelser og tilbakegang til854/180aktive allokeringer. Siste runde09:05:8856→6032→8844byte ledig pool,1→3→1forbindelser,ingen observerte bad_alloc/ContainerAllocator-feil. 18UIhandlinger er ikke18bekreftede overføringer. Detaljer finnes i Transport Node docs/qd/2026-10-24/.

## Gjenværende utvikling og referansegrener

- RNode: debug/xiao-deferred-dio1-irq ved8527cb6f0af53e7bc1f93cd05e04e0e2f4afb90b, lokalt og på GitHub. Flytter IRQ-behandling til hovedløkken. Eksperiment, ikke del av stabil fbfb168; ingen automatisk fletteanbefaling.
- Transport Node: port/xiao-nrf52840 ved1bd8a68, lokal historisk gren med fire egne commits. Beholdes; ikke produksjonsreferanse og ikke klar for fletting.
- microReticulum: GitHub-grenene base64,csma,log-opt,path_healing,provision_opt,tcp_announce har egne eksperimentendringer og er beholdt. Ingen av dem er del av installert QD-referanse.

references/RNode_Firmware_upstream (markqvist,d39339f) og references/RNode_Firmware_CE (liberatedsystems,df2cfe1) er separate lokale referansekopier. CE-commitens melding sier «Unresolved merge conflict - fix later»; Git-ren mappe betyr ikke verifisert firmware. Referansekopiene og tredjeparts remote-grener er ikke ryddet bort.

## Arkivmerker og historikk

Alle gamle grentipper er bevart som annoterte GitHub-merker under archive/2026-10-24/. Gamle grennavn er fjernet etter verifisert bevaring. Ingen commits eller firmwarepakker er kastet. Git-bundles og arkivregistre finnes lokalt under archive/2026-10-24/. RNode-debug-grenene er arkivert som unike eksperimenter, ikke fordi de var flettet.

### Transport Node —16arkivmerker

- `archive/2026-10-24/chore/repository-cleanup` → `07400bc`
- `archive/2026-10-24/cleanup/xiao-upstream-logging-baseline` → `a253174`
- `archive/2026-10-24/diagnostic/qd-lockup-2026-09-20` → `b3290ff`
- `archive/2026-10-24/feature/external-flash-backend-activation` → `8fecfa9`
- `archive/2026-10-24/feature/external-flash-transport-integration` → `14f0a78`
- `archive/2026-10-24/feature/microstore-fs-probe` → `7721402`
- `archive/2026-10-24/feature/qspi-phase1-probe` → `0559c44`
- `archive/2026-10-24/feature/spi-storage-baseline` → `3b22772`
- `archive/2026-10-24/feature/storage-component-build-integration` → `068902e`
- `archive/2026-10-24/feature/xiao-nrf52840-battery-monitor` → `8c63d47`
- `archive/2026-10-24/fix/qd-nomadnet-memory-cleanup` → `66de4b0`
- `archive/2026-10-24/fix/qd-rapid-response-admission` → `2c802e0`
- `archive/2026-10-24/fix/xiao-spi-peripheral-assignment` → `cb1e3b1`
- `archive/2026-10-24/master` → `592c826`
- `archive/2026-10-24/port/xiao-debug-good` → `0559c44`
- `archive/2026-10-24/test/xiao-rnode-tcxo-sync-2026-08-07` → `6065069`

### microReticulum —4arkivmerker

- `archive/2026-10-24/baseline/qd-2026-09-20` → `40fa628`
- `archive/2026-10-24/bytes_allocator` → `4a5e72e`
- `archive/2026-10-24/fix/qd-nomadnet-memory-cleanup` → `d3b0d31`
- `archive/2026-10-24/fix/qd-rapid-response-admission` → `9ce2fba`

### RNode —9arkivmerker

- `archive/2026-10-24/debug/xiao-ce-complete-irq-model` → `01303f1`
- `archive/2026-10-24/debug/xiao-ce-infinite-rx-fix` → `3090ab8`
- `archive/2026-10-24/debug/xiao-sx1262-agc-reset` → `dd23bf0`
- `archive/2026-10-24/debug/xiao-tcxo-1v8` → `91f5727`
- `archive/2026-10-24/debug/xiao-tx-rx-recovery` → `7ac0138`
- `archive/2026-10-24/debug/xiao-wio-tcxo-init-order` → `c7f5eee`
- `archive/2026-10-24/feature/xiao-nrf52840-battery-monitor` → `fbfb168`
- `archive/2026-10-24/port/xiao-nrf52840` → `fc3f69b`
- `archive/2026-10-24/test/xiao-1.86-clean-125k-sf7` → `fc3f69b`

RNode-merker for TCXO-forsøk beskriver at rettelsene allerede finnes i stabil kode. IRQ-, AGC- og RX-forsøk er merket som historiske eksperimenter uten tilstrekkelig dokumentert forbedring over stabil referanse. Arkiverte grener kan gjenopprettes fra merket uten å påvirke main.

De fire gamle Transport Node/microReticulum-PR-ene er lukket som erstattet: innholdet ble ført inn i hovedgrenene ved fast-forward. GitHub kan vise dem som «closed» selv om koden er inkludert.

## Kjente begrensninger

QD: registrert canonical BIN-hash og enhetens beregnede hash avviker fortsatt, som før oppdateringen; dette er ikke løst. Overlappende ressursforespørsler kan avvises uten BUSY-pakke og gi klienttimeout. Firmware rapporterer ikke oppetid eller oppstartsårsak. Upload-hooken kan utføre provisioning; fremtidig QD-flashing må fortsatt følge kontrollert applikasjons-DFU og README-hashprosedyre med bevart identitet.

Denne oversikten er dokumentasjon av kildeversjoner, arkiv og observerte tester; ingen garanti for alle belastninger. Dokumentasjonsoppdateringen endrer ikke firmware, biblioteklås, innstillinger eller tjenester.

## Reversibel filopprydding — nye stier

De tre aktive repoene står uendret under /home/jar/UMBRAreticulum/firmware/: RNode_Firmware/, UMBRA_Transport_Node/ og microReticulum/. Relative byggeverktøystier mellom dem er beholdt.

| Tidligere sti under firmware/ | Ny sti under firmware/ | Operasjon |
|---|---|---|
| RNode_Firmware_upstream/ | references/RNode_Firmware_upstream/ | Flyttet, innhold verifisert |
| RNode_Firmware_CE/ | references/RNode_Firmware_CE/ | Flyttet, innhold verifisert |
| analysis/ | evidence/analysis/ | Flyttet |
| xiao-port-analysis/ | evidence/xiao-port-analysis/ | Flyttet |
| build-logs/ | evidence/build-logs/ | Flyttet |
| Løse *.txt-analyser | evidence/legacy-analysis/ | Flyttet |

Relevant test- og firmwaremateriale fra chatens arbeidsmappe er kopiert til evidence/private/2026-10-24/chat-evidence/. Synkroniserte sources/ og Console-kildekode er ikke flyttet eller endret. Kjente midlertidige qd-/umbra-bygg og RNode-reproduksjonsbygget er kopiert fra /tmp til evidence/private/2026-10-24/temporary-snapshots/. Alle disse originalene er beholdt; kopiene er historiske snapshots, ikke aktive byggerepoer. Midlertidige originale symlinker er bevart som lenker, og lenkemål er registrert. Avhengigheter som ligger utenfor snapshotet er ikke automatisk innlemmet.

Private evidence har tilgang begrenset til eieren; rå USB/EEPROM-materiale publiseres ikke på GitHub. Verifiserte flashpakker ligger fortsatt i artifacts/. QD-pakkens historiske «unflashed»-filnavn er beholdt, mens installasjonsstatus er forklart i denne oversikten. Ingen kilde-/firmwarepakker er slettet. Ingen diskplassbesparelse hevdes ved kopiering.

Filregister: archive/2026-10-24/file-structure/file-register.json. Registeret beskriver 88 operasjoner og 13 247 fil-/lenkeoppføringer, med kilde, mål, størrelse og SHA256 for vanlige filer, eller lenkemål for symlinker. Hver destinasjon er kontrollert mot kildens register etter operasjonen. Registeret og råfilene lagres lokalt; denne oversikten er den publiserte oppsummeringen.

Reversering: følg archive/2026-10-24/file-structure/UNDO.md. Flytteoperasjoner reverseres i motsatt rekkefølge etter kontroll av registeret og at gammel sti er ledig. Kopier trenger ingen tilbakeflytting fordi originalene er beholdt. Ingen automatiske overskrivinger eller slettinger inngår. Dokumentasjonscommits er separate fra filflyttingene.

## Senere autorisert sletting — 24. oktober 2026

Etter den reversible flyttingen godkjente eier permanent sletting av overflødige filer, også gamle eksperimenter og rålogger. Tidligere avsnitt om fullstendige snapshots og reversering beskriver derfor tilstanden før denne slettingen og kan ikke brukes som løfte om at alle disse filene fortsatt finnes.

Fjernet: registrerte avsluttede /tmp-eksperimenter og bygg (Pi-proxyen beholdt), evidence/private/2026-10-24/temporary-snapshots/, archive/2026-10-24/staging-checkouts/, archive/2026-10-24/stale-september-build/, RNode_Firmware/build/, en verifisert overflødig eldre microReticulum-bundle, samt gamle USB-raw/frames-, terminal- og EEPROM-rålogger i valgte chatmapper og deres bevarte kopier. Rapporter, oppsummerte målinger, kildehistorikk, arkivmerker og artifacts/ med stabile/tilbakeføringspakker er beholdt. Aktive repoer og gjenværende utviklingsgrener er uendret. Ingen radiotjeneste eller QD er endret.

Sletteregister: archive/2026-10-24/deletion-audit/deleted-files.json. Det registrerer 117 slettemål og 2 161 645 868 byte logisk filinnhold (ca.2,16GB), med filnavn og SHA256 før sletting. Dette er ikke en måling av fysisk frigjorte diskblokker. Registeret er et revisjonsspor, ikke en sikkerhetskopi; slettede rålogger er ikke lenger tilgjengelige for full detaljanalyse. Oppsummerte testresultater består. Råregisteret holdes lokalt; bare sammendrag publiseres.
