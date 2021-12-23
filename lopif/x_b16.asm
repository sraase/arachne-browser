;---------------------------------------------------------------
; Procedure for drawing fill rect in 16 colors
; Draw from point XZ,YZ and drawing DX cols, DY rows
; with color COL
; Calling parameters: "C" language
;---------------------------------------------------------------
;
; Call from C: x_b16(int x1, int y1, int x2, int y2, int col, int mask)
;
; mask - LO byte - mask for filling vzorem
;        HI byte   !=0 for write AND,OR,XOR, ==0 COPY
;
SET_SVGA_SEG MACRO
	    push AX
	    push BX
	    push CX
	    push DX
	    mov  DX,[BP-10h]  ; Segment
	    mov  AX,[BP-12h]  ; xg_svga
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
	    mov  DX,[BP-10h]  ; Segment
	    mov  AX,[BP-12h]  ; xg_svga
	    call _x_svgar
	    pop  DX
	    pop  CX
	    pop  BX
	    pop  AX
	    ENDM

IFDEF  A_SMALL
_AOFF    EQU    4
ELSE
_AOFF    EQU    6
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
      ASSUME cs:_TEXT,DS:_BSS
_x_b16      proc near
ELSE
X_B16_TEXT SEGMENT BYTE PUBLIC 'CODE'
	   ASSUME cs:X_B16_TEXT,DS:_BSS
_x_b16      proc far
ENDIF
	    public _x_b16
;
	    push BP
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES

	    sub  SP,10              ; local variable (BP-0A..12h)

	    mov  AX,SEG _xg_svga    ; length of line
	    mov  ES,AX
	    mov  AX,ES:[_xg_svga]   ; grf. mode
	    ;;and  AL,80h
	    and  AL,0E0h            ; ??
	    mov  [BP-12h],AX        ; x_svga
	    mov  CS:SVGAV,AH

	    mov AX,ES:[_xg_svga]    ; grf. mode
	    and AL,1Fh
	    cmp AL,02h              ; length of line 640,800,1024
	    je  Go640
	    cmp AL,04h
	    je  Go800
	    cmp AL,08h
	    je  Go1024
	    jmp Enderr

    Go640:  mov BX,640
	    jmp X_1
    Go800:  mov BX,800
	    jmp X_1
   Go1024:  mov BX,1024

      X_1:  shr  BX,1               ; BX/8
	    shr  BX,1
	    shr  BX,1
	    mov  [BP-0Ah],BX

	    mov  DX,[BP+_AOFF+6]    ; lines: Y2-Y1+1
	    sub  DX,[BP+_AOFF+2]
	    inc  DX
	    mov  [BP-0Ch],DX

	    mov  CL,3
	    mov  AX,[BP+_AOFF+4]    ; # bytes of line X2/8 - X1/8
	    shr  AX,CL
	    mov  DX,[BP+_AOFF]
	    shr  DX,CL
	    sub  AX,DX
	    mov  [BP-0Eh],AX        ; bytes of line

	    mov  AX,[BP+_AOFF+2]  ; Y1 [Begin byte videoram ES:DI]
	    mov  DX,BX
	    mul  DX
	    mov  BX,[BP+_AOFF]  ; X1 / 8
	    shr  BX,1
	    shr  BX,1
	    shr  BX,1
	    add  AX,BX
	    adc  DX,0
	    mov  [BP-10h],DX  ; begin segment videoram
	    mov  DI,AX
	    mov  AX,0A000h
	    mov  ES,AX        ; ES:DI begin adr videoram

;  Mask for first (BH) and last (BL) byte line
	    mov  DX,[BP+_AOFF]     ; X1
	    and  DX,7
	    mov  CL,8
	    sub  CL,DL
	    mov  BH,1
	    shl  BH,CL
	    dec  BH
	    mov  DX,[BP+_AOFF+4]    ; X2
	    and  DX,7
	    inc  DL
	    mov  CL,8
	    sub  CL,DL
	    mov  BL,1
	    shl  BL,CL
	    dec  BL
	    not  BL

;----------------- Set write mode 2 ----------------
	    mov  DX,3CEh    ; Mode registr
	    mov  AL,5
	    out  DX,AL
	    inc  DX
	    mov  AL,2
	    out  DX,AL

	    dec  DX         ; Bit mask registr
	    mov  AL,8
	    out  DX,AL

;----------------- Write to videoram -----------------
	    mov  CX,[BP-0Eh]           ; Bytes of line
	    mov  AX,[BP+_AOFF+10];     ; Mask for vzorek
	    cmp  AH,0
	    je   Copy
	    mov  AH,07Fh                        ;  7F  XOR
	    jmp  x_bx
     Copy:  mov  AH,0FFh                        ;  FF  COPY
     x_bx:  mov  SI,AX                 ; 0/1 7Fmm
	    mov  AX,[BP+_AOFF+8];      ; Color
	    mov  AH,AL
	    ;;cmp  word ptr [BP-0Ah],128
	    ;;jl   Hop_ooo
	    ;;and  SI,7FFFh              ; v 1024  XOR
  Hop_ooo:  cld

   Radek:   push BX
	    push CX
	    push DI

	    cmp  word ptr [BP-0Ah],128
	    jl   No_segm1
	    SET_SVGA_SEG
	    cmp  CS:SVGAV,80h  ; Pro VESA
	    jne  No_segm1
	    SET_SVGA_REA

No_segm1:   mov  DX,3CFh
	    cmp  CX,0
	    jne  Vice_bytu

	    and  BL,BH        ; All line in 1 byte
	    mov  AL,BL
	    and  AX,SI
	    out  DX,AL
	    mov  AL,ES:[DI]   ; EGA Latch reg.
	    mov  ES:[DI],AH
	    jmp  End_row

Vice_bytu:  mov  AL,BH        ; First Byte
	    and  AX,SI
	    out  DX,AL
	    mov  AL,ES:[DI]   ; EGA Latch reg.
	    mov  ES:[DI],AH
	    inc  DI
	    cmp  DI,0         ; New segment in line
	    jne  Hop_no_1
	    inc  word ptr [BP-10h]     ; next segment
	    SET_SVGA_SEG
	    cmp  CS:SVGAV,80h  ; for VESA
	    jne  Hop_no_1
	    SET_SVGA_REA
Hop_no_1:   dec  CX

	    cmp  CX,0
	    je   End_byte
	    mov  AL,0FFh
	    and  AX,SI
	    out  DX,AL
	    test SI,8000h
	    jz   Loox

	    mov  AL,ES:[DI]   ; for COPY
	    mov  AL,AH
	    rep   stosb             ; while CX != 0
;;;      LLL1: mov  AL,ES:[DI]
;;;	    mov  AL,AH
;;;	    mov  ES:[DI],AL
;;;	    inc  DI
;;;	    loop LLL1
	    jmp  End_byte

      Loox: mov  BH,ES:[DI]   ; for XOR
	    mov  ES:[DI],AH
	    inc  DI
	    cmp  DI,0
	    jne  Hop_no_2
	    inc  word ptr [BP-10h]     ; next segment
	    SET_SVGA_SEG
	    cmp  CS:SVGAV,80h  ; for VESA
	    jne  Hop_no_2
	    SET_SVGA_REA
 Hop_no_2:  loop Loox

End_byte:   mov  AL,BL
	    and  AX,SI
	    out  DX,AL
	    mov  AL,ES:[DI]   ; EGA Latch reg.
	    mov  ES:[DI],AH

  End_row:  pop  DI
	    pop  CX
	    pop  BX

	    push DI
	    add  DI,word ptr [BP-0Ah]  ; Add length line of grf. mode
	    pop  DX
	    cmp  DI,DX
	    ja   No_new_seg
	    inc  word ptr [BP-10h]
No_new_seg: dec  word ptr [BP-0Ch]     ; Num. of lines
	    je   End_end
	    jmp  Radek

;-------------- Ending --------------------------
 End_end:   mov  DX,3CEh
	    mov  AX,5
	    out  DX,AX
	    ;;mov  AX,3
	    ;;out  DX,AX
	    mov  AX,0FF08h
	    out  DX,AX
	    mov  AX,0F02h
	    mov  DX,3C4h
	    out  DX,AX

   Enderr:  add  SP,10
	    pop  ES
	    pop  DS
	    pop  SI
	    pop  DI
	    pop  BP
;
	    ret
_x_b16      endp

SVGAV  DB   0

IFDEF  A_SMALL
_TEXT ENDS
ELSE
X_B16_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_svga:word
	EXTRN  _xg_vesa:word
_BSS          ENDS
	END
