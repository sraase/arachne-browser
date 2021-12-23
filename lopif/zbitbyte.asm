;---------------------------------------------------------------
; Unpack 8 bits from a byte into 8 bytes
;---------------------------------------------------------------
;
; void _far _cdecl z_bitbyte(byte far *ibuf, byte far *obuf, word ilen);
;
; *ibuf  buffer with packed pixels
; *obuf  buffer for unpacked pixels
; ilen   number of packed bytes in input buffer
;
; Modifies: es = FP_SEG(out); dx = 0; cx = (ilen ? 0 : cx);
;

        PUBLIC _z_bitbyte

_AOFF   EQU  6  ; Offset of parameters in stack frame for _far function

ZBITBYTE_TEXT   SEGMENT BYTE PUBLIC 'CODE'
                ASSUME CS:ZBITBYTE_TEXT,DS:DGROUP

_z_bitbyte      PROC FAR

        push    bp
        mov     bp,sp                   ; set up stack frame
        mov     dx,[BP+_AOFF+8]         ; nbytes
        test    dx,dx                   ; zero? (would mean 65536 bytes!)
        jz      done                    ; nothing to do
        push    di
        push    si                      ; save TurboC register variables
        push    ds                      ;   and global DATA segment
        cld                             ; 'up' direction for lodsb/stosb
        lds     si,[bp+_AOFF+0]         ; ds:si = ibuf
        les     di,[bp+_AOFF+4]         ; es:di = obuf
byteloop:
        lodsb                           ; al = next byte from ibuf
        mov     cx,8                    ; bit count
        test    al,al                   ; test for common case (speed up)
        jz      store                   ; all bits 0, so store 8 zeroes
        cmp     al,0FFh                 ; another common case: all bits 1
        mov     ah,al                   ; make a copy of the byte
        mov     al,1
        je      store                   ; if all 1, store 8 ones
bitloop:
        rol     ax,1                    ; move next bit into b0 in al
        and     al,1                    ; mask it out to get 0 or 1
        stosb                           ; store into the buffer
        test    ah,ah                   ; any nonzero bits remaining in ah?
        loopne  bitloop                 ; cx--, continue if ah && cx
        mov     al,ah                   ; cx 0s remains to store, al = 0 then
store:
        rep     stosb                   ; store al cx times
        dec     dx                      ; nbytes--
        jnz     byteloop                ; nonzero, continue
        pop     ds
        pop     si
        pop     di                      ; restore saved registers
done:
        pop     bp                      ; restore caller's stack frame
        ret

_z_bitbyte      ENDP

ZBITBYTE_TEXT ENDS

DGROUP          GROUP   _DATA,_BSS

_BSS            SEGMENT WORD PUBLIC 'BSS'
_BSS            ENDS

_DATA           SEGMENT WORD PUBLIC 'DATA'
_DATA           ENDS

        END
