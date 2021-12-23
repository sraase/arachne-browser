;-----------------------------------------------------------
; Procedura pro kresleni pixlu v Hi COLOR  modech
;-----------------------------------------------------------
IFDEF  A_SMALL
A_OFF    EQU    4
ELSE
A_OFF    EQU    6
ENDIF

IFDEF  A_SMALL
_TEXT SEGMENT BYTE PUBLIC 'CODE'
      ASSUME CS:_TEXT,DS:_BSS
_xh_putpix   proc near
ELSE
XH_PUTPX_TEXT SEGMENT BYTE PUBLIC 'CODE'
	    ASSUME cs:XH_PUTPX_TEXT,DS:_BSS
_xh_putpix  proc far
ENDIF
	    public _xh_putpix
;
	    push BP          ; Uschova registru (?AX..DX)
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES
;
; Call from C: rgb_putpix(int x, int y, unsigned color1)
;
;------------------ RGB barevne mody --------------------------
IFDEF  A_SMALL
	    mov  AX,[_xg_svga]
ELSE
	    mov  AX,SEG _xg_svga
	    mov  ES,AX
	    mov  AX,ES:[_xg_svga]   ; graf.mod
ENDIF
	    push AX         ; xg_svga
;; JdS 2004/10/16 {
            cmp  AH,80h             ; VESA mode? 0695
            jne  No_vesa

            push ES
            mov  BX,SEG _xg_vesa
            mov  ES,BX
            lea  BX,ES:_xg_vesa     ; ES:BX address VESA struct
            mov  BX,ES:[BX+20]      ; Bytes/line
            pop  ES

	    cmp  BX,0           ; If the VESA bytes/line is unknown,
            jne  X_1            ;   make best guess from graphics mode

No_vesa:
;; JdS 2004/10/16 }

	    and AL,1Fh
	    cmp AL,01h      ; length of line 320,640,800,1024
	    je  Go320;
	    cmp AL,02h
	    je  Go640
	    cmp AL,04h
	    je  Go800
	    cmp AL,08h
	    je  Go1024

	    pop AX
	    jmp End_put

    Go320:  mov BX,640
	    jmp X_1
    Go640:  mov BX,1280
	    jmp X_1
    Go800:  mov BX,1600
	    jmp X_1
   Go1024:  mov BX,2048

      X_1:  pop AX
	    and AL,0E0h
	    cmp AL,60h ; HiColor
	    je  X_Hi
	    jmp End_put

IFDEF A_SMALL
  X_Hi:     push AX
	    mov  AX,[_xg_wrt]
	    mov  CS:WRTMOD,AX
ELSE
  X_Hi:     push AX
	    mov  AX,SEG _xg_wrt
	    mov  ES,AX
	    mov  AX,ES:[_xg_wrt]   ; XOR
	    mov  CS:WRTMOD,AX
ENDIF
	    mov  AX,0A000h         ; DS:SI - addr in video
	    mov  DS,AX
;
	    mov  DX,[BP+A_OFF+2]  ; line
	    mov  AX,BX            ; 640/800 pixlu na radek * 2(schovame)
	    mul  DX
	    add  AX,[BP+A_OFF]    ; pricteme sloupec * 2
	    adc  DX,0             ; v AX offset, v DX segment (! pro 64kB)
	    add  AX,[BP+A_OFF]
	    adc  DX,0
	    mov  CS:SEG64,DX
;
	    mov  SI,AX        ; v SI offset
	    mov  CS:XORSI,SI
;
; Only  VESA !!!
	    pop  AX            ; xg_svga
	    cmp  AH,80h
	    je   VESA
	    jmp  End_put

  VESA:
;------------- VESA standart -----------------
IFDEF A_SMALL
	    lea BX,_xg_vesa
ELSE
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa   ; ES:BX adresa VESA struct
ENDIF
	    mov  CL,ES:[BX+2]     ; Najit okno pro WRITE/READ
	    cmp  CS:WRTMOD,0
	    jne  For_read2
	    and  CL,05h   ; WRITE
	    cmp  CL,05h
	    jmp  Ok_wrt
For_read2:  and  CL,03h   ; READ
	    cmp  CL,03h

Ok_wrt:	    je   Okno_A
	    mov  CS:WND_BL, 1
	    jmp  Granul
Okno_A:     mov  CS:WND_BL, 0
Granul:
	    push DX               ; 0695
	    mov  CS:XORSI,SI
	    push DI
	    push BX            
	    mov  BX,ES:[BX+4]     ; Granularita
	    cmp  BX,64
	    je   Vesa_hop
	    mov  CL,10            ; * 1024
	    shl  BX,CL
	    mov  AX,SI
	    div  BX               ; DX:AX / BX
	    xchg AX,DX            ; DX - granule , AX offset v granuli
	    mov  SI,AX
 Vesa_hop:  mov  BH,0
	    mov  BL,CS:WND_BL
	    mov  AX,4F05h         ; VESA Bios Window control
	    ;;int  10h
	    pop  DI               ; DI<-BX
	    call DWORD PTR ES:[DI+0Ch]
	    pop  DI               ; refresh DI,DX
	    pop  DX
;-------------------------------------------------------
;
; Vlastni kresleni - psani do videoram
;
   Draw:
	    mov  AX,CS:WRTMOD
	    cmp  AX,0
	    je   Skip1

	    mov  CX,DS:[SI]      ; read videoram for XOR
	    xor  CX,[BP+A_OFF+4]
	    mov  CS:XORCX,CX
	    mov  SI,CS:XORSI

IFDEF A_SMALL
	    lea BX,_xg_vesa
ELSE
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa   ; ES:BX adresa VESA struct
ENDIF
	    mov  CL,ES:[BX+2]     ; Najit okno pro WRITE/READ
	    and  CL,05h   ; WRITE
	    cmp  CL,05h
	    je   Okno_A2
	    mov  CS:WND_BL, 1
	    jmp  Granul2
Okno_A2:    mov  CS:WND_BL, 0
Granul2:
	    push DI
	    push BX
	    mov  BX,ES:[BX+4]     ; Granularita
	    cmp  BX,64
	    je   Vesa_hop2
	    mov  CL,10            ; * 1024
	    shl  BX,CL
	    mov  AX,SI
	    div  BX               ; DX:AX / BX
	    xchg AX,DX            ; DX - granule , AX offset v granuli
	    mov  SI,AX
 Vesa_hop2: mov  BH,0
	    mov  BL,CS:WND_BL
	    mov  AX,4F05h         ; VESA Bios Window control
	    ;;int  10h
	    pop  DI               ; DI<-BX
	    call DWORD PTR ES:[DI+0Ch]
	    pop  DI               ; refresh DI
	    mov  CX,CS:XORCX
	    jmp  Skip2;

   Skip1:   mov  CX,[BP+A_OFF+4]   ; barva pixlu;
   Skip2:   mov  DS:[SI],CX
;
   End_put: pop  ES         ; Obnoveni registru
	    pop  DS
	    pop  SI
	    pop  DI
	    pop  BP
;
	    ret
_xh_putpix  endp

WRTMOD DW 0
SEG64  DW 0
WND_BL DB 0
XORSI  DW 0
XORCX  DW 0

IFDEF  A_SMALL
_TEXT ENDS
ELSE
XH_PUTPX_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_wrt:word     ; XOR
	EXTRN  _xg_svga:word
	EXTRN  _xg_vesa:word
_BSS          ENDS

	END
