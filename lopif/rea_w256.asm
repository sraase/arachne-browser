;-----------------------------------------------------
; Procedure for reading raster image in 256 colors
;-----------------------------------------------------

include svga_mac.inc

IFDEF  A_SMALL
A_OFF    EQU    4
A_SHF    EQU    0
ELSE
A_OFF    EQU    6
A_SHF    EQU    2
ENDIF

IFDEF  A_SMALL
_TEXT SEGMENT BYTE PUBLIC 'CODE'
      ASSUME cs:_TEXT,DS:_BSS
_rea_w256 proc near
ELSE
REA_W256_TEXT SEGMENT BYTE PUBLIC 'CODE'
	      ASSUME cs:REA_W256_TEXT,DS:_BSS
_rea_w256 proc far
ENDIF
	    public _rea_w256
;
	    push BP
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES
;
; Call from C: rea_w256(char *buf,int xpix, ypix, ncol, nrow)
;
            mov  CS:VESA64, 0       ; Default value ;; JdS 2004/09/27

	    mov  AX,SEG _xg_svga
	    mov  ES,AX
	    mov  AX,ES:[_xg_svga]   ; graf.mod

	    cmp  AH,80h             ; Is this VESA mode? 0695
	    jne  No_vesa

	    push ES
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa     ; ES:BX address VESA struct
;; JdS 2004/09/27 {
;;          mov  BX,[ES:BX+4]       ; Granularity
;;          pop  ES
;;          cmp  BX,64
;;          je   No_vesa
;;          mov  CS:VESA64, 1
;;          jmp  Cont_1;
;;No_vesa:  mov  CS:VESA64, 0

            mov  CX,ES:[BX+4]       ; Granularity
            mov  BX,ES:[BX+20]      ; Bytes/line
            pop  ES
            cmp  CX,64
            je   Chk_BpL
            mov  CS:VESA64, 1
Chk_BpL:    cmp  BX,0           ; If the VESA bytes/line is unknown,
            jne  X_1            ;   make best guess from graphics mode
No_vesa:
;; JdS 2004/09/27 }

Cont_1:	    and AL,1Fh
	    cmp AL,01h      ; length of line 320,640,800,1024
	    je  Go320;
	    cmp AL,02h
	    je  Go640
	    cmp AL,04h
	    je  Go800
	    cmp AL,08h
	    je  Go1024
	    jmp Errend

    Go320:  mov BX,320
	    jmp X_1
    Go640:  mov BX,640
	    jmp X_1
    Go800:  mov BX,800
	    jmp X_1
   Go1024:  mov BX,1024

       X_1: mov  DI,[BP+A_OFF]    ; ES:DI - output buffer with image
IFDEF  A_SMALL
	    mov  DX,DS
ELSE
	    mov  DX,[BP+A_OFF+2]
ENDIF
	    mov  ES,DX

	    push AX           ; [BP-0ah] xg_svga
	    mov  CX,[BP+A_OFF+A_SHF+6]  ; num pixels of line
	    push CX           ; [BP-0ch]
	    push BX           ; [BP-0Eh] length of line
	    sub  SP,4
	    mov  word ptr[BP-10h],-1 ; Akt. seg (granul) videoram
	    mov  word ptr[BP-12h],0  ; Length of segment+1 (65536 = 0)
;
	    mov  AX,0A000h    ; DS:SI - address videoram
	    mov  DS,AX
;
  Radek:
	    mov  CX,[BP+A_OFF+A_SHF+6]  ; pixels per line
	    mov  [BP-0ch],CX
	    mov  DX,[BP+A_OFF+A_SHF+4]  ; line
	    mov  AX,[BP-0Eh]  ; 320/640/800/1024 save
	    mul  DX
	    add  AX,[BP+A_OFF+A_SHF+2]  ; add column
	    adc  DX,0         ; AX offset, DX segment
;
	    mov  SI,AX        ; SI offset
	    push DX           ; save segment
;
	    cmp  word ptr[BP-12h],0     ; 64K segment
	    jne  Set_seg
	    cmp  word ptr[BP-10h],DX    ; Set segment ?
	    jne  Set_seg
	    jmp  Draw
; Set segment of videoram
;
   Set_seg: cmp  word ptr[BP-12h],0
	    jne  Hop_seg       ; for < 64kB set at VESA
	    mov  [BP-10h],DX
   Hop_seg: mov  AX,[BP-0ah]   ; grf. mode xg_svga
	    cmp  AH,00h
	    je   Standart               ; 320 x 200
	    cmp  AH,01h
	    je   Tseng
	    cmp  AH,02h
	    je   OAK
	    cmp  AH,04h
	    je   Tamara
	    cmp  AH,08h
	    je   Trident
	    cmp  AH,10h
	    je   RealTek
	    cmp  AH,20h
	    je   Tseng4
	    cmp  AH,40h
	    je   M1
	    cmp  AH,80h
	    jne  Standart
	    jmp  VESA
;
 Standart:  jmp Draw
;------------- TSENG --------------------------
   Tseng:
	    SEG_TSENG3
	    jmp  Draw
;------------- OAK ----------------------------
     OAK:
	    SEG_OAK
	    jmp  Draw
;------------- TAMARACK -----------------------
  Tamara:
	    SEG_TAMARA
	    jmp  Draw
;------------- TRIDENT -----------------------
  Trident:
	    SEG_TRIDENT
	    jmp  Draw
;------------- RealTek -----------------------
RealTek:
	    SEG_REALTEK
	    jmp  Draw
;------------- Tseng 4000 --------------------
Tseng4:
	    SEG_TSENG4
	    jmp  Draw
;------------- M1 ----------------------------
M1:
	    SEG_M1R
	    jmp Draw
;------------- VESA --------------------------
  VESA:	    push ES
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa   ; ES:BX address VESA struct

	    mov  CL,ES:[BX+2]     ; Window for READ
	    and  CL,03h
	    cmp  CL,03h
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
	    mov  [BP-12h],BX      ; length of segment
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

; Reading videoram
;
  Draw:	    mov  CX,[BP-0ch] ; # pixels (all line|remainder)
;
; Read one line
;
  Cykl:
	    mov  AL,DS:[SI]
	    mov  ES:[DI],AL
	    dec  CX
	    jz   Endx
	    inc  DI
	    inc  SI
	    cmp  SI, word ptr [BP-12h]  ; New segm. in line
	    jne  Cykl
;                             ; SI = 0 -> next segment
	    mov  [BP-0ch],CX  ; save ramainder
	    mov  SI,0         ; only for seg < 64K

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

 No_granul: pop  DX
	    inc  DX
	    push DX
	    jmp  Set_seg      ; set segment to DX

; Next line
  Endx:     pop  DX
	    inc  DI
	    inc  SI
	    inc  word ptr [BP+A_OFF+A_SHF+4]
	    dec  word ptr [BP+A_OFF+A_SHF+8]
	    jz   E256
	    jmp  Radek

     E256:  add  SP,10        ; dealloc local
;
  Errend:
;
	    pop  ES
	    pop  DS
	    pop  SI
	    pop  DI
	    pop  BP
;
	    ret
_rea_w256   endp

WND_BL  DB  0
VESA64  DB  0

IFDEF  A_SMALL
_TEXT ENDS
ELSE
REA_W256_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_svga:word
	EXTRN  _xg_vesa:word
_BSS          ENDS
	END
