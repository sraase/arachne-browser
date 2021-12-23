;--------------------------------------------------------------
; Procedure for reading pixels from videoram in Hi-Color modes
;--------------------------------------------------------------

;include svga_mac.inc

IFDEF  A_SMALL
A_OFF    EQU    4
ELSE
A_OFF    EQU    6
ENDIF

IFDEF  A_SMALL
_TEXT SEGMENT BYTE PUBLIC 'CODE'
      ASSUME CS:_TEXT,DS:_BSS
_xh_getpix  proc near
ELSE
XH_GETPX_TEXT SEGMENT BYTE PUBLIC 'CODE'
	      ASSUME cs:XH_GETPX_TEXT,DS:_BSS
_xh_getpix  proc far
ENDIF
	    public _xh_getpix
;
	    push BP          ; Uschova registru (?AX..DX)
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES
;
; Call from C: color = xh_getpix(int x, int y)
;
	    mov  AX,SEG _xg_svga
	    mov  ES,AX
	    mov  AX,ES:[_xg_svga]   ; grf. mode
	    push AX
;; JdS 2004/10/04 {
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
;; JdS 2004/10/04 }

	    and AL,1Fh
	    cmp AL,01h      ; length of line 320,640,800,1024
	    je  Go320;
	    cmp AL,02h
	    je  Go640
	    cmp AL,04h
	    je  Go800
	    cmp AL,08h
	    je  Go1024

;; JdS 2004/10/04 {
	    pop AX
	    jmp End_nic
;; JdS 2004/10/04 }

    Go320:  mov BX,640
	    jmp X_1
    Go640:  mov BX,1280
	    jmp X_1
    Go800:  mov BX,1600
	    jmp X_1
   Go1024:  mov BX,2048

      X_1:  pop AX
	    and AL,0E0h
	    cmp AL,60h
	    je  Dale     ; Hi - colors

	    jmp End_nic

;--------------- Hi colors ------------------------------
    Dale:   push AX           ; xg_svga
	    mov  AX,0A000h    ; DS:SI - address in videoram
	    mov  DS,AX
;
	    mov  DX,[BP+A_OFF+2]  ; line
	    mov  AX,BX
	    mul  DX
	    add  AX,[BP+A_OFF]  ; add col
	    adc  DX,0           ; AX offset, DX segment
	    add  AX,[BP+A_OFF]  ; add col
	    adc  DX,0           ; AX offset, DX segment
;
	    mov  SI,AX          ; SI offset
;
; Set segment videoram
	    pop  AX             ; xg_svga
	    cmp  AH,80h
	    je   VESA
	    jmp  End_nic

;------------- VESA standart -----------------
  VESA:	    mov  BX,SEG _xg_vesa
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
	    pop  DI               ; refresh DI
;---------------------------------------------------
;
; Read pixel from videoram
;
   Draw:    mov  AX,DS:[SI]   ; color in AX - return value
;------------ End ----------------------------------
End_nic:    pop  ES         ; refresh registers
	    pop  DS
	    pop  SI
	    pop  DI
	    pop  BP
;
	    ret
_xh_getpix  endp

WND_BL  DB  0

IFDEF  A_SMALL
_TEXT  ENDS
ELSE
XH_GETPX_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_svga:word
	EXTRN  _xg_vesa:word
_BSS          ENDS
	END
