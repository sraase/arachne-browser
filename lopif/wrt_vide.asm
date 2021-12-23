;-----------------------------------------------------
; Procedure for drawing rastr. image on EGA\VGA\SVGA
;-----------------------------------------------------

include svga_mac.inc
;
SET_SVGA_SEG MACRO
            push AX
            push BX
            push CX
            push DX
            mov  DX,CS:VSEGM  ; Segment
            mov  AX,CS:SVGAA  ; xg_svga
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
            mov  DX,CS:VSEGM  ; Segment
            mov  AX,CS:SVGAA  ; xg_svga
            call _x_svgar
            pop  DX
            pop  CX
            pop  BX
            pop  AX
            ENDM

IFDEF  A_SMALL
A_OFF    EQU    4
A_SHF    EQU    0
ELSE
A_OFF    EQU    6
A_SHF    EQU    2
ENDIF

IFDEF  A_SMALL
         EXTRN  _x_svgaw:NEAR
         EXTRN  _x_svgar:NEAR
         EXTRN  _wrt_bincga:NEAR
ELSE
         EXTRN  _x_svgaw:FAR
         EXTRN  _x_svgar:FAR
         EXTRN  _wrt_bincga:FAR
ENDIF

IFDEF  A_SMALL
_TEXT SEGMENT BYTE PUBLIC 'CODE'
      ASSUME cs:_TEXT,DS:_BSS
_wrt_video1  proc near
ELSE
WRT_VIDE_TEXT SEGMENT BYTE PUBLIC 'CODE'
              ASSUME cs:WRT_VIDE_TEXT,DS:_BSS
_wrt_video1  proc far
ENDIF
            public _wrt_video1
;
            push BP          ; Uschova registru (?AX..DX)
            mov  BP,SP
            push DI
            push SI
            push DS
            push ES
;
; Call from C: wrt_video1(char *buf, int xpix, int ypix, int ncol, int nrow, int npix)
;
; npix - number of pixels in byte
;

            mov  CS:VESA64, 0       ; Default value ;; JdS 2004/09/27

            mov  AX,SEG _xg_svga
            mov  ES,AX
            mov  AX,ES:[_xg_svga]   ; grf. mode
            cmp  AH,80h             ; VESA mode? 0695
            jne  No_vesa

            push ES
            mov  BX,SEG _xg_vesa
            mov  ES,BX
            lea  BX,ES:_xg_vesa     ; ES:BX address VESA struct
;; JdS 2004/09/27 {
;;          mov  BX,[ES:BX+4]       ; Granularity
;;          pop  ES
;;          cmp  BX,64
;;          je   No_vesa
;;          mov  CS:VESA64, 1
;;          jmp  Cont_1;
;;No_vesa:  mov  CS:VESA64, 0

            mov  CX,ES:[BX+4]       ; Granularity
            mov  BX,ES:[BX+20]      ; Bytes/line
            pop  ES
            cmp  CX,64
            je   Chk_BpL
            mov  CS:VESA64, 1
Chk_BpL:    cmp  BX,0           ; If the VESA bytes/line is unknown,
            jne  X_2            ;   make best guess from graphics mode
No_vesa:
;; JdS 2004/09/27 }

 Cont_1:    push AX
            and AL,1Fh
            cmp AL,01h      ; delka radku 320,640,800,1024
            je  Go320;
            cmp AL,02h
            je  Go640
            cmp AL,04h
            je  Go800
            cmp AL,08h
            je  Go1024
            pop AX
            jmp Enderr

    Go320:  mov BX,320
            jmp X_1
    Go640:  mov BX,640
            jmp X_1
    Go800:  mov BX,800
            jmp X_1
   Go1024:  mov BX,1024

;; JdS 2004/10/03 {
;;    X_1:  pop  AX
;;          and  AL,0E0h
;;          cmp  AL,40h
;;          je   Hup16
;;          cmp  AL,80h
; Check colour option, non-Vesa mode
      X_1:  pop  AX
	    and  AL,0E0h  ; select 3 bits
	    cmp  AL,40h
	    jne   X_3     ; if 16 colour mode,
	    shr  BX,1     ;   revise previous bytes/line guess
	    shr  BX,1     ;   (divide by eight)
	    shr  BX,1
	    jmp  Hup16
; Check colour option, Vesa mode
      X_2:  and  AL,0E0h  ; select 3 bits
	    cmp  AL,40h   ; 16 colour
	    je   Hup16
; Check colour option, common code
      X_3:  cmp  AL,80h   ; 256 colour
;; JdS 2004/10/03 }
            je   Hxx256
            cmp  AL,20h
            je   Hxx2
            jmp  EndErr

   Hxx256:  jmp  R256   ;-------->>> 256 color modes
   Hxx2:    jmp  Binary;

;-------------------------- 16 color modes -------------------
;; JdS 2004/10/03 {
;;   Hup16: shr  BX,1
;;          shr  BX,1
;;          shr  BX,1
;;          mov  CS:ROWLL,BX
     Hup16: mov  CS:ROWLL,BX
;; JdS 2004/10/03 }
            mov  CS:SVGAA,AX
            mov  CS:SVGAV,AH
            mov  CX,BX  ; number columns in bytes

            mov  DI,[BP+A_OFF]    ; ES:DI - input buffer (image)
IFDEF  A_SMALL
            mov  DX,DS
ELSE
            mov  DX,[BP+A_OFF+2]
ENDIF
            mov  ES,DX

            mov  AX,[BP+A_OFF+A_SHF+6]   ; NCOL/8
            shr  AX,1
            shr  AX,1
            shr  AX,1
            push AX           ; on stack at [BP-ah]
;
            push CX

; Begin image in videoram and next constants
            mov  AX,[BP+A_OFF+A_SHF+4]  ; ypix
            mov  DX,CX
            mul  DX           ; begin byte line in BX
            mov  BX,AX
            mov  AX,[BP+A_OFF+A_SHF+2]  ; xpix
            mov  CH,8
            div  CH           ; xpix/8 in AL, remainder in AH
            mov  CL,AH
            sub  AH,AH
            add  BX,AX
            adc  DX,0
            mov  CS:VSEGM,DX
            mov  SI,BX        ; in SI begin byte videoram
;
            pop  DX           ; (80|100|128-ncol/8)
            mov  BX,[BP-0ah]
            sub  DX,BX
            push DX           ; on stack at [BP-ch]
;
            mov  DX,[BP+A_OFF+A_SHF+8]  ; number of lines
            push DX           ; on stack at [BP-eh]
;
            mov  CH,[BP+A_OFF+A_SHF+0Ah]  ;  pixels in byte

            cmp  CH,1
            je   jeden
            cmp  CH,2
            je   dva
            cmp  CH,4
            je   ctyri
            cmp  CH,8
            je   osm

  jeden:    mov  AL,0FFh      ; Mask
            mov  AH,8         ; Number bits on pixel
            jmp ok
  dva:      mov  AL,0Fh
            mov  AH,4
            jmp ok
  ctyri:    mov  AL,3h
            mov  AH,2
            jmp ok
  osm:      mov  AL,1h
            mov  AH,1
   ok:      push AX          ; on stack [BP-10h]
;
; Set write mode EGA/VGA  2
            mov  DX,0A000h
            mov  DS,DX
;
            mov  DX,3ceh
            mov  AL,5
            out  DX,AL
            inc  DX           ; in DX adr 3cfh
            mov  AL,2
            out  DX,AL
            dec  DX           ; back to 3ceh
;
            mov  BL,80h       ; shift bit
            shr  BL,CL
            mov  AH,80h       ; cycle  8
            mov  CS:BLBEG,BL
            mov  CS:PRETOK,0  ; for sub SI,...

            mov  CX,[BP-10h]  ; shift packed pixels
            xchg CL,CH
            mov  CH,[BP+A_OFF+A_SHF+0Ah]
;
; Write inp. buffer in ES:DI to videoram on DS:SI
            mov  BH,ES:[DI]

            cmp  CS:ROWLL,128  ; pro 1024
            jl   cykl_r
            SET_SVGA_SEG
            cmp  CS:SVGAV,80h  ; Pro VESA
            jne  cykl_r
            SET_SVGA_REA

  cykl_r:   cmp  WORD PTR [BP-0Ah],0
            je   Zbytek_x
;
  cykl:     mov  AL,BH
            and  AL,[BP-10h]  ; Mask
            push AX
            dec  CH           ; 1,2,4,8
            jne  hopla1
            inc  DI
            mov  BH,ES:[DI]
            mov  CH,[BP+A_OFF+A_SHF+0Ah]
            jmp  hopla2
  hopla1:
            shr  BH,CL        ; shift about number pixels in byte
  hopla2:
            mov  AL,8         ; Set video. reg
            out  DX,AL        ; on 3ceh
            inc  DX
            mov  AL,BL
            out  DX,AL
            dec  DX
            mov  AL,[SI]      ; Read latch registers
            pop  AX
            mov  [SI],AL      ; Write pixel

            shr  BL,1
            jne  nicevo
            inc  SI           ; ?? 1024
            mov  BL,80h

  nicevo:   shr  AH,1
            jne  cykl         ; Cycle 8 pixels
;
            mov  AH,80h
            dec  WORD PTR [BP-0ah]
            jne  cykl         ; Lines cycle
;
;------------ remainder after byte ----------------
  Zbytek_x:
            mov  AX,[BP+A_OFF+A_SHF+6]
            and  AX,7
            je   Nullzb

            push CX
            dec  AL
            mov  CL,AL
            mov  AH,1
            shl  AH,CL
            pop  CX
;
  cykl_z:   mov  AL,BH
            and  AL,[BP-10h]  ; Mask
            push AX
            dec  CH           ; 1,2,4,8
            jne  hopla1_z
            inc  DI
            mov  BH,ES:[DI]
            mov  CH,[BP+A_OFF+A_SHF+0Ah]
            jmp  hopla2_z
  hopla1_z:
            shr  BH,CL        ; number bits in pixel
  hopla2_z:
            mov  AL,8         ; Set video. reg
            out  DX,AL        ; on 3ceh
            inc  DX
            mov  AL,BL
            out  DX,AL
            dec  DX
            mov  AL,[SI]      ; Read latch registers
            pop  AX
            mov  [SI],AL      ; Write pixel

            shr  BL,1
            jne  nicevo_z
            inc  SI
            mov  CS:PRETOK,1
            mov  BL,80h

  nicevo_z: shr  AH,1
            jne  cykl_z         ; Cycle 8 pixels

            push CX             ; refresh AH,BL
            mov  AX,[BP+A_OFF+A_SHF+2]
            and  AX,7
            mov  CL,AL
            mov  BL,80h
            shr  BL,CL
            mov  AH,80h
            pop  CX

  Nullzb:
            mov  BL,80h
            push DX
            mov  DX,[BP+A_OFF+A_SHF+6]  ; bytes on line
            shr  DX,1
            shr  DX,1
            shr  DX,1
            mov  [BP-0ah],DX
            pop  DX
            sub  SI,CS:PRETOK
            mov  CS:PRETOK,0
            mov  CS:WORK1,SI
            add  SI,[BP-0ch]  ; New adr videoram (next line)

            cmp  CS:ROWLL,128
            jl   No_seg_1
            cmp  SI,CS:WORK1  ; for 1024
            ja   No_seg_1
            inc  CS:VSEGM
            SET_SVGA_SEG
            cmp  CS:SVGAV,80h ; for VESA
            jne  No_seg_1
            SET_SVGA_REA

  No_seg_1: dec  WORD PTR [BP-0eh]     ; Lines
            je   End16
            mov  AH,80h
            mov  BL,CS:BLBEG
            jmp  cykl_r                ; Lines cycle
;
; Original write mode EGA/VGA (mode 0)
    End16:  mov  AL,8         ; Bits mask
            out  DX,AL
            inc  DX
            mov  AL,0ffh
            out  DX,AL
            dec  DX
;
            mov  AL,5         ; Mode registr
            out  DX,AL
            inc  DX
            mov  AL,0
            out  DX,AL
;
            add  SP,8        ; Delete local variables
            jmp  Endx;

;------------------ 256 color modes ----------------
;
     R256:  push AX           ; [BP-0Ah] xg_svga
            mov  CX,[BP+A_OFF+A_SHF+6]  ; number pixels on line
            push CX           ; [BP-0Ch]
            push BX           ; [BP-0Eh]  length line
            sub  SP,4
            mov  word ptr[BP-10h],-1 ; Akt seg. (granul) videoram
            mov  word ptr[BP-12h],0  ; Length of segment+1 (65536 = 0)

            mov  DI,[BP+A_OFF]    ; ES:DI - input buffer
IFDEF  A_SMALL
            mov  DX,DS
ELSE
            mov  DX,[BP+A_OFF+2]
ENDIF
            mov  ES,DX

            mov  AX,0A000h    ; DS:SI - adsress in videoram
            mov  DS,AX
;
  Radek:
            mov  CX,[BP+A_OFF+A_SHF+6]  ; pixels per line
            mov  [BP-0ch],CX
            mov  DX,[BP+A_OFF+A_SHF+4]  ; line
            mov  AX,[BP-0Eh]  ; 320/640/800/1024 save
            mul  DX
            add  AX,[BP+A_OFF+A_SHF+2]  ; add col
            adc  DX,0                   ; AX offset, DX segment
;
            mov  SI,AX        ; SI offset
            push DX           ; save segment
;
            cmp  word ptr[BP-12h],0     ; 64K segment
            jne  Set_seg
            cmp  word ptr[BP-10h],DX    ; Set segment
            jne  Set_seg
            jmp  Draw
;----------------  Set segmentu videoram
;
   Set_seg: cmp  word ptr[BP-12h],0
            jne  Hop_seg       ; for < 64kB set in VESA
            mov  [BP-10h],DX
   Hop_seg: mov  AX,[BP-0ah]   ; grf. mode xg_svga
            cmp  AH,00h
            je   Standart               ; 320 x 200
            cmp  AH,01h
            je   Tseng
            cmp  AH,02h
            je   OAK
            cmp  AH,04h
            je   Tamara
            cmp  AH,08h
            je   Trident
            cmp  AH,10h
            je   RealTek
            cmp  AH,20h
            je   Tseng4
            cmp  AH,40h
            je   M1
            cmp  AH,80h
            jne  Standart
            jmp  VESA

 Standart:  jmp  Draw
;
;------------- TSENG --------------------------
   Tseng:
            SEG_TSENG3
            jmp  Draw
;------------- OAK ----------------------------
     OAK:
            SEG_OAK
            jmp  Draw
;------------- TAMARACK -----------------------
  Tamara:
            SEG_TAMARA
            jmp  Draw
;------------- TRIDENT -----------------------
  Trident:
            SEG_TRIDENT
            jmp  Draw
;------------- RealTek -----------------------
RealTek:
            SEG_REALTEK
            jmp  Draw
;------------- Tseng 4000 --------------------
Tseng4:
            SEG_TSENG4
            jmp  Draw
;------------- M1 ----------------------------
M1:
            SEG_M1W2
            jmp Draw
;------------- VESA --------------------------
  VESA:     push ES
            mov  BX,SEG _xg_vesa
            mov  ES,BX
            lea  BX,ES:_xg_vesa   ; ES:BX adresa VESA struct

            mov  CL,ES:[BX+2]     ; Najit okno pro WRITE
            and  CL,05h
            cmp  CL,05h
            je   Okno_A
            mov  CS:WND_BL, 1
            jmp  Granul
Okno_A:     mov  CS:WND_BL, 0
Granul:
            push DI
            push BX
            mov  BX,ES:[BX+4]     ; Granularity
            cmp  BX,64
            je   Vesa_hop
            mov  CL,10            ; * 1024
            shl  BX,CL
            mov  [BP-12h],BX      ; length of segment
            mov  AX,SI
            div  BX               ; DX:AX / BX
            xchg AX,DX            ; DX - granul , AX offset in granul
            mov  [BP-10h],DX
            mov  SI,AX
 Vesa_hop:  mov  BH,0
            mov  BL,CS:WND_BL
            mov  AX,4F05h         ; VESA Bios Window control
            ;;int  10h
            pop  DI               ; DI<-BX
            call DWORD PTR ES:[DI+0Ch]
            pop  DI               ; refresh DI,ES
            pop  ES
;-------------------------------------------------------
; Write image to videoram
;
  Draw:     mov  CX,[BP-0ch] ; number of pixels (all line|remainder)
;
; Draw one line
;
  Cyk3:     mov  AL,ES:[DI]
            mov  DS:[SI],AL
            dec  CX
            jz   End3
            inc  DI
            inc  SI
            cmp  SI, word ptr [BP-12h]  ; New segm. in line
            jne  Cyk3
;                             ; SI = 0 -> next segment
            mov  [BP-0ch],CX  ; save remainder
            mov  SI,0         ; only for seg < 64K

            cmp  CS:VESA64, 0           ; 0695
            je   No_granul
            inc  word ptr[BP-10h]
            mov  DX, [BP-10h]
            push ES
            mov  BX,SEG _xg_vesa  ; !--- 0397
            mov  ES,BX
            lea  BX,ES:_xg_vesa   ; ES:BX adresa VESA struct
            push DI
            push BX               ; !---
            jmp  Vesa_hop

No_granul:  pop  DX
            inc  DX
            push DX
            jmp  Set_seg      ; set segment to DX
;
; Next line
  End3:     pop  DX
            inc  DI
            inc  SI
            inc  word ptr [BP+A_OFF+A_SHF+4]
            dec  word ptr [BP+A_OFF+A_SHF+8]
            jz   E256
            jmp  Radek

     E256:  add  SP,10        ; dealloc local variables
            jmp  Endx
;
;------------------ Binary (call deleted) ------------------------------
; wrt_bincga(char *Img, int x1, int y1, int dx, int dy, int Lo, int Fir)
Binary:
            jmp Enderr;

;;          mov  AX,0
;;          push AX                     ; First
;;          push AX                     ; lo
;;          push word ptr[BP+A_OFF+0Ah] ; dy
;;          push word ptr[BP+A_OFF+08h] ; dx
;;          push word ptr[BP+A_OFF+06h] ; y1
;;          push word ptr[BP+A_OFF+04h] ; x1
;;          push word ptr[BP+A_OFF+02h] ; Img Seg
;;          push word ptr[BP+A_OFF+00h] ; Img Off
;;          call _wrt_bincga
;;          add SP,16

;------------------ Common end ------------------------
     Endx:  mov  AX,1       ; O.K. end
            jmp  Uplny

     Enderr: mov AX,-1      ; Err end

     Uplny: pop  ES         ; refresh registers
            pop  DS
            pop  SI
            pop  DI
            pop  BP
;
            ret
_wrt_video1  endp

ROWLL  DW  0
SVGAA  DW  0
VSEGM  DW  0
WORK1  DW  0
PRETOK DW  0
WND_BL DB  0
SVGAV  DB  0
BLBEG  DB  0
VESA64 DB  0

IFDEF  A_SMALL
_TEXT ENDS
ELSE
WRT_VIDE_TEXT ENDS
ENDIF

DGROUP         GROUP    _BSS
_BSS          SEGMENT  WORD PUBLIC 'BSS'
        EXTRN  _xg_svga:word
        EXTRN  _xg_vesa:word
_BSS          ENDS
        END
