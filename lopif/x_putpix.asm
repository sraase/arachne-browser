;-----------------------------------------------------------
; Procedure for drawing pixels in 256 colors
;-----------------------------------------------------------

include svga_mac.inc

IFDEF  A_SMALL
A_OFF    EQU    4
ELSE
A_OFF    EQU    6
ENDIF

IFDEF  A_SMALL
	 EXTRN  _x_svgaw:NEAR
	 EXTRN  _x_svgar:NEAR
ELSE
	 EXTRN  _x_svgaw:FAR
	 EXTRN  _x_svgar:FAR
ENDIF

IFDEF  A_SMALL
_TEXT SEGMENT BYTE PUBLIC 'CODE'
      ASSUME CS:_TEXT,DS:_BSS
_x_putpix_v   proc near
ELSE
X_PUTPIX_TEXT SEGMENT BYTE PUBLIC 'CODE'
	      ASSUME cs:X_PUTPIX_TEXT,DS:_BSS
_x_putpix_v   proc far
ENDIF
	    public _x_putpix_v
;
	    push BP          ; Save registers (?AX..DX)
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES
;
; Call from C: x_putpix(int x, int y, int color)
;
IFDEF  A_SMALL
	    mov  AX,[_xg_svga]
ELSE
	    mov  AX,SEG _xg_svga
	    mov  ES,AX
	    mov  AX,ES:[_xg_svga]   ; grf.mode
ENDIF
	    push AX
;; JdS 2004/10/10 {
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
;; JdS 2004/10/10 }
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

    Go320:  mov BX,320
	    jmp X_1
    Go640:  mov BX,640
	    jmp X_1
    Go800:  mov BX,800
	    jmp X_1
   Go1024:  mov BX,1024

;; JdS 2004/10/10 {
;;    X_1:  pop AX
;;	    and AL,0E0h
;;	    cmp AL,80h ; 256
;;	    je  Dale256
;;	    cmp AL,40h ;  16
;;	    je  Dale16
;;	    cmp AL,20h ;   2
;;	    je  Dale2
;;
;; Dale16:  shr BX,1   ; /8 for 16 colors
;;	    shr BX,1
;;	    shr BX,1
;;	    jmp G_16
; Check colour option, non-Vesa mode
      X_1:  pop  AX
	    and  AL,0E0h  ; select 3 bits
	    cmp  AL,80h   ; 256 colour
	    je   Dale256
	    shr  BX,1     ; if not 256 colour (ie. 2/16 colour),
	    shr  BX,1     ;   revise previous bytes/line guess
	    shr  BX,1     ;   (divide it by eight)
	    jmp  X_3
; Check colour option, Vesa mode
      X_2:  pop  AX
	    and  AL,0E0h  ; select 3 bits
	    cmp  AL,80h   ; 256 colour
	    je   Dale256
; Check colour option, common code
      X_3:  cmp  AL,40h   ; 16 colour
	    je   Dale16
	    cmp  AL,20h   ; 2 colour
	    je   Dale2

   Dale16:  jmp G_16

;; JdS 2004/10/10 }

    Dale2:  jmp G_2

;------ 256 -------------------------------------------------
  Dale256:
	    push AX           ; xg_svga
IFDEF A_SMALL
	    mov  AX,[_xg_wrt]
	    mov  CS:WRTMOD,AX
ELSE
	    mov  AX,SEG _xg_wrt
	    mov  ES,AX
	    mov  AX,ES:[_xg_wrt]   ; XOR
	    mov  CS:WRTMOD,AX
ENDIF
	    mov  AX,0A000h    ; DS:SI - address in videoram
	    mov  DS,AX
;
	    mov  DX,[BP+A_OFF+2]  ; line
	    mov  AX,BX
	    mul  DX
	    add  AX,[BP+A_OFF]    ; add col
	    adc  DX,0             ; in AX offset, in DX segment (! for 64kB)
	    mov  CS:SEG64,DX
;
	    mov  SI,AX        ; in SI offset
	    mov  CS:XORSI,SI
;
; Set segment videoram :
;
	    pop  AX            ; xg_svga
	    mov  CS:SVGAT,AH
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
M1:         push DX
	    mov  AX,87A7h
	    mov  DX,3C4h
	    out  DX,AX
	    pop  DX
	    cmp  CS:WRTMOD,0
	    jne  For_read
	    mov  AH,DL
	    mov  AL,0C5h
	    mov  DX,3C4h
	    out  DX,AX
	    jmp  Draw
For_read:   shl  DL,1
	    shl  DL,1
	    shl  DL,1
	    shl  DL,1
	    mov  AH,DL
	    mov  AL,0C5h
	    mov  DX,3C4h
	    out  DX,AX
	    mov  AX,00A7H
	    out  DX,AX
	    jmp  Draw
;------------- VESA standart -----------------
  VESA:
IFDEF A_SMALL
	    lea BX,_xg_vesa
ELSE
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa   ; ES:BX address VESA struct
ENDIF
	    mov  CL,ES:[BX+2]     ; Window for WRITE/READ
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
	    pop  DI               ; refresh DI,DX
	    pop  DX
;-------------------------------------------------------
; Write pixel to videoram
;
   Draw:
	    mov  AX,CS:WRTMOD
	    cmp  AX,0
	    jne  ReaVideo
	    jmp  Skip1

ReaVideo:   mov  CL,DS:[SI]      ; read videoram
	    xor  CL,[BP+A_OFF+4]
	    mov  SI,CS:XORSI

	    cmp  CS:SVGAT,40h   ; M1
	    je   M1_write
	    cmp  CS:SVGAT,80h   ; VESA
	    jne  Skip2;

	    mov  CH,CL          ; !!! spec for VESA save CL
IFDEF A_SMALL
	    lea BX,_xg_vesa
ELSE
	    mov  BX,SEG _xg_vesa
	    mov  ES,BX
	    lea  BX,ES:_xg_vesa   ; ES:BX address VESA struct
ENDIF
	    mov  CL,ES:[BX+2]     ; Window for WRITE/READ
	    and  CL,05h           ; WRITE
	    cmp  CL,05h
	    je   Okno_A2
	    mov  CS:WND_BL, 1
	    jmp  Granul2
Okno_A2:    mov  CS:WND_BL, 0
Granul2:
	    push DI
	    push BX
	    mov  BX,ES:[BX+4]     ; Granularity
	    cmp  BX,64
	    je   Vesa_hop2
	    mov  CL,10            ; * 1024
	    shl  BX,CL
	    mov  AX,SI
	    div  BX               ; DX:AX / BX
	    xchg AX,DX            ; DX - granul , AX offset in granul
	    mov  SI,AX
 Vesa_hop2: mov  BH,0
	    mov  BL,CS:WND_BL
	    mov  AX,4F05h         ; VESA Bios Window control
	    ;;int  10h
	    pop  DI               ; DI<-BX
	    call DWORD PTR ES:[DI+0Ch]
	    pop  DI               ; refresh DI
	    mov  CL,CH
	    jmp  Skip2;

M1_write:   mov  AX,87A7h      ; !!! SPEC for M1
	    mov  DX,3C4h
	    out  DX,AX
	    mov  DX,CS:SEG64
	    mov  AH,DL
	    mov  AL,0C5h
	    mov  DX,3C4h
	    out  DX,AX
	    jmp  Skip2

   Skip1:   mov  CL,[BP+A_OFF+4]   ; color pixel;
   Skip2:   mov  DS:[SI],CL
	    jmp  End_put
;
;---------------------- 16 colors modes ----------------
   G_16:    push AX           ; xg_svga
	    mov  AX,0A000h    ; DS:SI - adress in videoram
	    mov  DS,AX
;
	    mov  DX,3ceh      ; Write_mod 2
	    mov  AL,5
	    out  DX,AL
	    inc  DX           ; in DX adr 3cfh
	    mov  AL,2
	    out  DX,AL
	    dec  DX           ; back to 3ceh
;
	    mov  AX,[BP+A_OFF]  ; Pixel in byte ( % 8)
	    and  AX,7
	    mov  CL,AL
	    mov  CH,80h
	    shr  CH,CL

	    mov  AL,8         ; Set video reg
	    out  DX,AL        ; to 3ceh
	    inc  DX
	    mov  AL,CH
	    out  DX,AL

	    push BX
	    mov  DX,[BP+A_OFF+2]  ; lien
	    mov  AX,BX        ; 320/640/800/1024 pixels /8
	    mul  DX
	    mov  BX,[BP+A_OFF]  ; add column/8
	    shr  BX,1
	    shr  BX,1
	    shr  BX,1
	    add  AX,BX
	    adc  DX,0         ; in AX offset, in DX segment (1024)
	    mov  SI,AX

; For 1024 pixel per line call set segment
	    pop  BX
	    pop  AX           ; xg_svga
	    cmp  BX,128
	    jl   No_segm
	    cmp  AH,80h       ; VESA
	    je   WrtRea
	    call _x_svgaw
	    jmp  No_segm
WrtRea:     push AX           ; VESA
	    push DX
	    call _x_svgaw
	    pop  DX
	    pop  AX
	    call _x_svgar

  No_segm:  mov  AL,DS:[SI]
	    mov  AL,[BP+A_OFF+4]  ; Color
	    mov  DS:[SI],AL
;
; Set original write mode EGA/VGA (mode 0)
	    mov  DX,3ceh
	    mov  AL,8
	    out  DX,AL
	    inc  DX
	    mov  AL,0ffh
	    out  DX,AL
	    dec  DX
;
	    mov  AL,5         ; Mode registr
	    out  DX,AL
	    inc  DX
	    mov  AL,0
	    out  DX,AL
	    jmp  End_put
;
;------------------- Binary BCGA ------------------------
   G_2:
IFDEF A_SMALL
	    mov  AX,[_xg_wrt]
	    mov  CS:WRTMOD,AX
ELSE
	    mov  AX,SEG _xg_wrt
	    mov  ES,AX
	    mov  AX,ES:[_xg_wrt]   ; XOR
	    mov  CS:WRTMOD,AX
ENDIF
;; JdS 2004/10/10 {
;;	    shr  BX,1           ; / 8 length line in byte (80)
;;	    shr  BX,1
;;	    shr  BX,1
;; JdS 2004/10/10 }
	    mov  AX,0B800h      ; DS:SI - addres in videoram
	    mov  DS,AX
;
	    mov  AX,[BP+A_OFF]  ; pixel in byte ( % 8)
	    and  AX,7
	    mov  CL,AL
	    mov  CH,80h
	    shr  CH,CL          ; CH - Bit for write
	    push CX

	    mov  DX,[BP+A_OFF+2]  ; line
	    test DX,1
	    je   Sudy
	    mov  CL,1
	    jmp  Nasob
     Sudy:  mov  CL,0

    Nasob:  shr  DX,1           ; / 2
	    mov  AX,BX          ; 320/640/800 pixlu na radek/8
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
	    mov  AL,[BP+A_OFF+4]  ; Color 0-black,else white
	    mov  AH,DS:[SI]       ; load byte from video

	    cmp  CS:WRTMOD,0
	    jne  WrXOR

	    ;;cmp  AL,0            ; COPY pixel
	    and  AL,1            ; odd-white, even-black pixel
	    ;;je   Hop0
	    jz   Hop0
	    or   AH,CH           ; write bit to AH
	    jmp  Write
   Hop0:    not  CH              ; release bit in AH
	    and  AH,CH
	    jmp  Write

   WrXOR:   ;;cmp  AL,0            ; XOR pixel
	    and  AL,1
	    ;;jne  Hop2
	    jnz  Hop2
	    mov  CH,0
     Hop2:  xor  AH,CH

   Write:   mov  DS:[SI],AH      ; zapis
;
	    mov  AX,1            ; ret val
;------------ Common end ----------------------
   End_put: pop  ES         ; Refresh registers
	    pop  DS
	    pop  SI
	    pop  DI
	    pop  BP
;
	    ret
_x_putpix_v   endp

WRTMOD DW 0
SEG64  DW 0
SVGAT  DB 0
WND_BL DB 0
XORSI  DW 0

IFDEF  A_SMALL
_TEXT ENDS
ELSE
X_PUTPIX_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_wrt:word     ; XOR
	EXTRN  _xg_svga:word
	EXTRN  _xg_vesa:word
_BSS          ENDS
	END
