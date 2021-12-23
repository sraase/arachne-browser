; Add fce for manipulation with XMS:
; mem_xmem - ret max. free block and all free XMS in Kb

_AOFF     EQU  6

DATASET   MACRO
	  push AX
	  mov  AX,_DATA
	  mov  DS,AX
	  pop  AX
	  ENDM

;	 .MODEL   LARGE

XMSADD_TEXT SEGMENT BYTE PUBLIC 'CODE'
	    ASSUME CS:XMSADD_TEXT,DS:_DATA

; Call from "C" ist = mem_xmem1(&maxblok, &freeall) -> KB XMS memory

	   PUBLIC _mem_xmem1
_mem_xmem1 PROC far
	  push BP
	  mov  BP,SP

	  mov  AX,4300h            ; Exist driver ?
	  int  2Fh
	  cmp  AL,80h
	  je   Nex1

	  mov  AX,0
	  jmp  Nex2

Nex1:     mov  AX,4310h
	  int  2Fh                 ; Adress of driver
	  mov  WORD PTR CS:[_CONTOFF],BX
	  mov  WORD PTR CS:[_CONTSEG],ES

	  mov  AH,08h              ; All mem XMS
	  call CS:[CONTROL]
	  cmp  BL,0
	  je   OKcall
	  mov  AX,0
	  jmp  Nex2

OKcall:	  les  BX,[BP+_AOFF]
	  mov  ES:WORD PTR [BX], AX   ; max.block
	  les  BX,[BP+_AOFF+4]
	  mov  ES:WORD PTR [BX], DX   ; free total
	  mov  AX,1

Nex2:	  DATASET
	  pop BP
	  ret
_mem_xmem1 ENDP

_DUMMY    PROC
	  mov AX,0FFFFh
	  ret
_DUMMY    ENDP

 CONTROL  LABEL DWORD
_CONTOFF  DW _DUMMY
_CONTSEG  DW XMSADD_TEXT

XMSADD_TEXT  ENDS

DGROUP    GROUP _DATA,_BSS
_DATA     SEGMENT WORD PUBLIC 'DATA'
_DATA     ENDS
_BSS      SEGMENT WORD PUBLIC 'BSS'
_BSS      ENDS
	  END
