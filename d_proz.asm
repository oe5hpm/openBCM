; /***************************************************************
;
;  BayCom(R)	   Packet-Radio fuer IBM PC
;
;  OpenBayCom-Mailbox
;
;  -------------------------------
;  DOS: Ermittelt den Prozessortyp
;  -------------------------------
;
;
;  Copyright (C)       Florian Radlherr
;		      Taubenbergstr. 32
;		      83627 Warngau
;
;  Alle Rechte vorbehalten / All Rights Reserved
;
; ***************************************************************/
;
;//19980207 DG9AML (ANDreas) created; support 8086-80786
;//19980208 OE3DZW fixed linebreak _DATA was no closed
;//19980215 DG9AML fixed check for 486er

CPUID           MACRO
		db      0Fh,0A2h                ;Macro for CPUID instruction
ENDM

IGROUP group _text                ;Zusammenfassung der Programm-Segmente
DGROUP group _bss,  _data         ;Zusammenfassung der Daten-Segmente
			 assume CS:IGROUP, DS:DGROUP, ES:DGROUP, SS:DGROUP

_BSS   segment word public 'BSS'  ;dieses Segment nimmt alle nicht ini-
_BSS   ends                       ;tialisierten statischen Variablen auf

_DATA  segment word public 'FAR_DATA' ;alle initialisierten globalen und
				;statischen Variablen werden in diesem
				;Segment untergebracht
_DATA  ends

;== Konstanten =========================================================

p_80886   equ   8
p_80786   equ   7
p_80686   equ   6
p_80586   equ   5
p_80486   equ   4                 ;Codes fÅr die verschiedenen Pro-
p_80386   equ   3                 ;zessortypen
p_80286   equ   2
p_8086    equ   1
p_unknown equ   0

;== Programm ===========================================================

_TEXT  segment byte public 'CODE' ;das Programmsegment

public   @getproz$qv

; GETPROZ: ermittelt den Prozessortyp, syntax int getproz( void );

@getproz$qv     proc    far

		pushf                   ;den Inhalt des Flag-Registers sichern
		push di

;
; Check for an 8086 or 8088 by attempting to clear bits 12-15 of the
; FLAGS register. On an 8086 or 8088, these bits are always set.
;
		mov  dl,p_8086
		cli                             ;Interrupts off
		pushf                           ;Save FLAGS
		pushf                           ;Push FLAGS on stack
		pop     ax                      ;Pop FLAGS into AX
		and     ax,0FFFh                ;Clear bits 12-15
		or      ax,5000h                ;Set bits 12 and 14
		push    ax                      ;Push AX on stack
		popf                            ;Pop AX into FLAGS
		pushf                           ;Push FLAGS on stack
		pop     ax                      ;Pop FLAGS into AX
		popf                            ;Restore FLAGS
		sti                             ;Interrupts on
		and     ax,0F000h               ;Clear bits 0-11
		cmp     ax,0F000h               ;Are bits 12-15 set?
		jne     check_286               ;No, then it's a 286 or higher
		jmp     done                    ;Yes, then it's an 8086/88
;
; Check for a 286 by seeing if bits 12-15 are clear after the previous
; operation. On a 286 running in real mode, these bits are always clear.
;
check_286:	mov  dl,p_80286                 ;Assume 286
		or      ax,ax                   ;Are bits 12-15 clear?
		jne     check_386               ;No, then it's a 386 or higher
		jmp     done                    ;Yes, then it's a 286
;
; Check for a 386 by attempting to toggle the EFLAGS register's Alignment
; Check (AC) bit. This bit cannot be changed on a 386.
;
		.386

check_386:	mov      dl,p_80386             ;Assume 386
		cli                             ;Interrupts off
		pushfd                          ;Save EFLAGS
		pushfd                          ;Push EFLAGS on stack
		pop     eax                     ;Pop EFLAGS into EAX
		mov     ebx,eax                 ;Store EAX in EBX
		xor     eax,40000h              ;Toggle bit 18
		push    eax                     ;Push EAX on stack
		popfd                           ;Pop EAX into EFLAGS
		pushfd                          ;Push EFLAGS on stack
		pop     eax                     ;Pop EFLAGS into EAX
		popfd                           ;Restore EFLAGS
		sti                             ;Interrupts on
		and     eax,40000h              ;Clear all but bit 18
		and     ebx,40000h              ;in EAX and EBX
		cmp     eax,ebx                 ;Compare EAX and EBX
		je      done                    ;If equal, then it's a 386
;
; Check for a 486 by attempting to toggle the EFLAGS register's ID bit.
; This bit cannot be changed on *most* 486s.
;
		mov  dl,p_80486                 ;Assume 486
		cli                             ;Interrupts off
		pushfd                          ;Save EFLAGS
		pushfd                          ;Push EFLAGS on stack
		pop     eax                     ;Pop EFLAGS into EAX
		mov     ebx,eax                 ;Store EAX in EBX
		xor     eax,200000h             ;Toggle bit 21
		push    eax                     ;Push EAX on stack
		popfd                           ;Pop EAX into EFLAGS
		pushfd                          ;Push EFLAGS on stack
		pop     eax                     ;Pop EFLAGS into EAX
		popfd                           ;Restore EFLAGS
		sti                             ;Interrupts on
		and     eax,200000h             ;Clear all but bit 21 in
		and     ebx,200000h             ;EAX and EBX
		cmp     eax,ebx                 ;Compare EAX and EBX
		je      done                    ;If equal, then it's a 486
;
; We succeeded in toggling the processor's ID bit, so it's probably a
; Pentium or higher. But it *could* be a 486, so be careful. To identify
; the processor, use the family ID code returned by the CPUID instruction.
;
		mov     eax,1                   ;Get CPU ID information
		CPUID                           ;via the CPUID instruction
		and     eax,0F00h               ;Clear all but bits 8-11
		shr     eax,8                   ;Shift right 8 places
						;(EAX = Family ID)

		mov  dl,p_80486
		cmp	eax,4			;Is the family ID4?
		je	done			;Yes, it's a newer 486

		mov  dl,p_80586
		cmp     eax,5                   ;Is the family ID 5?
		je      done                    ;Yes, then it's a Pentium

		mov  dl,p_80686
		cmp     eax,6                   ;Is the family ID 6?
		je      done                    ;Yes

		mov  dl,p_80786
		cmp     eax,7                   ;Is the family ID 7?
		je      done                    ;Yes

		mov  dl,p_80886
		cmp     eax,8                   ;Is the family ID 8?
		je      done                    ;Yes

		mov dl,p_unknown

done     label near              ;die Tests sind abgeschlossen

	  pop  di                 ;DI wieder vom Stack holen
	  popf                    ;Flag-Register wieder vom Stack holen
	  xor  dh,dh              ;HI-Byte des Prozessorcodes auf 0
	  mov  ax,dx              ;Prozessorcode ist Return-Wert der Fkt

	  ret                     ;zurÅck zum Aufrufer

@getproz$qv	endp                    ;Ende der Prozedur


;== Ende ===============================================================

_text     ends                    ;Ende des Programm-Segments
		end                     ;Ende des Assembler-Source
