;-----------------------------------------------------
; Procedure for drawing raster char on EGA\VGA\SVGA
;-----------------------------------------------------

include svga_mac.inc
;
SET_SVGA_SEG MACRO
	    push AX
	    push BX
	    push CX
	    push DX
	    mov  DX,CS:VSEGM  ; Segment
	    mov  AX,CS:SVGAA  ; xg_svga
	    call _x_svgaw
	    pop  DX
	    pop  CX
	    pop  BX
	    pop  AX
	    ENDM

SET_SVGA_REA MACRO
	    push AX
	    push BX
	    push CX
	    push DX
	    mov  DX,CS:VSEGM  ; Segment
	    mov  AX,CS:SVGAA  ; xg_svga
	    call _x_svgar
	    pop  DX
	    pop  CX
	    pop  BX
	    pop  AX
	    ENDM

IFDEF  A_SMALL
A_OFF    EQU    4
A_SHF    EQU    0
ELSE
A_OFF    EQU    6
A_SHF    EQU    2
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
      ASSUME cs:_TEXT,DS:_DATA,_BSS
_wrt_chr    proc near
ELSE
WRT_CHR_TEXT SEGMENT BYTE PUBLIC 'CODE'
	     ASSUME cs:WRT_CHR_TEXT,DS:_DATA,_BSS
_wrt_chr    proc far
ENDIF
	    public _wrt_chr
;
	    push BP
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES
;
; Call from C: wrt_chr(char *buf,int xpix,ypix,ncol,nrow,col1,col2);
; col1,col2 - color of foreground, background of char
;
            mov  CS:VESA64, 0        ; Default value ;; JdS 2004/09/27
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
            jne  X_2            ;   make best guess from graphics mode
No_vesa:
;; JdS 2004/09/27 }

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

    Go320:  mov BX,320
	    jmp X_1
    Go640:  mov BX,640
	    jmp X_1
    Go800:  mov BX,800
	    jmp X_1
   Go1024:  mov BX,1024

;; JdS 2004/10/03 {
;;    X_1:  pop  AX
;;	    and  AL,0E0h  ; 3 bits
;;	    cmp  AL,40h   ; 16 color
;;	    je   Hup16
;;	    cmp  AL,80h   ; 256 color
; Check colour option, non-Vesa mode
      X_1:  pop  AX
	    and  AL,0E0h  ; select 3 bits
	    cmp  AL,40h
	    jne   X_3     ; if 16 colour mode,
	    shr  BX,1     ;   revise previous bytes/line guess
	    shr  BX,1     ;   (divide by eight)
	    shr  BX,1
	    jmp  Hup16
; Check colour option, Vesa mode
      X_2:  and  AL,0E0h  ; select 3 bits
	    cmp  AL,40h   ; 16 colour
	    je   Hup16
; Check colour option, common code
      X_3:  cmp  AL,80h   ; 256 colour
;; JdS 2004/10/03 }
	    je   To256
	    cmp  AL,20    ; 2 color
	    je   To2;

   To256:  jmp  R256   ;-------->>> 256 color modes
   To2:    jmp  BinCol ;-------->>> binary

;-------------------------- 16 color mode -------------------
;; JdS 2004/10/03 {
;;   Hup16: shr  BX,1
;;	    shr  BX,1
;;	    shr  BX,1
;;	    mov  CX,BX  ; cols in bytes
     Hup16: mov  CX,BX  ; cols in bytes
;; JdS 2004/10/03 }
	    mov  CS:ROWLL,BX
	    mov  CS:SVGAA,AX
	    mov  CS:SVGAV,AH

	    mov  DI,[BP+A_OFF]    ; ES:DI - input bufer with image of char
IFDEF  A_SMALL
	    mov  DX,DS
ELSE
	    mov  DX,[BP+A_OFF+2]
ENDIF
	    mov  ES,DX

   Dale:    mov  AX,[BP+A_OFF+A_SHF+6]   ; NCOL/8
	    shr  AX,1
	    shr  AX,1
	    shr  AX,1
	    push AX           ; on stack [BP-ah]
;
	    push CX

; Begin image in videoram and next constants
	    mov  AX,[BP+A_OFF+A_SHF+4]  ; ypix
	    mov  DX,CX
	    mul  DX           ; begin byte of line in BX
	    mov  BX,AX
	    mov  AX,[BP+A_OFF+A_SHF+2]  ; xpix
	    mov  CH,8
	    div  CH           ; xpix/8 in AL, remainder in AH
	    mov  CL,AH
	    sub  AH,AH
	    add  BX,AX
	    adc  DX,0
	    mov  CS:VSEGM,DX

	    mov  SI,BX
;
	    pop  BX           ; in BX bytes per line
	    push SI           ; on stack [BP-ch]
;
	    mov  DX,[BP+A_OFF+A_SHF+8]  ; # of lines
	    push DX           ; on stack [BP-eh]
;
	    mov  AL,1         ; 8 pixels/byte (mask and shift)
	    mov  AH,1
	    push AX           ; on stack [BP-10h]

	    push BX           ; Bytes per line in [BP-12h]
;
; Set write modu EGA/VGA  2
	    mov  DX,0A000h
	    mov  DS,DX
;
	    mov  DX,3ceh
	    mov  AL,5
	    out  DX,AL
	    inc  DX           ; in DX adr 3cfh
	    mov  AL,2
	    out  DX,AL
	    dec  DX           ; back 3ceh
;
	    mov  BL,80h       ; shift bit
	    shr  BL,CL
	    mov  AH,80h       ; cycle of 8

	    mov  CX,[BP-10h]  ; shift packed pixels
	    xchg CL,CH
;
; Write inp buffer in ES:DI to videoram on DS:SI
	    cmp  CS:ROWLL,128  ; for 1024
	    jl   cykl_r
	    SET_SVGA_SEG
	    cmp  CS:SVGAV,80h  ; for VESA
	    jne  cykl_r
	    SET_SVGA_REA
;
  cykl_r:   mov  BH,ES:[DI]
	    mov  CH,8

	    cmp  WORD PTR [BP-0Ah],0
	    je   Zbytek_x
;
  cykl:     mov  AL,BH
	    and  AL,80h         ; Mask
	    je   poza
	    mov  AL,[BP+A_OFF+A_SHF+0Ah]
	    mov  CS:[POZADI],0
	    jmp  A_1
   poza:    mov  AL,[BP+A_OFF+A_SHF+0Ch]
	    push CS:[CHRMOD]
	    pop  CS:[POZADI]
   A_1:	    push AX
	    dec  CH           ; 8
	    jne  hopla1
	    inc  DI
	    mov  BH,ES:[DI]
	    mov  CH,8
	    jmp  hopla2
  hopla1:
	    shl  BH,CL        ; bits in pixel
  hopla2:
	    mov  AL,8         ; Set video. reg
	    out  DX,AL        ; on 3ceh
	    inc  DX
	    mov  AL,BL
	    out  DX,AL
	    dec  DX
	    mov  AL,[SI]      ; read latch registers
	    pop  AX
	    cmp  CS:[POZADI],0
	    jne  Nepi_01
	    mov  [SI],AL      ; write pixel

  Nepi_01:  shr  BL,1
	    jne  nicevo
	    inc  SI
	    mov  BL,80h

  nicevo:   shr  AH,1
	    jne  cykl         ; Cycle per 8 pixels
;
	    mov  AH,80h
	    dec  WORD PTR [BP-0ah]
	    jne  cykl         ; Cycle per line
;
;------------ remainder ----------------
  Zbytek_x:
	    mov  AX,[BP+A_OFF+A_SHF+6]
	    and  AX,7
	    je   Nullzb

	    push CX
	    dec  AL
	    mov  CL,AL
	    mov  AH,1
	    shl  AH,CL
	    pop  CX
;
  cykl_z:   mov  AL,BH
	    and  AL,80h      ; Mask
	    je   pozaz
	    mov  AL,[BP+A_OFF+A_SHF+0Ah]
	    mov  CS:[POZADI],0
	    jmp  A_2
   pozaz:   mov  AL,[BP+A_OFF+A_SHF+0Ch]
	    push CS:[CHRMOD]
	    pop  CS:[POZADI]
     A_2:   push AX
	    dec  CH           ; 8
	    jne  hopla1_z
	    inc  DI
	    mov  BH,ES:[DI]
	    mov  CH,8
	    jmp  hopla2_z
  hopla1_z:
	    shl  BH,CL        ; # bits in pixel
  hopla2_z:
	    mov  AL,8         ; Set video. reg
	    out  DX,AL        ; on 3ceh
	    inc  DX
	    mov  AL,BL
	    out  DX,AL
	    dec  DX
	    mov  AL,[SI]      ; Read latch registers
	    pop  AX
	    cmp  CS:[POZADI],0
	    jne  Nepi_02
	    mov  [SI],AL      ; write pixel

 Nepi_02:   shr  BL,1
	    jne  nicevo_z
	    inc  SI
	    mov  BL,80h

  nicevo_z: shr  AH,1
	    jne  cykl_z         ; Cycle per 8 pixels

	    push CX             ; refresh AH,BL
	    mov  AX,[BP+A_OFF+A_SHF+2]
	    and  AX,7
	    mov  CL,AL
	    mov  BL,80h
	    shr  BL,CL
	    pop  CX
	    inc  DI             ; next byte from input

  Nullzb:   mov  AH,80h
;----------------------------------------------------
	    push DX
	    mov  DX,[BP+A_OFF+A_SHF+6]  ; bytes per line
	    shr  DX,1
	    shr  DX,1
	    shr  DX,1
	    mov  [BP-0ah],DX
	    pop  DX
	    mov  SI,[BP-0ch]
	    mov  CS:WORK1,SI
	    add  SI,[BP-12h]  ; New adr videoram (next line)
	    cmp  SI,CS:WORK1  ; for 1024
	    ja   No_seg_1
	    inc  CS:VSEGM
	    SET_SVGA_SEG
	    cmp  CS:SVGAV,80h  ; for VESA
	    jne  No_seg_1
	    SET_SVGA_REA

  No_seg_1: mov  [BP-0ch],SI
	    dec  WORD PTR [BP-0eh]     ; number of lines
	    je   End16
	    jmp  cykl_r                ; Cycle per lines
;
; Set original write mode EGA/VGA (mode 0)
    End16:  mov  AL,8         ; Bits mask
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
;
	    add  SP,10        ; Dealoc local
	    jmp  Endx;

;------------------ 256 color modes ----------------
;
     R256:  push AX           ; [BP-0Ah] xg_svga
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
IFDEF  A_SMALL
	    mov  DX,DS
ELSE
	    mov  DX,[BP+A_OFF+2]
ENDIF
	    mov  ES,DX

	    mov  AX,0A000h
	    mov  DS,AX
;
  Radek:
	    mov  word ptr [BP-14h],0    ; New line
	    mov  CX,[BP+A_OFF+A_SHF+6]  ; # of pixel per line
	    mov  [BP-0ch],CX
	    mov  DX,[BP+A_OFF+A_SHF+4]  ; line
	    mov  AX,[BP-0Eh]  ; 320/640/800/1024 save
	    mul  DX
	    add  AX,[BP+A_OFF+A_SHF+2]  ; add col
	    adc  DX,0                   ; AX offset, DX segment
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
	    cmp  AH,00h
	    je   Standart      ; 320 x 200
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

 Standart:  jmp  Draw
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
	    SEG_M1W2
	    jmp Draw
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
	    mov  AL,[BP+A_OFF+A_SHF+0Ah]  ; color of text
	    jmp  A_3

   poz_c:   cmp  CS:CHRMOD,0
	    jne  No_poza
	    mov  AL,[BP+A_OFF+A_SHF+0Ch]  ; color of background

     A_3:   mov  DS:[SI],AL
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
	    inc  word ptr [BP+A_OFF+A_SHF+4]
	    dec  word ptr [BP+A_OFF+A_SHF+8]
	    jz   Hop2
	    jmp  Radek

      Hop2: add  SP,16     ; local var
	    jmp  Endx
;
;------------------ Binary ------------------------------
  BinCol:   mov AX,0
	    jmp  Uplny	    ;; JdS 2004/10/07

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
_wrt_chr  endp

ROWLL  DW  0
SVGAA  DW  0
VSEGM  DW  0
WORK1  DW  0
POZADI DW  0   ; akt. status, write pixel ? y/n
CHRMOD DW  0   ; alfa text 0-n, 1-y
WND_BL DB  0   ; VESA window
SVGAV  DB  0
VESA64 DB  0   ; 0/1 granul < 64

IFDEF  A_SMALL
_TEXT ENDS
ELSE
WRT_CHR_TEXT ENDS
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
