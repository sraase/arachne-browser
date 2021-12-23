;--------------------------------------------------------------
; Procedure for drawing of image for VESA Hi-Color modes
;--------------------------------------------------------------

; Call from "C": xh_write(unsigned char *Buf, int x, int y,
;                         int dx, int dy);
; x,y   : origin
; dx,dy : extent
; Buf   : Image in HiColor(15|16bits per pixel)

; externs of xlopif: xg_svga, xg_vesa

A_OFF    EQU    6

XG_SVGA  EQU    10   ; local var [BP-XG_SVGA]...
XG_DX    EQU    12
XG_LEN   EQU    14
XG_SG    EQU    16
XG_LSG   EQU    18

XH_WRITE_TEXT SEGMENT BYTE PUBLIC 'CODE'
	      ASSUME cs:XH_WRITE_TEXT,DS:_BSS
_xh_write   proc far
	    public _xh_write
;
	    push BP          ; Save registers
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES
;
            mov  CS:VESA64, 0       ; Default value ;; JdS 2004/10/16

	    mov  AX,SEG _xg_svga
	    mov  ES,AX
	    mov  AX,ES:[_xg_svga]   ; grf. mode
;; JdS 2004/10/16 {
            cmp  AH,80h             ; VESA mode? 0695
            jne  No_vesa
;; JdS 2004/10/16 }

	    push ES
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa     ; ES:BX address VESA struct
;; JdS 2004/10/16 {
;;	    mov  BX,[ES:BX+4]       ; Granularity
;;	    pop  ES
;;	    cmp  BX,64
;;	    je   No_gran1
;;	    mov  CS:VESA64, 1
;;	    jmp  Cont_1;
;;No_gran1: mov  CS:VESA64, 0
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
	    cmp AL,01h      ; length of line 320,640,800,1024 pixels
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

      X_1:  pop  AX           ; only Hi-color modes
      X_2:  and  AL,0E0h      ; VESA jump point ;; JdS 2004/10/16
	    cmp  AL,60h
	    je   Hi_color
	    jmp  EndErr

;------------------ Hi-color modes ----------------
;
 Hi_color:  push AX           ; [BP-0Ah] XG_SVGA

	    mov  CX,[BP+A_OFF+8]  ; number pixels on line (dx)
	    push CX           ; [BP-0Ch]  XG_DX
	    push BX           ; [BP-0Eh]  XG_LEN length line
	    sub  SP,4
	    mov  word ptr[BP-XG_SG],-1  ; Akt seg. (granul) videoram
	    mov  word ptr[BP-XG_LSG],0  ; Length of segment+1 (65536 = 0)

	    mov  DI,[BP+A_OFF]    ; ES:DI - input buffer
	    mov  DX,[BP+A_OFF+2]
	    mov  ES,DX

	    mov  AX,0A000h        ; DS:SI - adsress in videoram
	    mov  DS,AX
;
  Radek:
	    mov  CX,[BP+A_OFF+8]        ; pixels per line
	    mov  word ptr[BP-XG_DX],CX

	    mov  DX,[BP+A_OFF+6]        ; y1 - line
	    mov  AX,[BP-XG_LEN]         ; 640/1280/1600/2048
	    mul  DX
	    add  AX,[BP+A_OFF+4]
	    adc  DX,0
	    add  AX,[BP+A_OFF+4]        ; add col 2x
	    adc  DX,0                   ; AX offset, DX segment
;
	    mov  SI,AX        ; SI offset
	    push DX           ; save segment
;
	    cmp  word ptr[BP-XG_LSG],0     ; 64K segment
	    jne  Set_seg
	    cmp  word ptr[BP-XG_SG],DX    ; Set segment
	    jne  Set_seg
	    jmp  Draw

;----------------  Set segment of videoram
;
   Set_seg: cmp  word ptr[BP-XG_LSG],0
	    jne  Hop_seg           ; for < 64kB set in VESA
	    mov  [BP-XG_SG],DX
   Hop_seg: mov  AX,[BP-XG_SVGA]   ; grf. mode xg_svga
	    cmp  AH,80h
	    je   VESA
	    jmp  EndErr

;------------- VESA --------------------------
  VESA:	    push ES
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa   ; ES:BX adresa VESA struct

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
	    mov  [BP-XG_LSG],BX   ; length of segment
	    mov  AX,SI
	    div  BX               ; DX:AX / BX
	    xchg AX,DX            ; DX - granul , AX offset in granul
	    mov  [BP-XG_SG],DX
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
; Write image to videoram
;
  Draw:	    mov  CX,[BP-XG_DX] ; number of pixels (all line|remainder)
;
; Draw one line
;
  Cyk3:     mov  AX,ES:[DI]
	    mov  DS:[SI],AX
	    dec  CX
	    jz   End3
	    inc  DI    ; +2
	    inc  DI
	    inc  SI    ; +2
	    inc  SI
	    cmp  SI, word ptr [BP-XG_LSG]  ; New segm. in line
	    jne  Cyk3
;                               ; SI = 0 -> next segment
	    mov  [BP-XG_DX],CX  ; save remainder
	    mov  SI,0           ; only for seg < 64K

	    cmp  CS:VESA64, 0           ; 0695
	    je   No_granul
	    inc  word ptr[BP-XG_SG]
	    mov  DX, [BP-XG_SG]
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
	    jmp  Set_seg      ; set segment to DX
;
  End3:     pop  DX
	    inc  DI
	    inc  DI
	    inc  SI
	    inc  SI
	    inc  word ptr [BP+A_OFF+6]  ; y1++
	    dec  word ptr [BP+A_OFF+10] ; dy--
	    jz   End_lines
	    jmp  Radek

End_lines:  add  SP,10        ; dealloc local variables
;
;------------------ end ----------------------------------
     Endx:  mov  AX,1       ; O.K. end
	    jmp  Uplny

     Enderr: mov AX,-1      ; Err end

     Uplny: pop  ES         ; refresh registers
	    pop  DS
	    pop  SI
	    pop  DI
	    pop  BP
;
	    ret
_xh_write   endp

VESA64 DB  0
WND_BL DB  0

XH_WRITE_TEXT ENDS

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_svga:word
	EXTRN  _xg_vesa:word
	EXTRN  _xg_wrt:word
	EXTRN  _xg_chrmod:word
_BSS          ENDS
	END
