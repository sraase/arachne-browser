;--------------------------------------------------------------------
; Procedure for read raster image from videoram for mode 6, 640x200x1
;--------------------------------------------------------------------
;
; Call from C: rea_bincga(char *Img, int x1, int y1, int dx, int dy);
;
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
_rea_bincga   proc near
ELSE
REA_BCGA_TEXT SEGMENT BYTE PUBLIC 'CODE'
	      ASSUME cs:REA_BCGA_TEXT,DS:_BSS
_rea_bincga   proc far
ENDIF
	    public _rea_bincga
;
	    push BP          ; Save registers (?AX..DX)
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES

	    mov  DI,[BP+A_OFF]    ; ES:DI - output buffer
IFDEF  A_SMALL
	    mov  DX,ES
ELSE
	    mov  DX,[BP+A_OFF+2]
ENDIF
	    mov  ES,DX
;
	    mov  BX,50h
	    mov  AX,[BP+A_OFF+A_SHF+4]  ; Y1
	    mov  CS:YAKT, AX
	    test AX,1
	    je   Even1         ; even
	    mov  SI,2000h
	    jmp  Next1
     Even1: mov  SI,0h
     Next1: shr  AX,1
	    mul  BX           ; *80
	    mov  BX,[BP+A_OFF+A_SHF+2]  ; X1
	    mov  CL,3
	    shr  BX,CL        ; divide by 8
	    add  BX,AX
	    add  SI,BX
	    mov  BX,0B800h
	    mov  DS,BX        ; DS:SI - begin adress in videoram

	    mov  DX,[BP+A_OFF+A_SHF+6] ; Width
	    dec  DX
	    shr  DX,1
	    shr  DX,1
	    shr  DX,1
	    ;;inc  DX
	    mov  CS:LINEBYTE,DX        ; width-1! in bytes in output buffer.

	    mov  AX,[BP+A_OFF+A_SHF+2]    ; X1
	    mov  CH,8
	    div  CH           ; X1/8 in AL remainder in AH
	    mov  CL,AH        ; in CL remainder

	    mov  AX,[BP+A_OFF+A_SHF+2]   ; (X1+Width)
	    add  AX,[BP+A_OFF+A_SHF+6]
	    dec  AX                      ; -1 = X2

	    ; BL - mask pro last byte line (X2 / 8)
	    and  AX,0007h             ; AL remeinder after div 8 from X2
	    inc  AL
	    cmp  CL,AL
	    jge  Hop_zb
	    sub  AL,CL                ; zb X1 < zb X2
	    mov  AH,8
	    sub  AH,AL
	    jmp  Mask_1
    Hop_zb: mov  AH,CL                ; zb X1 >= zb X2
	    sub  AH,AL
    Mask_1: push CX
	    mov  BL,1                 ; (1<<AH)-1 and neg
	    mov  CL,AH
	    shl  BL,CL
	    sub  BL,1
	    not  BL
	    pop  CX

;----------------- Read line from videoram -----------------
New_line:
	    mov  DX,CS:LINEBYTE  ; number of bytes of line

	    lodsb                ; byte DS:SI++ -> AL
	    mov  AH,AL

     Pix8:  lodsb

	    mov  CH,AL        ; original byte from buffer
	    shl  AX,CL
	    and  DX,DX
	    je   End8
	    mov  ES:[DI],AH
	    inc  DI
	    mov  AH,CH
	    dec  DX
	    jmp  Pix8

   End8:    and  AH,BL      ; BL maska
	    mov  ES:[DI],AH
	    inc  DI

; -------- Next lines
	    mov  DX,[BP+A_OFF+A_SHF+4] ; Y1
	    add  DX,[BP+A_OFF+A_SHF+8] ; Height
	    mov  AX, CS:YAKT
	    inc  AX
	    cmp  AX, DX
	    jge  EndAll

	    push BX
	    push CX

	    mov  CS:YAKT,AX
	    mov  BX,50h
	    test AX,1
	    je   Even3         ; even
	    mov  SI,2000h
	    jmp  Next3
     Even3: mov  SI,0h
     Next3: shr  AX,1
	    mul  BX           ; *80
	    mov  BX,[BP+A_OFF+A_SHF+2]  ; X1
	    mov  CL,3
	    shr  BX,CL        ; divide by 8
	    add  BX,AX
	    add  SI,BX

	    pop  CX
	    pop  BX
	    jmp  New_Line        ; Write next line to videoram

;-------------- END --------------------------
  EndAll:   pop  ES
	    pop  DS
	    pop  SI
	    pop  DI
	    pop  BP
;
	    ret
_rea_bincga endp

LINEBYTE    DW   0
YAKT        DW   0

IFDEF  A_SMALL
_TEXT ENDS
ELSE
REA_BCGA_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_svga:word
_BSS          ENDS
	END
