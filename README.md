# Astro Fight 

Progetto di Computer Grafica 2D sviluppato in C++ utilizzando la libreria SFML 3.0.
Il gioco è un arcade ispirato a classici come Space Invaders e AstroFight, strutturato in 14 tappe di sviluppo incrementali.

## Come compilare (Build)
Il progetto è configurato per compilare tutte le 14 tappe contemporaneamente tramite un unico `CMakeLists.txt` centrale. 

Aprire il terminale nella cartella root del progetto e lanciare i seguenti comandi:

```bash
cmake -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```
`Release`  can be changed to `Debug` if necessary.(consiglio preso dai laboratori guidati visti in classe).

##  Come eseguire
Una volta terminata la compilazione, i file eseguibili di tutte le tappe si troveranno all'interno della cartella `build`. 
Poiché il gioco utilizza percorsi relativi per caricare le texture e i suoni, è **necessario** lanciare gli eseguibili trovandosi all'interno della cartella `build`.

Aprire il terminale e lanciare:
1. `cd build`
2. `./Tappa14` (su Mac/Linux) oppure `.\Tappa14.exe` (su Windows)

**NOTA**: facendo dei test ho notato che su macchina Windows dopo il comando  `cd build` bisogna usare ` .\Debug\Tappa14.exe` 

Per testare le tappe precedenti, sostituire il numero nell'eseguibile (es. `./Tappa08`).

Per muovere la navicella usare le frecce direzionali. Per la relazione scaricare con il tasto download, GitHub per qualche motivo non riesce a caricare la preview del documento 
