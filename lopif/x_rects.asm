;---------------------------------------------------------
; Procedure for drawing rectangle (no fill), COPY,XOR
;---------------------------------------------------------
;
IFDEF  A_SMALL
A_OFF    EQU    4
ELSE
A_OFF    EQU    6
ENDIF

PUT_PIX MACRO
	push AX          ; color
	push DX          ; y
	push CX          ; x
	call _x_putpix
	pop  CX
	pop  DX
	pop  AX
ENDM

GET_PIX MACRO
	push DX          ; y
	push CX          ; x
	call _x_getpix
	pop  CX
	pop  DX
ENDM

IFDEF  A_SMALL
     EXTRN  _x_getpix:NEAR
     EXTRN  _x_putpix:NEAR
ELSE
     EXTRN  _x_getpix:FAR
     EXTRN  _x_putpix:FAR
ENDIF

IFDEF  A_SMALL
_TEXT SEGMENT  BYTE PUBLIC 'CODE'
      ASSUME CS:_TEXT,DS:_BSS
_x_rect_s proc near
ELSE
X_RECTS_TEXT SEGMENT BYTE PUBLIC 'CODE'
	     ASSUME cs:X_RECTS_TEXT,DS:_BSS
_x_rect_s proc far
ENDIF
	    public _x_rect_s
;
	    push BP
	    mov  BP,SP
	    push DI
	    push SI
	    push DS
	    push ES
;
; Call from C: x_rect_s(int col,xz,yz,dx,dy,xr)
;
	    mov AX,SEG _xg_256    ; 2/16/256 colors
	    mov ES,AX
	    mov AX,ES:[_xg_256]

	    cmp  AL,40h        ; 16
	    je   m16

	    cmp  AL,80h        ; 256
	    je   m256
	    cmp  AL,20h        ; 2
	    je   m2
	    jmp  End_exx       ; ?unknown?

    m256:   mov  SI,256        ; 256 colors
	    push SI
	    jmp  Dale

     m16:   mov  SI,16         ;  16 color
	    push SI
	    mov  AX,[BP+A_OFF+0Ah]
	    cmp  AX,0
	    je   Hop2
;
	    mov  DX,03CEh      ; rotate register (XOR)
	    mov  AL,3
	    out  DX,AL
	    inc  DX
	    mov  AL,18h        ; xor - bits 3,4 "11"
	    out  DX,AL
	    jmp  Hop2

     m2:    mov  SI,16         ; xor ????
	    push SI
	    jmp  Hop2
;
     Dale:  mov  AX,[BP+A_OFF+0Ah]  ; 256
	    cmp  AX,0          ; without xor
	    jne  Hop2
	    mov  SI,16

     Hop2:  mov  AX,[BP+A_OFF]     ; color
	    mov  CX,[BP+A_OFF+2]   ; column
	    mov  DX,[BP+A_OFF+4]   ; row
	    mov  DI,[BP+A_OFF+6]   ; # of column

     Cyk1:  cmp  SI,16         ; top line rect
	    je   Put1
	    GET_PIX            ; get pixel, in AX color
	    xor  AX,[BP+A_OFF]
     Put1:  PUT_PIX            ; put pixel
	    inc  CX
	    dec  DI
	    jnz  Cyk1

	    mov  DI,[BP+A_OFF+8]
	    cmp  DI,2
	    jle  End_spec      ; only one line
	    sub  DI,2

     Cyk2:  mov  CX,[BP+A_OFF+2] ; left,right
	    inc  DX
	    cmp  SI,16
	    je   Put2
	    GET_PIX
	    xor  AX,[BP+A_OFF]
     Put2:  PUT_PIX
	    cmp  word ptr[BP+A_OFF+6],1   ; for one column
	    je   Skip1
	    add  CX,[BP+A_OFF+6]
	    dec  CX
	    cmp  SI,16
	    je   Put22
	    GET_PIX
	    xor  AX,[BP+A_OFF]
     Put22: PUT_PIX
     Skip1: dec  DI
	    jnz  Cyk2

	    mov  DI,[BP+A_OFF+6]   ; # columns
	    mov  CX,[BP+A_OFF+2]   ; col
	    mov  DX,[BP+A_OFF+4]   ; row
	    add  DX,[BP+A_OFF+8]
	    dec  DX

     Cyk3:  cmp  SI,16         ; bottom line
	    je   Put3
	    GET_PIX
	    xor  AX,[BP+A_OFF]
     Put3:  PUT_PIX
	    inc  CX
	    dec  DI
	    jnz  Cyk3

; Ending
;
 End_spec:  pop  SI
	    cmp  SI,16
	    jne  End_exx

	    mov  DX,03CEh       ; rotate register na 0
	    mov  AL,3
	    out  DX,AL
	    inc  DX
	    mov  AL,00h        ; bits 3,4 "00"
	    out  DX,AL

   End_exx: pop  ES
	    pop  DS
	    pop  SI
	    pop  DI
	    pop  BP
;
	    ret
_x_rect_s   endp
IFDEF  A_SMALL
_TEXT ENDS
ELSE
X_RECTS_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
	EXTRN  _xg_256:word
_BSS          ENDS
	END
