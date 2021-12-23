;-----------------------------------------------------
; Procedure for drawing fill rect in Hi-colors
; Draw from point XZ,YZ and drawing DX cols, DY rows
; with color COL
; Calling parameters: "C" language
;-----------------------------------------------------

;include svga_mac.inc
;
IFDEF  A_SMALL
A_OFF    EQU    4
ELSE
A_OFF    EQU    6
ENDIF
;
; Call from C: xh_barx(col,xz,yz,dx,dy)
;
IFDEF  A_SMALL
_TEXT SEGMENT BYTE PUBLIC 'CODE'
      ASSUME cs:_TEXT,DS:_BSS
_xh_barx proc near
ELSE
XH_BARX_TEXT SEGMENT BYTE PUBLIC 'CODE'
	     ASSUME cs:XH_BARX_TEXT,DS:_BSS
_xh_barx proc far
ENDIF
	    public _xh_barx
;
	    push BP          ; Save registers (?AX..DX)
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES

	    sub  SP,6        ; Local variables from [BP-0Ah]
	    mov  word ptr[BP-0Ah],-1 ; Aktual seg. (granul) videoram
	    mov  word ptr[BP-0Ch],0  ; Length of segment+1 (65536 = 0)
	    mov  word ptr[BP-0Eh],0  ; Length of line

	    mov  AX,0A000h    ; DS:SI - address of videoram
	    mov  DS,AX
;
            mov  CS:VESA64, 0       ; Default value ;; JdS 2004/10/10

	    mov  AX,SEG _xg_svga
	    mov  ES,AX
	    mov  AX,ES:[_xg_svga]   ; grf.mode + hardware
	    cmp  AH,80h             ; is this VESA mode? 0695
	    jne  No_vesa

	    push ES
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa     ; ES:BX address VESA struct
;; JdS 2004/10/10 {
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
Chk_BpL:    mov word ptr[BP-0Eh],BX ; Store bytes/line (should be known)
	    cmp  BX,0               ; If the VESA bytes/line is unknown,
            jne  Radek              ;   make best guess from graphics mode

No_vesa:
;; JdS 2004/10/10 }

 Cont_1:    and AL,1Fh
	    cmp AL,01h      ; length of line 320,640,800,1024
	    je  Go320;
	    cmp AL,02h
	    je  Go640
	    cmp AL,04h
	    je  Go800
	    cmp AL,08h
	    je  Go1024
	    jmp  Errend

    Go320:  mov word ptr[BP-0Eh],640
	    jmp Radek
    Go640:  mov word ptr[BP-0Eh],1280
	    jmp Radek
    Go800:  mov word ptr[BP-0Eh],1600
	    jmp Radek
   Go1024:  mov word ptr[BP-0Eh],2048
;	    jmp  Errend
;
  Radek:    mov  DI,[BP+A_OFF+6]      ; number of pixels (->CX)
	    mov  DX,[BP+A_OFF+4]      ; line
	    mov  AX,word ptr[BP-0Eh]  ; 1024/640/800/320 pixels
	    mul  DX
	    add  AX,[BP+A_OFF+2]  ; add column 2x
	    adc  DX,0             ; AX offset, DX segment
	    add  AX,[BP+A_OFF+2]
	    adc  DX,0
;
	    mov  SI,AX        ; SI offset
	    push DX           ; save segment

	    cmp  word ptr[BP-0Ch],0     ; 64K segment
	    jne  Set_seg
	    cmp  word ptr[BP-0Ah],DX    ; Set segment videoram
	    jne  Set_seg
	    jmp  Draw
;
;------------- Set segment videoram
;
   Set_seg: cmp  word ptr[BP-0Ch],0
	    jne  Hop_seg            ; for < 64kB sets in VESA
	    mov  [BP-0Ah],DX
   Hop_seg: mov  AX,ES:[_xg_svga]   ; grf. mode
	    cmp  AH,80h
	    je   VESA
	    jmp  Errend

;------------- VESA --------------------------
  VESA:	    push ES
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa   ; ES:BX addr of VESA struct

	    mov  CL,ES:[BX+2]     ; Window for WRITE
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
	    mov  [BP-0Ch],BX
	    mov  AX,SI
	    div  BX               ; DX:AX / BX
	    xchg AX,DX            ; DX - granule , AX offset v granuli
	    mov  [BP-0Ah],DX
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
;
; Write rect to videoram
;
  Draw:	    mov  CX,DI         ; number of pixels
	    mov  AX,[BP+A_OFF] ; color
; Draw one line
  Cykl:
	    mov  DS:[SI],AX
	    dec  CX
	    jz   Endx
	    inc  SI
	    inc  SI
	    cmp  SI, word ptr [BP-0Ch]  ; New segm. in line
	    jne  Cykl
;
	    mov  DI,CX
	    mov  SI,0                   ; Only for segm. < 64KB !!

	    cmp  CS:VESA64, 0           ; 0695
	    je   No_granul
	    inc  word ptr[BP-0Ah]
	    mov  DX, [BP-0Ah]
	    push ES
	    mov  BX,SEG _xg_vesa  ; !--- 0397
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa   ; ES:BX adresa VESA struct
	    push DI
	    push BX               ; !---
	    jmp  Vesa_hop

 No_granul: pop  DX
	    inc  DX
	    push DX          ; Segment (granul) + 1
	    jmp  Set_seg
;
; Next line
  Endx:     pop  DX
	    inc  SI
	    inc  SI
	    inc  word ptr [BP+A_OFF+4] ; y1++
	    dec  word ptr [BP+A_OFF+8] ; dy--
	    jz   Errend
	    jmp  Radek
  Errend:
	    add  SP,6        ; Dealoc local variable

	    pop  ES          ; Refresh registers
	    pop  DS
	    pop  SI
	    pop  DI
	    pop  BP
;
	    ret
_xh_barx    endp

WND_BL  DB  0
VESA64  DB  0   ; for VESA with granul < 64

IFDEF  A_SMALL
_TEXT ENDS
ELSE
XH_BARX_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_svga:word
	EXTRN  _xg_vesa:word
_BSS          ENDS
      END
