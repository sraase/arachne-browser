;-----------------------------------------------------
; Procedure for drawing raster char on Hi-color modes
;-----------------------------------------------------

IFDEF  A_SMALL
A_OFF    EQU    4
A_SHF    EQU    0
ELSE
A_OFF    EQU    6
A_SHF    EQU    2
ENDIF

IFDEF  A_SMALL
_TEXT SEGMENT BYTE PUBLIC 'CODE'
      ASSUME cs:_TEXT,DS:_DATA,_BSS
_xh_wrtchr    proc near
ELSE
XH_WRCHR_TEXT SEGMENT BYTE PUBLIC 'CODE'
	      ASSUME cs:XH_WRCHR_TEXT,DS:_DATA,_BSS
_xh_wrtchr    proc far
ENDIF
	    public _xh_wrtchr
;
	    push BP
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES
;
; Call from C: xh_wrtchr(char *buf,int xpix,ypix,ncol,nrow,col1,col2);
; buf       - binary image of char
; col1,col2 - color of foreground, background of char
;

            mov  CS:VESA64, 0       ; Default value ;; JdS 2004/10/16

IFDEF A_SMALL
	    mov  AX,[_xg_chrmod]     ; text mode (fill, alfa)
	    mov  CS:CHRMOD,AX

	    mov  AX,[_xg_svga]
ELSE
	    mov  AX,SEG _xg_chrmod   ; text mode
	    mov  ES,AX
	    mov  AX,ES:[_xg_chrmod]
	    mov  CS:CHRMOD,AX

	    mov  AX,SEG _xg_svga
	    mov  ES,AX
	    mov  AX,ES:[_xg_svga]   ; grf. mode
ENDIF
	    cmp  AH,80h             ; VESA mode? 0695
	    jne  No_vesa

	    push ES
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa     ; ES:BX address VESA struct
;; JdS 2004/10/16 {
;;	    mov  BX,[ES:BX+4]       ; Granularity
;;	    pop  ES
;;	    cmp  BX,64
;;	    je   No_vesa
;;	    mov  CS:VESA64, 1
;;	    jmp  Cont_1;
;;No_vesa:  mov  CS:VESA64, 0
            mov  CX,ES:[BX+4]       ; Granularity
            mov  BX,ES:[BX+20]      ; Bytes/line
            pop  ES
            cmp  CX,64
            je   Chk_BpL
            mov  CS:VESA64, 1
Chk_BpL:    cmp  BX,0           ; If the VESA bytes/line is unknown,
            jne  X_2            ;   make best guess from graphics mode
No_vesa:
;; JdS 2004/10/16 }

 Cont_1:    push AX

	    and AL,1Fh
	    cmp AL,01h      ; length of 320,640,800,1024
	    je  Go320;
	    cmp AL,02h
	    je  Go640
	    cmp AL,04h
	    je  Go800
	    cmp AL,08h
	    je  Go1024
	    pop AX
	    jmp Enderr

    Go320:  mov BX,640
	    jmp X_1
    Go640:  mov BX,1280
	    jmp X_1
    Go800:  mov BX,1600
	    jmp X_1
   Go1024:  mov BX,2048

      X_1:  pop  AX
      X_2:  and  AL,0E0h  ; 3 bits ;; JdS 2004/10/16 (VESA jump point)
	    cmp  AL,60h   ; Hi-color
	    je   Hi_color
	    jmp  Enderr

;------------------ Hi_color color modes ---------------------
;
 Hi_color:  push AX           ; [BP-0Ah] xg_svga
	    mov  CX,[BP+A_OFF+A_SHF+6]  ; pixels per line
	    push CX           ; [BP-0Ch]
	    push BX           ; [BP-0Eh] length of line
	    sub  SP,4
	    mov  word ptr[BP-10h],-1 ; Akt seg. (granul) videoram
	    mov  word ptr[BP-12h],0  ; Length segment+1 (65536 = 0)

	    sub SP,6
	    mov  word ptr[BP-14h],0 ; Continue line after SET_SEG
	    mov  word ptr[BP-16h],0 ; AX after SET_SEG
	    mov  word ptr[BP-18h],0 ; BX after SET_SEG

	    mov  DI,[BP+A_OFF]    ; ES:DI - input buffer (char)
	    mov  DX,[BP+A_OFF+2]
	    mov  ES,DX

	    mov  AX,0A000h        ; DS:SI - videoram
	    mov  DS,AX
;
  Radek:
	    mov  word ptr [BP-14h],0    ; New line
	    mov  CX,[BP+A_OFF+A_SHF+6]  ; # of pixel per line
	    mov  [BP-0ch],CX
	    mov  DX,[BP+A_OFF+A_SHF+4]  ; line
	    mov  AX,[BP-0Eh]  ; 320/640/800/1024 save
	    mul  DX
	    add  AX,[BP+A_OFF+A_SHF+2]  ; add col  2x
	    adc  DX,0                   ; AX offset, DX segment
	    add  AX,[BP+A_OFF+A_SHF+2]
	    adc  DX,0
;
	    mov  SI,AX        ; SI offset
	    push DX           ; save segment
;
	    cmp  word ptr[BP-12h],0     ; 64K segment
	    jne  Set_seg
	    cmp  word ptr[BP-10h],DX    ; set segment
	    jne  Set_seg
	    jmp  Draw
; Set segment videoram
;
   Set_seg: cmp  word ptr[BP-12h],0
	    jne  Hop_seg       ; for < 64kB set at VESA
	    mov  [BP-10h],DX
   Hop_seg: mov  AX,[BP-0ah]   ; grf. mode xg_svga

	    cmp  AH,80h
	    je   VESA
	    jmp  Hop2          ; end + dealloc stack

;------------- VESA --------------------------
  VESA:	    push ES
IFDEF A_SMALL
	    lea  BX,_xg_vesa
ELSE
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa   ; ES:BX adresa VESA struct
ENDIF
	    mov  CL,ES:[BX+2]     ; Najit okno pro WRITE
	    and  CL,05h
	    cmp  CL,05h
	    je   Okno_A
	    mov  CS:WND_BL, 1
	    jmp  Granul
Okno_A:     mov  CS:WND_BL, 0
Granul:
	    push DI
	    push BX
	    mov  BX,ES:[BX+4]     ; Granularity
	    cmp  BX,64
	    je   Vesa_hop
	    mov  CL,10            ; * 1024
	    shl  BX,CL
	    mov  [BP-12h],BX      ; Length of segment
	    mov  AX,SI
	    div  BX               ; DX:AX / BX
	    xchg AX,DX            ; DX - granul , AX offset in granul
	    mov  [BP-10h],DX
	    mov  SI,AX
 Vesa_hop:  mov  BH,0
	    mov  BL,CS:WND_BL
	    mov  AX,4F05h         ; VESA Bios Window control
	    ;;int  10h
	    pop  DI               ; DI<-BX
	    call DWORD PTR ES:[DI+0Ch]
	    pop  DI               ; refresh DI,ES
	    pop  ES
;-------------------------------------------------------
; Write image of char to videoram
;
   Draw:    mov  CX,[BP-0ch] ; number of pixel  (all line / remainder)

	    cmp  word ptr [BP-14h],0  ; continuing line after SET_SEG
	    jne  Restore
	    mov  AH,ES:[DI]    ; Begin line
	    mov  BL,8
	    jmp  Cyk3
   Restore: mov  AX,[BP-16h]
	    mov  BX,[BP-18h]
;
; Draw one line
;
  Cyk3:     mov  AL,AH
	    and  AL,80h
	    je   poz_c
	    mov  DX,[BP+A_OFF+A_SHF+0Ah]  ; color of text
	    jmp  A_3

   poz_c:   cmp  CS:CHRMOD,0
	    jne  No_poza
	    mov  DX,[BP+A_OFF+A_SHF+0Ch]  ; color of background

     A_3:   mov  DS:[SI],DX  ; write to videoram

 No_poza:   dec  CX
	    jz   End3
	    dec  BL           ; 8
	    jne  hop_x1
	    inc  DI
	    mov  AH,ES:[DI]
	    mov  BL,8
	    jmp  hop_x2
  hop_x1:   shl  AH,1
  hop_x2:
	    inc  SI
	    inc  SI
	    cmp  SI, word ptr [BP-12h]  ; New segm. in line
	    jne  Cyk3
;                            ; SI = 0 -> next segm
	    mov  [BP-0ch],CX  ; save + AX,BX
	    mov  SI,0
	    mov  word ptr [BP-14h],1
	    mov  [BP-16h],AX
	    mov  [BP-18h],BX

	    cmp  CS:VESA64, 0           ; 0695
	    je   No_granul
	    inc  word ptr[BP-10h]
	    mov  DX, [BP-10h]
	    push ES
	    mov  BX,SEG _xg_vesa  ; !--- 0397
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa   ; ES:BX adresa VESA struct
	    push DI
	    push BX               ; !---
	    jmp  Vesa_hop

No_granul:  pop  DX
	    inc  DX
	    push DX
	    jmp  Set_seg      ; set segment in DX
;
; Next line
  End3:     pop  DX
	    inc  DI
	    inc  SI
	    inc  SI
	    inc  word ptr [BP+A_OFF+A_SHF+4] ; y1++
	    dec  word ptr [BP+A_OFF+A_SHF+8] ; dy--
	    jz   Hop2
	    jmp  Radek

      Hop2: add  SP,16     ; local var
	    jmp  Endx
;
;------------------ Common end ------------------------
     Endx:  mov  AX,1       ; O.K. end
	    jmp  Uplny

     Enderr: mov AX,-1      ; Err end

     Uplny: pop  ES
	    pop  DS
	    pop  SI
	    pop  DI
	    pop  BP
;
	    ret
_xh_wrtchr  endp

CHRMOD DW  0   ; alfa text 0-n, 1-y
WND_BL DB  0   ; VESA window
VESA64 DB  0   ; 0/1 granul < 64

IFDEF  A_SMALL
_TEXT ENDS
ELSE
XH_WRCHR_TEXT ENDS
ENDIF

DGROUP         GROUP    _DATA,_BSS
_DATA          SEGMENT  WORD PUBLIC 'DATA'
	EXTRN  _xg_chrmod:word
_DATA          ENDS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_svga:word
	EXTRN  _xg_vesa:word
_BSS          ENDS
	END
