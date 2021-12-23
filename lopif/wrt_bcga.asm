;-------------------------------------------------------------------
; Procedure for write raster image to videoram for mode 6, 640x200x1
;-------------------------------------------------------------------
;
; Call from C: wrt_bincga(char *Img, int x1, int y1, int dx, int dy,
;                         int Lo, int First)
;
; Lo - log.oper : 0 - COPY
;      bit  0: 0x01  xor
;      bit  1: 0x02  and
;      bit  2: 0x04  or
;      bit  3: 0x08  not
; First - 0-copy all bytes from buffer, 1-copy first byte from buf (draw bar)
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
_wrt_bincga   proc near
ELSE
WRT_BCGA_TEXT SEGMENT BYTE PUBLIC 'CODE'
	      ASSUME cs:WRT_BCGA_TEXT,DS:_BSS
_wrt_bincga   proc far
ENDIF
	    public _wrt_bincga
;
	    push BP          ; Save registers (?AX..DX)
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES

	    mov  SI,[BP+A_OFF]    ; DS:SI - input buffer
IFDEF  A_SMALL
	    mov  DX,DS
ELSE
	    mov  DX,[BP+A_OFF+2]
ENDIF
	    mov  DS,DX
;
            mov  BX,50h
	    mov  AX,[BP+A_OFF+A_SHF+4]  ; Y1
	    mov  CS:YAKT, AX
	    test AX,1
	    je   Even1         ; even
	    mov  DI,2000h
	    jmp  Next1
     Even1: mov  DI,0h
     Next1: shr  AX,1
	    mul  BX           ; *80
	    mov  BX,[BP+A_OFF+A_SHF+2]  ; X1
	    mov  CL,3
	    shr  BX,CL        ; divide by 8
	    add  BX,AX
	    add  DI,BX
	    mov  BX,0B800h
	    mov  ES,BX        ; ES:DI - begin adress in videoram

	    mov  DX,[BP+A_OFF+A_SHF+6] ; Width
	    dec  DX
	    shr  DX,1
	    shr  DX,1
	    shr  DX,1
	    inc  DX
	    mov  CS:LINEBYTE,DX        ; width in bytes in input buffer.
	    mov  CS:LTDX8,0

	    mov  AX,[BP+A_OFF+A_SHF+2]       ; X1 / 8
	    shr  AX,1
	    shr  AX,1
	    shr  AX,1

	    mov  DX,[BP+A_OFF+A_SHF+2]   ; (X1+Width)/8
	    add  DX,[BP+A_OFF+A_SHF+6]
	    dec  DX
	    shr  DX,1
	    shr  DX,1
	    shr  DX,1
	    cmp  DX,AX
	    jne  Skipx;
	    mov  CS:LTDX8,1
     Skipx: inc  DX
	    sub  DX,AX
	    cmp  DX,CS:LINEBYTE
	    jg   Skok1
	    mov  CS:ZBDELKA,0     ; 0/1 1 - pridat na konci jeden cely byte
	    jmp  Skok2
   Skok1:   mov  CS:ZBDELKA,1
   Skok2:
	    mov  AX,[BP+A_OFF+A_SHF+2]    ; X1
	    mov  CH,8
	    div  CH           ; X1/8 in AL remainder in AH
	    mov  CL,AH        ; in CL remainder

	    ; BX - mask for first and last byte of line
	    push CX
	    mov  DX,[BP+A_OFF+A_SHF+2]     ; for X1
	    and  DX,7
	    mov  CL,8
	    sub  CL,DL
	    mov  BH,1
	    shl  BH,CL
	    dec  BH
	    not  BH    ; 11111000
	    mov  DX,[BP+A_OFF+A_SHF+2]     ; X2 = X1+Width-1
	    add  DX,[BP+A_OFF+A_SHF+6]
	    dec  DX
	    and  DX,7
	    inc  DL
	    mov  CL,8
	    sub  CL,DL
	    mov  BL,1
	    shl  BL,CL
	    dec  BL    ; 00000111
	    pop  CX

	    mov  AX,[BP+A_OFF+A_SHF+0Ah]
	    mov  CS:WRTMODE,AX
	    mov  AX,[BP+A_OFF+A_SHF+0Ch]
	    mov  CS:FIRST,AX

;----------------- Write line to videoram -----------------
New_line:
	    push BP
	    mov  BP,CS:LINEBYTE  ; number of bytes of line
	    push BX              ; masks

	    cmp  CS:FIRST,0
	    jne  First1;
	    lodsb                ; byte DS:SI++ -> AL
	    jmp  Normal1
  First1:   mov  AL,DS:[SI]

 Normal1:   test CS:WRTMODE,08h
	    jz   NoNeg1
	    not  AL
  NoNeg1:   dec  BP
	    xor  AH,AH
	    mov  CH,AL        ; original byte from buffer
	    mov  BL,BH        ; in BL is bit mask

   Line:    shr  AX,CL        ; in CL num. of shifts
	    and  BP,BP
	    je   End_plane

	    mov  AH,ES:[DI]   ; Byte from video
	    test CS:WRTMODE,07h ; xor|or|and
	    jnz  IsLog1
	    and  AH,BL        ; copy (NOT copy) "clear bits"
   IsLog1:  not  BL
	    and  AL,BL        ; shift byte from buffer
	    not  BL
	    test CS:WRTMODE,01h ; xor
	    jnz  IsXor2
	    test CS:WRTMODE,02h ; and
	    jnz  IsAnd2
	    or   AL,AH          ; copy, or
	    jmp  Write1
   IsXor2:  xor  AL,AH
	    jmp  Write1
   IsAnd2:  test CS:WRTMODE,04  ; and + or ?
	    jnz  IsAndOr2
	    or   AL,BL
	    and  AL,AH
	    jmp  Write1
  IsAndOr2: mov  DH,AL
	    not  DH
	    not  BL
	    and  DH,BL
	    not  BL
	    or   AL,BL
	    and  AL,AH
	    or   AL,DH

   Write1:  mov  ES:[DI],AL     ; write to video
	    mov  BL,0h        ; mask
	    inc  DI

	    mov  AH,CH
	    cmp  CS:FIRST,0
	    jne  First2
	    lodsb
	    jmp  Normal2
    First2: mov  AL,DS:[SI]

   Normal2: test CS:WRTMODE,08h     ; normal/neg ? 1-neg
	    jz   NoNeg2
	    not  AL
   NoNeg2:  mov  CH,AL
	    dec  BP
	    jmp  Line

End_plane:  mov  DL,CS:ZBDELKA
	    cmp  DL,0
	    je   End_byte

	    mov  AH,ES:[DI]
	    test CS:WRTMODE,07h ; xor,and,or
	    jnz  IsLog2
	    and  AH,BL        ; clear bits
  IsLog2:   not  BL
	    and  AL,BL        ; shift byte from buffer
	    not  BL
	    test CS:WRTMODE,01h ; xor
	    jnz  IsXor4
	    test CS:WRTMODE,02h ; and
	    jnz  IsAnd4
	    or   AL,AH        ; copy, not copy, or
	    jmp  Write2
  IsXor4:   xor  AL,AH
	    jmp  Write2
  IsAnd4:   test CS:WRTMODE,04  ; and + or ?
	    jnz  IsAndOr4
	    or   AL,BL
	    and  AL,AH
	    jmp  Write2
  IsAndOr4: mov  DH,AL
	    not  DH
	    not  BL
	    and  DH,BL
	    not  BL
	    or   AL,BL
	    and  AL,AH
	    or   AL,DH

  Write2:   mov  ES:[DI],AL
	    inc  DI

	    mov  AH,CH
	    xor  AL,AL
	    shr  AX,CL

 End_byte:  pop  BX          ; last byte
	    cmp  CS:LTDX8,0
	    je   Hop1
	    or   BL,BH
     Hop1:  mov  AH,ES:[DI]
	    test CS:WRTMODE,07h
	    jnz  IsLog3
	    and  AH,BL
    IsLog3: not  BL
	    and  AL,BL
	    not  BL
	    test CS:WRTMODE,01h
	    jnz  IsXor6
	    test CS:WRTMODE,02h
	    jnz  IsAnd6
	    or   AL,AH
	    jmp  Write3
    IsXor6: xor  AL,AH
	    jmp  Write3
    IsAnd6: test CS:WRTMODE,04  ; and + or ?
	    jnz  IsAndOr6
	    or   AL,BL
	    and  AL,AH
	    jmp  Write3
  IsAndOr6: mov  DH,AL
	    not  DH
	    not  BL
	    and  DH,BL
	    not  BL
	    or   AL,BL
	    and  AL,AH
	    or   AL,DH

    Write3: mov  ES:[DI],AL

	    pop  BP

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
	    mov  DI,2000h
	    jmp  Next3
     Even3: mov  DI,0h
     Next3: shr  AX,1
	    mul  BX           ; *80
	    mov  BX,[BP+A_OFF+A_SHF+2]  ; X1
	    mov  CL,3
	    shr  BX,CL        ; divide by 8
	    add  BX,AX
	    add  DI,BX

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
_wrt_bincga endp

WRTMODE     DW   0
ZBDELKA     DB   0
LTDX8       DB   0
LINEBYTE    DW   0
YAKT        DW   0
FIRST       DW   0

IFDEF  A_SMALL
_TEXT ENDS
ELSE
WRT_BCGA_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_svga:word
_BSS          ENDS
	END
