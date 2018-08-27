### Marco Di Vincenzo - 0000795484

# Final Project - Computer Graphics 2018

## **Introduzione**
Questo progetto è stato realizzato in **C++** (standard 11) con 
- **OpenGL** (2.1+)
- **GLUT / FreeGLUT**

E' stato realizzato e testato su **MacOS 10.12**, ma *dovrebbe* funzionare ugualmente anche su versioni più recenti (ammesso che i framework richiesti si trovino nelle posizioni di default).

La compilazione *dovrebbe* essere possibile mediante un semplice ```make```, lasciando che sia l'opportuno ```makefile``` a fare il resto. Il makefile internalmente usa ```g++``` (```gcc```), ma *teoricamente* qualsiasi altro compilatore che supporti lo standard 11 dovrebbe funzionare, previa opportuna specifica nel makefile.

Si prega di perdonare la dimensione sconsiderata delle immagini incluse: si sarebbe potuto risparmiare molto usando immagini in formato ```jpg``` ma per la loro lettura sarebbero servite librerie aggiuntive (quali ad esempio ```SDL_Image```) sulle quali non si è voluto contare.

## **Controlli**

I controlli sono relativamente pochi:
- ```WASD``` keys per il movimento
- ```1``` per cambiare la camera
- ```2``` per attivare/disattivare il rendering in wireframe *"leggero"* (solo linee, non colorazione)
- ```3``` per trasformare in cristallo alcuni oggetti (cioè rendendoli trasparenti).
- ```4``` per attivare/disattivare le luci frontali
- ```5``` per attivare/disattivare le ombre
- ```0``` per attivare una resa in wireframe "pieno" (con anche colorazione), **ma a costo di dimezzare gli FPS** (da 60 a 30, di fatto occorre renderizzare due volte la scena, una volta per le linee e una volta per riempire)
- ```P``` per attivare/disattivare il movimento di alcuni oggetti
- ```M``` per attivare/disattivare la minimappa
- ```H``` per mostrare/nascondere l'help in-game
- ```Q```/```ESC``` per uscire.

Tra le varie opzioni possibili sulla camera, c'è anche il controllo manuale mediante mouse. Tuttavia, dato il limitato supporto che OpenGL + GLUT fornisce agli eventi legati alla rotella del mouse, quel tipo di interazione può risultare non molto intuitivo.

In quella modalità, si usi
- ```RMB``` (tasto destro) per allontare la visuale
- ```MWB``` (pressione sulla rotella) per avvicinare la visuale
- ```LMB``` (tasto sinistro) per trascinare e cambiare l'angolo di vista.


> Non è garantito, al momento, **nessun tipo di interazione via joystick**, principalmente a causa della carenza di hardware con cui testare da parte di chi scrive. *Dovrebbe* comunque essere possibile guidare la moto con il joystick (nel modo intuitivo), ma senza nessuna particolare finezza. La gestione degli eventi joystick è stata realizzata "al buio" e potrebbe per questo essere difettosa.

----

## **Potenziali problemi**
1. Uso prolungato (15+ minuti, variabile con la temperatura atmosferica) può causare *surriscaldamento* del PC. Questo è dovuto a una serie di scelte e comportamenti adottati che sacrificano un sano uso di risorse sull'altare dei FPS. In particolare, si è scelto di *non attendere* il completamento delle operazioni di resa di un frame prima di iniziare a lavorare sul successivo (ossia, nessuna chiamata ```glFinish()```). Questo non causa nessun difetto (apparente) nella resa, ma priva la GPU di momenti in cui la pipeline è inattiva; in altre parole, si priva la GPU di importanti momenti per raffreddarsi, provocando, alla lunga, probabile surriscaldamento.

2. In fase di compilazione, se si abilitano, si possono ricevere **numerosi** warning, per lo più concernenti l'uso di funzionalità deprecate. Si ignorino serenamente.
