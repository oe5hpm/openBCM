;
;  BayCom(R)	   Packet-Radio fuer IBM PC
;
;  OpenBayCom-Mailbox
;
;  Assembler-Teil
;
;
;
;
;
;    Routinen fuer Maus-Ansteuerung
;
	   .model large

	   public _mausinit,_mausein,_mausaus,_mausistda,_maussetz
	   public _mauszeile,_mausspalte,_mausknopf,_mausrechts

	   .code

_mausda    dw 0

_mausinit  proc far
	   push bp
	   mov  bp,sp
	   mov  ax,0h          ; Reset Maustreiber
	   int  33h
	   cmp  ax,0ffffh
	   jne  initfail

	   mov  ax,0ah
	   mov  bx,0
	   mov  cx,0ffffh
	   mov  dx,07700h
	   int  33h

	   mov  ax,4            ; ins Eck positionieren
	   mov  cx,(79*8)
	   mov  dx,0
	   int  33h

	   mov  ax,1            ; Mauscursor anzeigen
	   mov  cs:_mausda,ax
	   int  33h

	   mov  ax,8
	   mov  dx,[bp+6]       ; 1. Parameter am Stack holen (proc far)
	   mov  cl,3
	   shl  dx,cl
	   mov  cx,1
	   int  33h

	   mov  ax,7
	   mov  dx,[bp+8]
	   mov  cl,3
	   shl  dx,cl
	   mov  cx,1
	   int  33h

	   mov  ax,1
	   pop  bp
	   retf

initfail:  mov  ax,0
	   mov  cs:_mausda,ax
	   pop  bp
	   retf
_mausinit  endp

_maussetz  proc far
	   cmp  cs:_mausda,0
	   jz   nixsetz
	   push bp
	   mov  bp,sp
	   mov  cx,[bp+6]       ; 1. Parameter am Stack holen (proc far)
	   mov  cl,3
	   shl  cx,cl
	   mov  dx,[bp+8]       ; 2. Parameter am Stack holen (proc far)
	   mov  cl,3
	   shl  dx,cl
	   mov  ax,4
	   int  33h
	   pop  bp
nixsetz:   retf
_maussetz  endp

_mausknopf proc far
	   mov  ax,cs:_mausda
	   or   ax,ax
	   jz   keinknopf
	   mov  ax,3
	   int  33h
	   mov  ax,bx
	   and  ax,1
keinknopf: retf
_mausknopf endp

_mausrechts proc far
	   mov  ax,cs:_mausda
	   or   ax,ax
	   jz   keinknopf
	   mov  ax,3
	   int  33h
	   mov  ax,bx
	   and  ax,2
keinrechts: retf
_mausrechts endp

_mauszeile proc far
	   mov  ax,cs:_mausda
	   or   ax,ax
	   jz   nixzeile
	   mov  ax,3
	   int  33h
	   mov  ax,dx
	   mov  cl,3
	   shr  ax,cl
nixzeile:  retf
_mauszeile endp

_mausspalte proc far
	   mov  ax,cs:_mausda
	   or   ax,ax
	   jz   nixspalte
	   mov  ax,3
	   int  33h
	   mov  ax,cx
	   mov  cl,3
	   shr  ax,cl
nixspalte: retf
_mausspalte endp

_mausein   proc far
	   cmp  cs:_mausda,2
	   jnz  nixein
	   mov  ax,1
	   mov  cs:_mausda,ax
	   int  33h
nixein:    retf
_mausein   endp

_mausaus   proc far
	   cmp  cs:_mausda,1
	   jnz  nixaus
	   mov  ax,2
	   mov  cs:_mausda,ax
	   int  33h
nixaus:    retf
_mausaus   endp

_mausistda proc far
	   mov  ax,cs:_mausda
	   retf
_mausistda endp

	   end
