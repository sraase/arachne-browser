;---------------------------------------------------------------
; Procedure GETIMAGE for 16 colors
;---------------------------------------------------------------
;
; Call from C: x_gimg16(int x1, int y1, int x2, int y2,char *buf, int plan)
; plan = 1 for bit plane 1; plan = 4 for all (4) bits planes
;
IFDEF  A_SMALL
_AOFF    EQU    4
ELSE
_AOFF    EQU    6
ENDIF

SETPLAN  MACRO           ; Set bit plane 3,2,1,0 (CH)
	 push DX
	 mov  AL,4
	 mov  DX,03CEh
	 out  DX,AL
	 inc  DX
	 mov  AL,CH
	 dec  AL
	 out  DX,AL
	 pop  DX
	 ENDM

SET_SVGA_SEG MACRO            ; Set segmentu vidoram (1024)
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
	 EXTRN  _x_svgar:NEAR
ELSE
	 EXTRN  _x_svgar:FAR
ENDIF

IFDEF  A_SMALL
_TEXT SEGMENT BYTE PUBLIC 'CODE'
      ASSUME cs:_TEXT,DS:_BSS
_x_gimg16   proc near
ELSE
X_GIMG16_TEXT SEGMENT BYTE PUBLIC 'CODE'
	      ASSUME cs:X_GIMG16_TEXT,DS:_BSS
_x_gimg16   proc far
ENDIF
	    public _x_gimg16
;
	    push BP
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES

	    sub  SP,12              ; local var (BP-0A..14h)

	    mov  AX,SEG _xg_svga    ; length of line
	    mov  ES,AX
	    mov  AX,ES:[_xg_svga]   ; graf.mod
	    ;;and  AL,80h
	    and  AL,0E0h
	    mov  [BP-12h],AX        ; x_svga

	    mov  AX,ES:[_xg_svga]   ; grf. mode
	    and AL,1Fh
	    cmp AL,02h              ;  640,800,1024 pixels
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

	    mov  DI,[BP+_AOFF+8]    ; Out buffer ES:DI
IFDEF  A_SMALL
	    mov  DX,DS
	    mov  ES,DX
ELSE
	    mov  DX,[BP+_AOFF+0Ah]
	    mov  ES,DX
ENDIF

IFDEF  A_SMALL
	    mov  DX,[BP+_AOFF+0Ah]  ; # bits planes
ELSE
	    mov  DX,[BP+_AOFF+0Ch]  ; # bits planes
ENDIF
	    mov  CS:BIT_PLAN,DL

	    mov  DX,[BP+_AOFF+4]    ;  X2-X1+1
	    sub  DX,[BP+_AOFF]
	    inc  DX
	    mov  ES:[DI],DX
	    dec  DX
	    shr  DX,1
	    shr  DX,1
	    shr  DX,1
	    mov  [BP-0Eh],DX        ; bytes of line

	    mov  DX,[BP+_AOFF+6]    ; Y2-Y1+1
	    sub  DX,[BP+_AOFF+2]
	    inc  DX
	    mov  [BP-0Ch],DX
	    mov  ES:[DI+2],DX
	    add  DI,4               ; Begin out. buffer

	    mov  AX,[BP+_AOFF+2]    ; Y1 [First byte videoram DS:SI]
	    mov  DX,BX
	    mul  DX
	    mov  BX,AX              ; BX ofset, DX segment
	    mov  AX,[BP+_AOFF]      ; X1
	    mov  CH,8
	    div  CH                 ; X1/8 in AL, remainder in AH
	    mov  CL,AH              ; CL remeinder
	    sub  AH,AH
	    add  BX,AX
	    adc  DX,0               ; for 1024
	    mov  [BP-10h],DX
	    mov  SI,BX
	    mov  DX,0A000h
	    mov  DS,DX        ; DS:SI Begin address byte videoram

	    ; BL - mask pro last byte line
	    mov  AX,[BP+_AOFF+4]      ; X2 / 8
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

;----------------- Reading videoram -----------------
	    mov  DX,[BP-0Eh]

   Radek:   mov  CH,CS:BIT_PLAN    ; Cycle: lines  (1 nebo 4)

   Plane:   push CX                ; Cycle: planes (CH=4)
	    push DX
	    push SI
	    SETPLAN           ; set plane 3..0 (CH-1)
	    cmp word ptr [BP-0Ah],128
	    jl  No_set_1
	    SET_SVGA_SEG

 No_set_1:  lodsb             ; 1 lines of 1 bits plane to DS:SI->AL

	    cmp  SI,0
	    jne  No_set_2
	    inc  word ptr [BP-10h]
	    SET_SVGA_SEG
	    dec  word ptr [BP-10h]

 No_set_2:  mov  AH,AL
   Pix8:    lodsb

	    cmp  SI,0
	    jne  No_set_3
	    inc  word ptr [BP-10h]
	    SET_SVGA_SEG
	    dec  word ptr [BP-10h]

 No_set_3:  mov  CH,AL
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

	    pop  SI
	    pop  DX
	    pop  CX
	    dec  CH
	    je   End_pl1
	    jmp  Plane

  End_pl1:  mov  [BP-14h],SI
	    add  SI,word ptr [BP-0Ah]   ; length of line
	    cmp  SI,[BP-14h]
	    ja   No_new_seg
	    inc  word ptr [BP-10h]
No_new_seg: dec  word ptr [BP-0Ch]
	    je   End_uu1
	    jmp  Radek

;-------------- Ending --------------------------
 End_uu1:   mov  DX,3CEh
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
_x_gimg16   endp

BIT_PLAN  DB  0

IFDEF  A_SMALL
_TEXT ENDS
ELSE
X_GIMG16_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_svga:word
	EXTRN  _xg_vesa:word
_BSS          ENDS
	END
