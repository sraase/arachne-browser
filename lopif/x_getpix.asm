;-----------------------------------------------------------
; Procedure for reading pixels from videoram
;-----------------------------------------------------------

include svga_mac.inc

IFDEF  A_SMALL
A_OFF    EQU    4
ELSE
A_OFF    EQU    6
ENDIF

EGAPLANE    MACRO  ARG1     ; Set bit plane 1,2,4,8
	    mov  AL,4
	    mov  DX,03CEh
	    out  DX,AL
	    inc  DX
	    mov  AL,ARG1
	    out  DX,AL
	    ENDM

IFDEF  A_SMALL
	 EXTRN  _x_svgar:NEAR
ELSE
	 EXTRN  _x_svgar:FAR
ENDIF

IFDEF  A_SMALL
_TEXT SEGMENT BYTE PUBLIC 'CODE'
      ASSUME CS:_TEXT,DS:_BSS
_x_getpix_v   proc near
ELSE
X_GETPIX_TEXT SEGMENT BYTE PUBLIC 'CODE'
	      ASSUME cs:X_GETPIX_TEXT,DS:_BSS
_x_getpix_v   proc far
ENDIF
	    public _x_getpix_v
;
	    push BP          ; Uschova registru (?AX..DX)
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES
;
; Call from C: color = x_getpix(int x, int y)
;
	    mov  AX,SEG _xg_svga
	    mov  ES,AX
	    mov  AX,ES:[_xg_svga]   ; grf. mode
	    push AX
;; JdS 2004/10/04 {
            cmp  AH,80h             ; VESA mode? 0695
            jne  Cont_1

            push ES
            mov  BX,SEG _xg_vesa
            mov  ES,BX
            lea  BX,ES:_xg_vesa     ; ES:BX address VESA struct
            mov  BX,ES:[BX+20]      ; Bytes/line
            pop  ES

	    cmp  BX,0           ; If the VESA bytes/line is unknown,
            jne  X_2            ;   make best guess from graphics mode

Cont_1:
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

    Go320:  mov BX,320
	    jmp X_1
    Go640:  mov BX,640
	    jmp X_1
    Go800:  mov BX,800
	    jmp X_1
   Go1024:  mov BX,1024

;; JdS 2004/10/04 {
;;    X_1:  pop AX
;;	    and AL,0E0h
;;	    cmp AL,80h
;;	    je  Dale     ; 256 colors
;;
;;	    cmp AL,40h   ; 16 colors
;;	    je  Dale16
;;	    cmp AL,20h
;;	    je  Dale_2   ;  2 colors
;;
;; Dale16:  shr BX,1
;;	    shr BX,1
;;	    shr BX,1
;;	    jmp G_16
; Check colour option, non-Vesa mode
      X_1:  pop  AX
	    and  AL,0E0h  ; select 3 bits
	    cmp  AL,80h   ; 256 colour
	    je   Dale     
	    shr  BX,1     ; if not 256 colour (ie. 2/16 colour),
	    shr  BX,1     ;   revise previous bytes/line guess
	    shr  BX,1     ;   (divide it by eight)
	    jmp  X_3
; Check colour option, Vesa mode
      X_2:  pop  AX
	    and  AL,0E0h  ; select 3 bits
	    cmp  AL,80h   ; 256 colour
	    je   Dale
; Check colour option, common code
      X_3:  cmp  AL,40h   ; 16 colour
	    je   Dale16
	    cmp  AL,20h   ; 2 colour
	    je   Dale_2  

   Dale16:  jmp G_16

;; JdS 2004/10/04 }

   Dale_2:  jmp G_2

;--------------- 256 colors ------------------------------
    Dale:   push AX           ; xg_svga
	    mov  AX,0A000h    ; DS:SI - address in videoram
	    mov  DS,AX
;
	    mov  DX,[BP+A_OFF+2]  ; line
	    mov  AX,BX
	    mul  DX
	    add  AX,[BP+A_OFF]  ; add col
	    adc  DX,0           ; AX offset, DX segment
;
	    mov  SI,AX          ; SI offset
;
; Set segment videoram
;
	    pop  AX            ; xg_svga
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

   Sh320:   jmp  Draw

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
;-------------------------------------------------------
;
; Read pixel from videoram
;
   Draw:    mov  AL,DS:[SI]   ; color in AX - return value
	    mov  AH,0
	    jmp  End_nic

;---------------------- 16 color modes----------------
   G_16:    push AX           ; xg_svga
	    mov  AX,0A000h    ; DS:SI - address videoram
	    mov  DS,AX

	    mov  AX,[BP+A_OFF]  ; pixel in byte ( % 8)
	    and  AX,7
	    mov  CL,AL
	    mov  CH,80h
	    shr  CH,CL        ; in CH is pixel in byte

	    push BX
	    mov  DX,[BP+A_OFF+2]  ; line
	    mov  AX,BX            ; 320/640/800/1024 ( /8)
	    mul  DX
	    mov  BX,[BP+A_OFF]  ; add col /8
	    shr  BX,1
	    shr  BX,1
	    shr  BX,1
	    add  AX,BX
	    adc  DX,0         ; AX offset, DX segment
;
	    mov  SI,AX        ; SI offset  [video DS:SI]

	    pop  BX           ; Length line in bytes
	    pop  AX           ; xg_svga
	    cmp  BX,128
	    jl   No_segm
	    call _x_svgar

  No_segm:  xor  CL,CL        ; in CL color of pixel

	    EGAPLANE 0        ; destroy AL,DX
	    mov  BH,DS:[SI]
	    test BH,CH
	    jz   Ne1
	    inc  CL
      Ne1:  EGAPLANE 1
	    mov  BH,DS:[SI]
	    test BH,CH
	    jz   Ne2
	    add  CL,2
      Ne2:  EGAPLANE 2
	    mov  BH,DS:[SI]
	    test BH,CH
	    jz   Ne3
	    add  CL,4
      Ne3:  EGAPLANE 3
	    mov  BH,DS:[SI]
	    test BH,CH
	    jz   Ne4
	    add  CL,8

      Ne4:  xor  AH,AH     ; return color
	    mov  AL,CL
	    jmp  End_nic

;------------------- 2 colors---------------------------
      G_2:
;; JdS 2004/10/04 {
;;	    shr  BX,1           ; / 8 length line in byte (80)
;;	    shr  BX,1
;;	    shr  BX,1
;; JdS 2004/10/04 }
	    mov  AX,0B800h      ; DS:SI - addres in videoram
	    mov  DS,AX
;
	    mov  AX,[BP+A_OFF]  ; pixel in byte ( % 8)
	    and  AX,7
	    mov  CL,AL
	    mov  CH,80h
	    shr  CH,CL          ; CH - Bit for read
	    push CX

	    mov  DX,[BP+A_OFF+2]  ; line
	    test DX,1
	    je   Sudy
	    mov  CL,1
	    jmp  Nasob
     Sudy:  mov  CL,0

    Nasob:  shr  DX,1           ; / 2
	    mov  AX,BX
	    mul  DX

	    cmp  CL,0
	    je   Pricti
	    add  AX,2000h       ; odd line

   Pricti:  mov  BX,[BP+A_OFF]  ; add col/8
	    shr  BX,1
	    shr  BX,1
	    shr  BX,1
	    add  AX,BX
	    adc  DX,0
	    mov  SI,AX          ; DS:SI - Addres of byte in videoram

	    pop  CX
	    mov  AH,DS:[SI]       ; load byte from video

	    and  AH,CH           ; is bit for get 0|1?
	    cmp  AH,0
	    je   Ret_0
	    mov  AX,1
	    jmp  End_nic;
  Ret_0:    mov  AX,0

;------------ Common End --------------------------
End_nic:    pop  ES         ; refresh registers
	    pop  DS
	    pop  SI
	    pop  DI
	    pop  BP
;
	    ret
_x_getpix_v   endp

WND_BL  DB  0

IFDEF  A_SMALL
_TEXT  ENDS
ELSE
X_GETPIX_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_svga:word
	EXTRN  _xg_vesa:word
_BSS          ENDS
	END
