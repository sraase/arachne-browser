;-----------------------------------------------------------
; Procedure for setting segment of videoram SVGA (write)
;-----------------------------------------------------------

include svga_mac.inc

IFDEF  A_SMALL
_TEXT SEGMENT BYTE PUBLIC 'CODE'
      ASSUME CS:_TEXT,DS:_BSS
_x_svgaw    proc near
ELSE
X_SVGAW_TEXT SEGMENT BYTE PUBLIC 'CODE'
	     ASSUME cs:X_SVGAW_TEXT,DS:_BSS
_x_svgaw    proc far
ENDIF
	    public _x_svgaw
;
; Call only from assembler !!!
; Parameters in REG :  AX - xg_svga; DX - number of segment
; Destroy : CX,BX
;
	    cmp  AH,00
	    je   Sh320         ; 320 x 200
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
	    jne  Sh320
	    jmp  VESA

    Sh320:  jmp  Draw

;------------- TSENG 3000 ---------------------
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
;------------- M1 ---------------------------- (16 colors?)
M1:
	    SEG_M1W
	    jmp Draw
;------------- VESA standart -----------------
  VESA:     push ES
IFDEF A_SMALL
	    lea  BX,_xg_vesa
ELSE
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa   ; ES:BX addr VESA struct
ENDIF
	    mov  CL,ES:[BX+2]     ; Search window for WRITE
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
	    mov  AX,SI
	    div  BX               ; DX:AX / BX
	    xchg AX,DX            ; DX - granul , AX offset in granul
	    mov  SI,AX
 Vesa_hop:  mov  BH,0
	    mov  BL,CS:WND_BL
	    mov  AX,4F05h         ; VESA Bios Window control
	    ;;int  10h
	    pop  DI               ; DI<-BX
	    call DWORD PTR ES:[DI+0Ch]
	    pop  DI               ; refresh DI,ES
	    pop  ES
;
   Draw:    ret
_x_svgaw    endp

WND_BL  DB  0

IFDEF  A_SMALL
_TEXT ENDS
ELSE
X_SVGAW_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_vesa:word
_BSS          ENDS
     END
