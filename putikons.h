void Iniikons(void);           //volat na zacatku, po CLRSCR a pod.
      //tr.: call at the beginning, after CLRSCR etc.
void Putikona(int x0, int y0, char *iconame); //nacteni 1 ikony do pameti
      //tr.: load 1 icon into memory
void Drawikons(int uvolni);    //vykresleni ikon z pameti na obrazovku
      //tr.: draw icon from memory onto screen
void Putikonx(int x0, int y0, char *iconame, char noswap);
//nacteni 1 ikony do pameti a okamzite vykresleni
      // tr.: load 1 icon into memory and draw immeditately
