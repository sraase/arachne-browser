; Macros for settings segments of videoram SVGAs (.ASM)

SEG_TSENG3  MACRO
	    mov  AL,DL
	    mov  AH,AL
	    and  AX,0707h      ; 3 bits
	    shl  AH,1          ; AL = 9*AL | 0x40
	    shl  AH,1
	    shl  AH,1
	    or   AL,AH
	    or   AL,040h
	    mov  DX,3CDh
	    out  DX,AL
	    ENDM

SEG_OAK     MACRO
	    mov  AH,DL
	    mov  CL,4
	    shl  AH,CL
	    or   AH,DL
	    mov  AL,11h
	    mov  DX,3DEh
	    out  DX,AX
	    ENDM

SEG_TAMARA  MACRO
	    mov  AH,DL
	    mov  CL,4
	    shl  AH,CL
	    mov  AL,09h
	    mov  DX,3CEh
	    out  DX,AX
	    ENDM

SEG_TRIDENT MACRO
	    push DX
	    cli
	    mov  DX,3C4h
	    mov  AL,0Bh
	    out  DX,AL
	    inc  DL
	    in   AL,DX
	    dec  DL
	    pop  AX
	    mov  AH,AL
	    xor  AH,2
	    mov  AL,0Eh
	    out  DX,AX
	    sti
	    ENDM

SEG_REALTEK MACRO
	    mov  AL,DL
	    mov  DX,3D7h
	    out  DX,AL
	    ENDM

SEG_TSENG4  MACRO
	    mov  AL,DL
	    mov  AH,AL
	    mov  CL,4
	    shl  AL,CL
	    or   AL,AH
	    mov  DX,3CDh
	    out  DX,AL
	    ENDM

SEG_M1R     MACRO
	    push DX
	    mov  AX,87A7h
	    mov  DX,3C4h
	    out  DX,AX
	    pop  DX
	    shl  DL,1
	    shl  DL,1
	    shl  DL,1
	    shl  DL,1
	    mov  AH,DL
	    mov  AL,0C5h
	    mov  DX,3C4h
	    out  DX,AX
	    mov  AX,00A7H
	    out  DX,AX
	    ENDM

SEG_M1W     MACRO
	    push DX
	    mov  AX,87A7h
	    mov  DX,3C4h
	    out  DX,AX
	    pop  DX
	    mov  AH,DL
	    shl  DL,1
	    shl  DL,1
	    shl  DL,1
	    shl  DL,1
	    or   AH,DL
	    mov  AL,0C5h
	    mov  DX,3C4h
	    out  DX,AX
;	    mov  AX,0F02h
;	    out  DX,AX
	    ENDM

SEG_M1W2    MACRO
	    push DX
	    mov  AX,87A7h
	    mov  DX,3C4h
	    out  DX,AX
	    pop  DX
	    mov  AH,DL
	    mov  AL,0C5h
	    mov  DX,3C4h
	    out  DX,AX
	    ENDM