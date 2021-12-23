;---------------------------------------------------------------
; Procedure PUTIMAGE for 16 colors
;---------------------------------------------------------------
;
; Call from C: x_pimg16(int x1, int y1, char *buf, int oper, int plan)
; plane = 1 or 4 bits planes
;
IFDEF  A_SMALL
A_OFF    EQU    4
ELSE
A_OFF    EQU    6
ENDIF

SET_WRP  MACRO           ; Set bit plane 8 4 2 1 (dle CH)
	 push DX
	 push CX
	 mov  AL,2
	 mov  DX,03C4h
	 out  DX,AL
	 inc  DX
	 mov  CL,CH
	 dec  CL
	 mov  AL,1
	 shl  AL,CL
	 out  DX,AL
	 pop  CX
	 pop  DX
	 ENDM

SET_SVGA_SEG MACRO            ; Set segment vidoram (1024)
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
	 EXTRN  _x_svgaw:NEAR
	 EXTRN  _x_svgar:NEAR
ELSE
	 EXTRN  _x_svgaw:FAR
	 EXTRN  _x_svgar:FAR
ENDIF

IFDEF  A_SMALL
_TEXT SEGMENT BYTE PUBLIC 'CODE'
      ASSUME cs:_TEXT,DS:_BSS
_x_pimg16   proc near
ELSE
X_PIMG16_TEXT SEGMENT BYTE PUBLIC 'CODE'
	      ASSUME cs:X_PIMG16_TEXT,DS:_BSS
_x_pimg16   proc far
ENDIF
	    public _x_pimg16
;
	    push BP
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES

	    sub  SP,12               ; local var (BP-0A..14h)

IFDEF  A_SMALL
	    mov  BX,[BP+A_OFF+6]    ; COPY,OR,AND,XOR
ELSE
	    mov  BX,[BP+A_OFF+8]    ; COPY,OR,AND,XOR
ENDIF
	    mov  DX,3CEh
	    mov  AL,3
	    out  DX,AL
	    inc  DX
	    mov  AL,BL              ; bits 3,4
	    shl  AL,1
	    shl  AL,1
	    shl  AL,1
	    out  DX,AL

	    mov  AX,SEG _xg_svga
	    mov  ES,AX
	    mov  AX,ES:[_xg_svga]   ; grf.mode
	    and  AL,0E0h
	    mov  [BP-12h],AX        ; x_svga
	    mov  CS:SVGAV,AH        ; for VESA

	    mov  AX,ES:[_xg_svga]
	    and  AL,1Fh
	    cmp  AL,02h             ; 640,800,1024 pixels
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
	    mov  [BP-0Ah],BX        ; lengts of line in bytes
	    mov  CS:ROWLL,BX        ; !! BP

	    mov  SI,[BP+A_OFF+4]    ; Input buffer DS:SI
IFDEF  A_SMALL
ELSE
	    mov  DX,[BP+A_OFF+6]
	    mov  DS,DX
ENDIF
	    mov  DX,DS:[SI]         ; # columns
	    dec  DX
	    shr  DX,1
	    shr  DX,1
	    shr  DX,1
	    inc  DX
	    mov  [BP-0Eh],DX        ; # bytes of line in input buf.

	    mov  AX,[BP+A_OFF]       ; X1 / 8
	    shr  AX,1
	    shr  AX,1
	    shr  AX,1
	    mov  DX,DS:[SI]           ; remainder
	    add  DX,[BP+A_OFF]      ; X2  /8
	    dec  DX
	    shr  DX,1
	    shr  DX,1
	    shr  DX,1
	    inc  DX
	    sub  DX,AX
	    cmp  DX,[BP-0Eh]
	    jg   Skok1
	    mov  CS:ZBDELKA,0       ; 0/1 1 - add one byte
	    jmp  Skok2
   Skok1:   mov  CS:ZBDELKA,1

   Skok2:   mov  DX,DS:[SI+2]       ; # rows
	    mov  [BP-0Ch],DX

	    mov  AX,[BP+A_OFF+2]    ; Y1 [first byte videoram ES:DI]
	    mov  DX,BX
	    mul  DX
	    mov  BX,AX              ; BX ofset, DX segment
	    mov  AX,[BP+A_OFF]      ; X1
	    mov  CH,8
	    div  CH           ; X1/8 in AL, remainder in AH
	    mov  CL,AH        ; CL remainder
	    sub  AH,AH
	    add  BX,AX
	    adc  DX,0         ; carry for 1024
	    mov  [BP-10h],DX
	    mov  DI,BX
	    mov  DX,0A000h
	    mov  ES,DX        ; ES:DI first byte videoram

	    ; BX - mask for first and last byte line
	    push CX
	    mov  DX,[BP+A_OFF]     ; X1
	    and  DX,7
	    mov  CL,8
	    sub  CL,DL
	    mov  BH,1
	    shl  BH,CL
	    dec  BH
	    mov  DX,[BP+A_OFF]     ; X2 = X1+delka-1
	    add  DX,DS:[SI]
	    dec  DX
	    and  DX,7
	    inc  DL
	    mov  CL,8
	    sub  CL,DL
	    mov  BL,1
	    shl  BL,CL
	    dec  BL
	    not  BL
	    pop  CX

	    add  SI,4               ; Begin input image DS:SI

IFDEF A_SMALL
	    mov  DX,[BP+A_OFF+6]    ; write mode
ELSE
	    mov  DX,[BP+A_OFF+8]
ENDIF
	    mov  word ptr CS:WRTMODE,DX

IFDEF A_SMALL
	    mov  DX,[BP+A_OFF+8]    ; # planes
ELSE
	    mov  DX,[BP+A_OFF+0Ah]
ENDIF
	    mov  CS:BIT_PLAN,DL

	    mov  CS:ULOZBP,BP

;----------------- Write to videoram -----------------
	    mov  DX,3CEh      ; Bit mask reg
	    mov  AL,8
	    out  DX,AL

   Radek:   mov  CH,CS:BIT_PLAN    ; Cycle over lines
	    push BP                ; save original BP
	    mov  BP,[BP-0Eh]       ; # bytes per line

   Plane:   push CX                ; Cycle over bit planes (CH=4)
	    push BX
	    push BP
	    push DI
	    SET_WRP           ; Map mask reg 8 4 2 1

	    cmp  CS:ROWLL,128
	    jl   No_set_1
	    push BP
	    mov  BP,CS:ULOZBP
	    SET_SVGA_SEG
	    cmp  CS:SVGAV,80h  ; for VESA
	    jne  No_set_b1
	    SET_SVGA_REA
 No_set_b1: pop  BP

 No_set_1:  mov  DX,03CFh

	    push BX
	    lodsb             ; DS:SI++ -> AL
	    dec  BP
	    xor  AH,AH
	    mov  CH,AL
	    mov  BL,BH        ; in BL is mask

   Line:    shr  AX,CL        ; CL shift in byte
	    and  BP,BP
	    je   End_plane
	    mov  AH,AL
	    mov  AL,BL
	    out  DX,AL
	    mov  AL,AH
	    cmp  CS:WRTMODE,4 ; for neg
	    jne  Hop_1
	    not  AL
    Hop_1:  mov  AH,ES:[DI]     ; Latch registr
	    mov  ES:[DI],AL     ; write to video
	    mov  BL,0FFh      ; mask
	    inc  DI
	    cmp  DI,0

	    jne  No_set_2
	    push BP
	    mov  BP,CS:ULOZBP
	    inc  word ptr [BP-10h]
	    SET_SVGA_SEG
	    cmp  CS:SVGAV,80h  ; for VESA
	    jne  No_set_b2
	    SET_SVGA_REA
 No_set_b2: dec  word ptr [BP-10h]
	    pop  BP

 No_set_2:  mov  AH,CH
	    lodsb
	    mov  CH,AL
	    dec  BP
	    jmp  Line

End_plane:  mov  DL,CS:ZBDELKA
	    cmp  DL,0
	    pop  DX           ; mask from BX
	    je   End_byte

	    push DX           ; One all byte to video
	    mov  DX,3CFh
	    mov  AH,AL
	    mov  AL,BL
	    out  DX,AL
	    mov  AL,AH
	    cmp  CS:WRTMODE,4 ; for neg
	    jne  Hop_2
	    not  AL
    Hop_2:  mov  AH,ES:[DI]
	    mov  ES:[DI],AL
	    pop  DX
	    inc  DI
	    cmp  DI,0
	    jne  No_set_3
	    push BP
	    mov  BP,CS:ULOZBP
	    inc  word ptr [BP-10h]
	    SET_SVGA_SEG
	    cmp  CS:SVGAV,80h  ; for VESA
	    jne  No_set_b3
	    SET_SVGA_REA
No_set_b3:  dec  word ptr [BP-10h]
	    pop  BP

No_set_3:   mov  AH,CH
	    xor  AL,AL
	    shr  AX,CL
	    mov  BL,0FFh

 End_byte:  and  BL,DL       ; last byte
	    mov  DX,3CFh
	    mov  AH,AL
	    mov  AL,BL
	    out  DX,AL
	    mov  AL,AH
	    cmp  CS:WRTMODE,4 ; for neg
	    jne  Hop_3
	    not  AL
    Hop_3:  mov  AH,ES:[DI]
	    mov  ES:[DI],AL

	    pop  DI
	    pop  BP
	    pop  BX
	    pop  CX
	    dec  CH
	    je   Rad_1
	    jmp  Plane        ; next bit plane of line

    Rad_1:  pop  BP
	    mov  [BP-14h],DI
	    add  DI,word ptr [BP-0Ah]
	    cmp  DI,[BP-14h]
	    ja   No_new_seg
	    inc  word ptr [BP-10h]
No_new_seg: dec  word ptr [BP-0Ch]
	    je   Ukonci
	    jmp  Radek        ; Next line

;-------------- Ending --------------------------
  Ukonci:   mov  DX,3CEh
	    mov  AX,5
	    out  DX,AX
	    mov  AX,3
	    out  DX,AX
	    mov  AX,0FF08h
	    out  DX,AX
	    mov  AX,0F02h
	    mov  DX,3C4h
	    out  DX,AX

   Enderr:  add  SP,12
	    pop  ES
	    pop  DS
	    pop  SI
	    pop  DI
	    pop  BP
;
	    ret
_x_pimg16   endp

WRTMODE     DW   0
ZBDELKA     DB   0
ULOZBP      DW   0
ROWLL       DW   0
BIT_PLAN    DB   0
SVGAV       DB   0

IFDEF  A_SMALL
_TEXT ENDS
ELSE
X_PIMG16_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_svga:word
	EXTRN  _xg_vesa:word
_BSS          ENDS
	END
