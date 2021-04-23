# uPark

Gestione automatizzata della sosta di veicoli in università identificati su base targa.

<br>

## Struttura del progetto

![Imgur](https://i.imgur.com/yV7tuGd.png)

uPark è un **RESTful Web Service** ed implementa un'architettura **client-server**.
L'elemento principale è il **main server**. Gli altri elementi software sono:

 * **Client Utente/Amministratore**
 * **Client fotocamera**
 * **Server elaborazione**
 * **DB Università**
 * **DB uPark**


Tutte le comunicazioni rispettano lo stile architetturale precedentemente citato, fatta
eccezione per quelle tra il main server ed i **DBs** (che avvengono tramite connettore MySQL) e quella tra il **Server elaborazione** ed il **Client fotocamera** (che avviene tramite socket TCP).

<br>

## Scelte progettuali

### Backend

<br>

* **MAIN_SERVER**:

E' stato scelto come linguaggio **C++** per la sua efficienza e flessibilità. Il paradigma di **programmazione ad oggetti** è dominante ed è stato utilizzato un pattern architetturale riconducibile al **Data Mapper** al fine di ottenere un trasferimento dati bidirezionale tra i databases relazionali e la rappresentazione dei dati in memoria.

Il sistema implementa una **cache** in modo da velocizzare le operazioni di lettura, sgravando il database dalle richieste frequenti.

Il **main_server** realizza il **REST server** per mezzo della libreria [cpprestsdk](https://github.com/microsoft/cpprestsdk) gestendo le richieste provenienti dagli utenti (amministratori e non) e dal **processing_server**.
Il protocollo utilizzato per la comunicazione è **HTTPS**.

Si interfaccia con i 2 databases (**DB upark** e **DB university**) tramite il connettore  [MySQL Connector/C++ 8.0](https://dev.mysql.com/doc/connector-cpp/8.0/en/).

<br>

 * **Ingresso**

 Il **main_server** sulla base della targa processata e del parcheggio controlla se vi è una prenotazione valida (in corso o nei prossimi 5 minuti) autorizzando o meno l’ingresso del veicolo.

 Fatto ciò, in caso positivo andrà a registrare l’**entry_time** della relativa prenotazione (sul **DB UPark**).

<br>

 * **Uscita**

    Il **main_server** sulla base della targa processata e del parcheggio risale alla prenotazione registrandone l’**exit_time**. Successivamente in caso positivo comanda il sollevamento dell’asta.

<br>

 * **Policy del parcheggio**

   * La prenotazione minima è pari a **15 minuti**, quelle di durata superiore devono comunque essere *multiple del quarto d’ora* (es. 7.00 - 7.15). Orario di inizio e fine della prenotazione devono essere *multipli del quarto d’ora*.

   * Una prenotazione blocca la prenotazione relativa ai **15 minuti** *precedenti e successivi* per consentire un'eventuale *tolleranza* in uscita (**5 minuti**) e/o rimozione (**10 minuti**).

   * Lo stesso utente può estendere una precedente prenotazione (utilizzando lo stesso veicolo) se questa non interferisce con altre già presenti.

   * Nel caso in cui vengano sforati i **5 minuti** di tolleranza entro i successivi **10 minuti** sarà segnalata l’uscita tardiva nel campo **note** della prenotazione.

   * Oltre i **15 minuti** non sarà più possibile uscire dal parcheggio senza essersi rivolti prima agli addetti.

   * <ins>Si paga al momento della prenotazione</ins>.

   * Se l’utente non si presenta all’orario stabilito poiché l’importo è già stato pagato il posto sarà da considerarsi come riservato e a sua disposizione per l’intera durata della prenotazione.

   * Possibilità di ingresso (anche ritardata) ed **uscita multiple** durante l’intera durata della prenotazione.

<br>
<br>

* **PARKING_LOT_CLIENT**:

E' stato scelto come linguaggio **Python** per la rapidità di sviluppo e per la larga quantità di librerie a disposizione. Inserito l'identificativo di rete (port) del parcheggio, il client si occupa di sovrintendere l'ingresso/uscita dei veicoli in/da quest'ultimo.

Inoltre è stato realizzato uno script capace di generare un'immagine di un veicolo avente la targa immessa sotto forma testuale.  

<br>

 * **Ingresso**

    Simula l’arrivo di un veicolo all’ingresso di un preciso parcheggio. Trasmette un’immagine raffigurante una targa al **processing_server** prelevandola da una cartella precedentemente alimentata. Alla ricezione della notifica di “targa valida” invia il comando per il sollevamento dell’asta.

<br>

 * **Uscita**

    Simula l’arrivo di un veicolo all’uscita del parcheggio. Trasmette un’immagine raffigurante la targa del veicolo uscente in modo equivalente al caso d’ingresso.

I clients sono pari in numero al doppio dei parcheggi.

<br>
<br>

* **PROCESSING_SERVER**:

Ad ogni nuova richiesta di connessione avvia un thread che si occupa del processamento dell’immagine ricevuta dal client, ne estrae la targa avvalendosi della libreria [OpenALPR](https://github.com/openalpr/openalpr) e la invia al **main_server** insieme al **nome** del parcheggio da cui è stata trasmessa, al **crossing_type** e ad un **auth_token** (necessario per autenticarsi con il **main_server**). Rimane in attesa di riscontro dell'esistenza di una prenotazione valida per la suddetta targa ed inoltra al client la risposta (positiva o meno).

 Il **processing_server** dialoga con il **main_server** mediante REST API **POST /crossing**.

Il **processing_server** conosce quali sono i clients validi in quanto essi sono individuati all'interno di un file di configurazione **json** (*clients.json*) che associa ad una coppia (IP, porta) il nome del parcheggio (*parking_lot_name*) ed il tipo di attraversamento (in ingresso o uscita) (*crossing_type*).

<br>

### Frontend

Anche in questo caso è stato scelto come linguaggio **Python** ed in particolar modo si è fatto uso della libreria grafica [PyQt5](https://pypi.org/project/PyQt5/) per la realizzazione dell'intera GUI e la libreria HTTP [Requests](https://docs.python-requests.org/en/master/) per l'esecuzione delle richieste HTTPS.

<br>
<br>


* **CASI D’USO UTENTE (amministratore e non)**:

<br>

**Avvio**

Si presenta la schermata **"Home"** con la possibilità di *registrarsi* o *accedere*.

<br>

**Registrazione**

L’utente inserisce le credenziali con le quali desidera registrarsi al sistema: *email*, *nome*, *cognome*, *password*, *categoria* (Studente, Professore, Ospite, ecc.), *uParkCode*.

Un utente universitario non si può registrare con una *email* o una *categoria* diverse rispetto a quelle con cui figura sul **DB_University**. Condizione necessaria per la registrazione è la presenza sul **DB_University** (in quanto viene effettuato il controllo su quest'ultimo al momento della registrazione).

Gli utenti universitari sono coloro i quali hanno una forma di interazione con l’università più o meno lunga (rientrano in questa categoria gli studenti, i professori, il personale vario come dottorandi, stagisti, settore amministrativo, segreteria, amministratore ecc...).
Nel caso di utenti universitari l’**uParkCode** (simile ad un PIN) serve a verificare l’identità del singolo utente (congiuntamente alla categoria e alla email) mediante un controllo sul database dell’università proprietaria (**DB_University**, relazione **upark_users**). Se si ha una corrispondenza, la registrazione avviene con successo altrimenti viene rifiutata.
La gestione di tale database è demandata all’università, la quale si occuperà di fornire/negare secondo le proprie politiche l’accesso al servizio in questione.
Al fine di rendere l’università indipendente da uPark l’unica forma di interazione tra di esse avviene mediante la relazione **upark_users** sul **DB_University**.


    NOTA: l’amministratore non deve registrarsi, risulterà già presente nel sistema (nell'opportuna categoria) con le credenziali email: admin password: adminadmin da modificare al primo utilizzo.

<br>

**Accesso**

L’utente inserisce le credenziali, queste vengono controllate sul **DB uPark** e in caso di esito positivo, l’utente accede alla propria **Dashboard**, altrimenti viene avvisato del mancato login e invitato a riprovare o a registrarsi (nel caso non abbia ancora un account).

    NOTA: Il login è sempre permesso ad un utente registrato anche se non più “attivo”, ma con l’impossibilità di effettuare prenotazioni.

<br>
<br>

* **CASI D’USO UTENTE NON AMMINISTRATORE:**

<br>

**Prenotazione**

Il generico utente autenticatosi al sistema vuole trovare un posto in università (Menù:**”Park”**).

    NOTA: Ad un utente già registrato avente l’account disabilitato da parte dell’admin (es. poiché ritirato o pensionato nel caso sia un professore) non gli sarà più consentito effettuare prenotazioni.

Il sistema restituisce la lista dei parcheggi da esso gestiti opportunamente filtrati sulla categoria dell’utente. Selezionato il parcheggio si avrà visione degli stalli che lo compongono, della tipologia di veicolo ammessa per ciascuno di essi (es. auto, moto) e la relativa schedulazione oraria per la giornata odierna (default). Ad ogni stallo, in un determinato istante temporale, corrisponde un colore:
- **bianco** (libero)
- **grigio chiaro** (disattivato)
- **grigio scuro** (prenotato da un altro utente)
- **blu** (riservato disabilità - libero)
- **arancione** (prenotato dall’utente)

Si può effettuare la prenotazione per una delle fasce orarie libere del giorno stesso o di uno seguente. L’utente seleziona la soluzione (data e ora) a lui più confacente selezionando l'intervallo di tempo d'interesse e specificando il mezzo tra quelli posseduti precedentemente inseriti e filtrati sulla base del tipo di stallo.
Se i parametri della prenotazione vengono accettati dal sistema, l’utente viene avvisato dell’esito positivo della prenotazione, in caso contrario si riceverà un'opportuna notifica.

Se invece l’utente non trova una soluzione che soddisfa le sue aspettative (es. posti pieni o parcheggio troppo distante) chiude l’applicazione disconnettendosi dal sistema.


    NOTA: Un utente può effettuare prenotazioni solo se il suo account è attivo. Lo stato di “account_attivo” è settato di default ad attivo (True) al momento della registrazione con successo.

<br>

**Prenotazioni attive**

L’utente può visualizzare le sue prenotazioni in corso (valide temporalmente) (Menù:**”Bookings in progress”**) e per ciascuna di esse un riepilogo della prenotazione con i relativi dettagli.

Si può annullare una prenotazione in corso con un *preavviso minimo* (**2 ore**).

<br>

**Prenotazioni scadute**

L’utente può visualizzare le sue prenotazioni scadute e per ciascuna di esse un riepilogo della prenotazione con i relativi dettagli (Menù:**”Bookings expired”**).

<br>

**Gestione veicoli**

L’utente può vedere la totalità dei veicoli posseduti suddivisi per tipologia. Può inoltre aggiungerne di nuovi o rimuoverne. (Menù:**”Vehicles”**)

<br>

**Gestione profilo**

L’utente può modificare la sua password ed il suo saldo effettuando una ricarica. (Menu:**”Profile”**)

<br>

**Logout**

L’utente può disconnettersi dal sistema e ritornare alla schermata **Home** cliccando sul Menù:**"Logout"**.

<br>
<br>

* **CASO D’USO AMMINISTRATORE:**

L'amministratore una volta autenticatosi al sistema può gestire il sistema nella sua totalità.

<br>

**Gestione Parcheggi**

L’amministratore (*unico*) autenticatosi al sistema vuole gestire i parcheggi presenti in università (Menù:**"Parking Management"**):
* Può **visualizzare** tutti i parcheggi presenti, i relativi stalli e per ciascuno di essi la schedulazione oraria per la giornata odierna (default) o per un’altra data qualsiasi.
* Può **aggiungere** un parcheggio specificandone il nome, la via, il numero di stalli ed il tipo di veicolo associato di default.
* Può **eliminare** un parcheggio (rimuovendo tutto ciò che ad esso è associato: stalli, prenotazioni).
* Può **modificare** il nome, settare gli stalli riservati per i disabili, assegnare a ciascuno stallo il tipo di veicolo abilitato a sostare ed infine riservare il parcheggio per una o più (eventualmente tutte - di default) le categorie (es. professori).

<br>

**Gestione Prenotazioni**

L’amministratore autenticatosi al sistema può **visualizzare** le prenotazioni in corso (Menù:**"Booking in Progress"**) e quelle scadute (Menù:**"Booking expired"**) con i relativi dettagli per tutti gli utenti presenti su uPark.

<br>

**Altre impostazioni**

L'amministratore può:
 * aggiungere, eliminare e modificare le **tariffe orarie**.
 * aggiungere, eliminare e modificare i **tipi di veicolo**, specificandone il nome ed il **fattore di moltiplicazione** della tariffa oraria.
 * modificare la **tariffa oraria** associata con ciascuna **categoria utente**.

<br>

**Gestione Utenti**

 * L’amministratore può **visualizzare** la totalità degli utenti presenti sul **DB uPark** e le rispettive informazioni:  *categoria di appartenenza* ed *veicoli intestati*
 * può **eliminare** utenti esistenti
 * può **modificarne** la *password*, *abilitarne/disabilitarne* l’account o *riconoscerne la disabilità* (nel caso di accertamento con esito positivo a fronte della domanda da parte dell’utente).

<br>

**Gestione profilo**

L'amministratore può **visualizzare** il suo profilo, **modificarne** la *password* e visualizzare gli **incassi complessivi**.

<br>

**Logout**

L’utente può disconnettersi dal sistema e ritornare alla schermata **Home** cliccando sul Menù:**"Logout"**.

<br>
<br>

## Passaggi preliminari

1) Importare i file **db_university.sql** e **db_upark.sql** su mysql
2) Creare la cartella main_server/**build** e compilare il codice:
> build$ cmake .. && make

3) Creare la cartella processing_server/**build** e compilare il codice:
> build$ cmake .. && make

<br>
<br>

## Utilizzo

1) Accertarsi che **mysql server** sia attivo
2) Avviare il **main_server**
> bin$ ./upark_rest_server

        NOTA: Al primo utilizzo avviare 'uPark_client' e accedere come admin

        > uPark_client$ ./home.py

        inserire i parcheggi e le informazioni necessarie per il corretto funzionamento dell'applicativo

        NOTA: Modificare il file processing_server/utility/clients.json aggiungendo le informazioni dei parcheggi precedentemente inseriti

3) Avviare il **processing_server**
> bin$ ./upark_processing_server
4) Eseguire **pl_app.py** tante volte quanti sono gli ingressi e le uscite di tutti i parcheggi realizzati (presenti nel **clients.json**)
> parking_lot_client$ ./pl_app.py port

        NOTA: 'port' è il numero di porta indicato nel file clients.json per il parcheggio scelto in ingresso o in uscita

5) Avviare **uPark_client** come user
> uPark_client$ ./home.py

6) Una volta inserita una prenotazione valida all'interno del sistema, al fine di simulare l'ingresso/uscita nel/dal parcheggio, occorre selezionare un veicolo dall'istanza di **pl_app.py** corrispondente al parcheggio scelto.

    Selezionato il veicolo con cui si vuole entrare/uscire dal parcheggio, attendere la risposta dal server e quindi il sollevamento o meno dell'asta del parcheggio.

        NOTA: nel caso non si abbiamo immagini di veicoli disponibili eseguire prima del passo 6 lo script parking_lot_client/utility/license_plate_img_creator.py
        aggiungendo come primo parametro la targa di cui si vuole generare l'immagine
        es.  
            utility$ ./license_plate_img_creator.py   AB123CD



<br>
<br>

## Versioning
E' stato utilizzato [Git](https://git-scm.com/).

<br>
<br>

## Autori

**Michele Cannizzaro, Michele Grasso**
