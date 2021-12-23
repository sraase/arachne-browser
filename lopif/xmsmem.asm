;
;                  Extended memory control functions
;
VERSION_2       EQU     1
SUBVERSION      EQU     0

_AOFF           EQU     6

;THIS MACRO FETCHES THE DATA SEGMENT
DASEG         MACRO
                POP     DI
                POP     SI
                POP     SS
                PUSH    AX
		MOV     AX,_DATA
		MOV     DS,AX
		POP     AX
                ENDM

XMEM_TEXT       SEGMENT BYTE PUBLIC 'CODE'
		ASSUME CS:XMEM_TEXT,DS:_DATA

;THIS FUNCTION INITIALIZES THE DRIVER
;               CALLED AS
;               get_xmem1();
;
;         returns the version number or -1 if no driver
;
		PUBLIC  _get_xmem1
_get_xmem1      PROC    FAR
		PUSH    BP
		MOV     BP,SP
		PUSH    SS
		PUSH    SI
		PUSH    DI

		MOV     AX,4300H        ;CALL FOR THE DRIVER
		INT     2FH
		CMP     AL,80H          ;WAS IT THERE ?
		JE      GETX1

		MOV     AX,0FFFFH
		JMP     GETX2

GETX1:          MOV     AX,4310H        ;GET VECTOR, PLEASE...
		INT     2FH
		MOV     WORD PTR CS:[_CONTOFF],BX
		MOV     WORD PTR CS:[_CONTSEG],ES

		XOR     AX,AX           ;GET THE VERSION NUMBER
		CALL    CS:[CONTROL]

GETX2:          DASEG
		POP    BP
		RET
_get_xmem1      ENDP

;THIS FUNCITON MOVES EXTENDED MEMORY
;            CALLED AS
;            move_xmem1(p);
;            p = pointer to move structure
;                /* returns true if successfull */
;
		PUBLIC  _move_xmem1
_move_xmem1     PROC    FAR
		PUSH    BP
		MOV     BP,SP
		PUSH    SS
		PUSH    SI
		PUSH    DI

		MOV     SI,[BP + _AOFF + 0]         ;OFFSET OF STRUCTURE
		MOV     DS,[BP + _AOFF + 2]         ;SEGMENT OF STRUCTURE

		CLI                                 ;; disable int
		MOV     AH,11
		CALL    CS:[CONTROL]
		STI                                 ;; enable int

		DASEG
		POP    BP
		RET
_move_xmem1     ENDP

;THIS FUNCTION DEALLOCATES EXTENDED MEMORY
;           CALLED AS
;           dealloc_xmem1(h);
;           int h;  /* handle to deallocate */
;                   /* returns true if successfull */
;
		PUBLIC _dealloc_xmem1
_dealloc_xmem1  PROC   FAR
		PUSH   BP
		MOV    BP,SP
		PUSH    SS
		PUSH    SI
		PUSH    DI

		MOV    DX,[BP + _AOFF + 0]      ;HANDLE
		CLI    ;; disable int
		MOV    AH,10
		CALL   CS:[CONTROL]             ;BYE, MEMORY...
		STI    ;; enable int

		DASEG
		POP    BP
		RET
_dealloc_xmem1  ENDP

;THIS FUNCTION ALLOCATES EXTENDED MEMORY
;           CALLED AS
;           alloc_xmem1(n);
;           int n;  /* number of kilobytes to allocate */
;                   /* returns handle or -1 if error */
;
		PUBLIC _alloc_xmem1
_alloc_xmem1    PROC   FAR
		PUSH   BP
		MOV    BP,SP
		PUSH    SS
		PUSH    SI
		PUSH    DI

		MOV    DX,[BP + _AOFF + 0]      ;NUMBER OF KILOBYTES
		CLI    ;; disable int
		MOV    AH,9
		CALL   CS:[CONTROL]             ;SOME MEMORY, PLEASE...
		STI    ;; enable int

		OR     AX,AX
		JZ     ALLOC1

		MOV    AX,DX
		JMP    ALLOC2

ALLOC1:         MOV    AX,0FFFFH

ALLOC2:         DASEG
		POP    BP
		RET
_alloc_xmem1    ENDP

;THIS FUNCTION REALLOCATES EXTENDED MEMORY
;           CALLED AS
;           realloc_xmem(handle, newkb);
;           int handle;  /* handle for XMS     */
;           int newkb    /* New size XMS in kB */
;           returns      1-if O.K. 0-if error
;
		PUBLIC _realloc_xmem
_realloc_xmem   PROC   FAR
		PUSH   BP
		MOV    BP,SP
		PUSH   SS
		PUSH   SI
		PUSH   DI

		MOV    DX,[BP + _AOFF + 0]      ; HANDLE
		MOV    BX,[BP + _AOFF + 2]      ; KB
		MOV    AH,0Fh
		CALL   CS:[CONTROL]

		DASEG
		POP    BP
		RET
_realloc_xmem   ENDP


;THIS FUNCTION GET HANDLE INFORMATION OF EXTENDED MEMORY
;           CALLED AS
;           hinf_xmem(handle, *RetKB, *RetFreeh);
;           int handle;   /* handle for XMS     */
;           int *RetKB    /* Size of block XMS for this handle */
;           int *RetFreeh /* Number of free XMS handles */
;           returns      0-if O.K. -1-if error
;
		PUBLIC _hinf_xmem
_hinf_xmem      PROC   FAR
		PUSH   BP
		MOV    BP,SP
		PUSH   SS
		PUSH   SI
		PUSH   DI

		MOV    DX,[BP + _AOFF + 0]      ; HANDLE
		MOV    AH,0Eh
		CLI
		CALL   CS:[CONTROL]
		STI
		TEST   BL,80h
		JNZ    Error

		MOV  CX,BX
		MOV  CH,0
		LES  BX,[BP+_AOFF+2]
		MOV  ES:WORD PTR [BX], DX   ; KB
		LES  BX,[BP+_AOFF+6]
		MOV  ES:WORD PTR [BX], CX   ; free handle
		MOV  AX,0
		JMP  Konec

	Error:  MOV  AX,-1
	Konec:	DASEG
		POP    BP
		RET
_hinf_xmem      ENDP


;THIS FUNCTION CONVERTS A POINTER TO AN INTEL LONG
;              CALLED AS
;              long ptr2long(p);
;              char *p
;
		PUBLIC  _ptr2long
_ptr2long       PROC    FAR
		PUSH    BP
		MOV     BP,SP
		PUSH    SS
		PUSH    SI
		PUSH    DI

		MOV     AX,[BP + _AOFF + 0]     ;OFFSET OF POINTER
		MOV     DX,[BP + _AOFF + 2]     ;SEGMENT OF POINTER

		DASEG
		POP    BP
		RET
_ptr2long       ENDP


;THIS FUNCTION IS A DUMMY RETURN FOR UNSET PROCEDURES
_DUMMY         PROC     FAR
	       MOV      AX,0FFFFH
	       RET
_DUMMY         ENDP

CONTROL        LABEL    DWORD
_CONTOFF       DW       _DUMMY
_CONTSEG       DW       XMEM_TEXT

XMEM_TEXT      ENDS

DGROUP         GROUP    _DATA,_BSS
_DATA          SEGMENT  WORD PUBLIC 'DATA'

_DATA          ENDS

_BSS           SEGMENT WORD PUBLIC 'BSS'
_BSS           ENDS
	       END


