[bits 32]
global _main
extern  _printf
section .text
PRINTF_NUMBER: db "%i", 10, 0
DATA_LABEL:
db 'm', 'a', 'x', 'S', 't', 'e', 'p', 's', 10, 0
db 'a', 't', 'N', 'u', 'm', 'b', 'e', 'r', 10, 0



_main:
;LABEL LABEL_INDEX_7
LABEL_7:
;OPENSCOPE 
push ebp
mov ebp, esp
	;MOV 0
	mov eax, 0
	;CREATE_VARIABLE int, maxSteps
	sub esp, 4
	;READ maxSteps
	mov dword [ ebp + -4], eax
	;MOV 0
	mov eax, 0
	;CREATE_VARIABLE int, atNumber
	sub esp, 4
	;READ atNumber
	mov dword [ ebp + -8], eax
	;MOV 1
	mov eax, 1
	;CREATE_VARIABLE int, testNumber
	sub esp, 4
	;READ testNumber
	mov dword [ ebp + -12], eax
	;LABEL 0
	LABEL_0:
	;MOV testNumber
	mov eax, dword [ ebp + -12]
	;MOV2 100000
	mov ebx, 100000
	;SETL EBX
	cmp eax, ebx
	setl al
	movzx eax, al
	;CJMP EAX, LABEL_INDEX_6
	cmp eax, 0
	je LABEL_6
	;OPENSCOPE 
	push ebp
	mov ebp, esp
		;MOV testNumber
		mov eax, dword [ ebp + 4]
		;CREATE_VARIABLE int, num
		sub esp, 4
		;READ num
		mov dword [ ebp + -4], eax
		;MOV 0
		mov eax, 0
		;CREATE_VARIABLE int, steps
		sub esp, 4
		;READ steps
		mov dword [ ebp + -8], eax
		;LABEL 1
		LABEL_1:
		;MOV num
		mov eax, dword [ ebp + -4]
		;MOV2 4
		mov ebx, 4
		;SETNE EBX
		cmp eax, ebx
		setne al
		movzx eax, al
		;CJMP EAX, LABEL_INDEX_4
		cmp eax, 0
		je LABEL_4
		;OPENSCOPE 
		push ebp
		mov ebp, esp
			;MOV num
			mov eax, dword [ ebp + 8]
			;MOD EBX
			and eax, 1
			;CREATE_VARIABLE int, isOdd
			sub esp, 4
			;READ isOdd
			mov dword [ ebp + -4], eax
			;MOV isOdd
			mov eax, dword [ ebp + -4]
			;MOV2 1
			mov ebx, 1
			;SETE EBX
			cmp eax, ebx
			sete al
			movzx eax, al
			;CJMP EAX, LABEL_INDEX_2
			cmp eax, 0
			je LABEL_2
			;OPENSCOPE 
			push ebp
			mov ebp, esp
				;MOV num
				mov eax, dword [ ebp + 16]
				;MOV2 3
				mov ebx, 3
				;MUL EBX
				xor edx, edx
				mul ebx
				;ADD EBX
				inc eax
				;READ num
				mov dword [ ebp + 16], eax
				;CLOSESCOPE 
				mov esp, ebp
				pop ebp
			;LABEL 2
			LABEL_2:
			;MOV isOdd
			mov eax, dword [ ebp + -4]
			;MOV2 0
			mov ebx, 0
			;SETE EBX
			cmp eax, ebx
			sete al
			movzx eax, al
			;CJMP EAX, LABEL_INDEX_4
			cmp eax, 0
			je LABEL_4
			;OPENSCOPE 
			push ebp
			mov ebp, esp
				;MOV num
				mov eax, dword [ ebp + 16]
				;MOV2 2
				mov ebx, 2
				;DIV EBX
				xor edx, edx
				div ebx
				;READ num
				mov dword [ ebp + 16], eax
				;CLOSESCOPE 
				mov esp, ebp
				pop ebp
			;LABEL 3
			LABEL_3:
			;MOV steps
			mov eax, dword [ ebp + 4]
			;ADD EBX
			inc eax
			;READ steps
			mov dword [ ebp + 4], eax
			;CLOSESCOPE 
			mov esp, ebp
			pop ebp
		;JMP LABEL_INDEX_6
		jmp LABEL_6
		;LABEL 4
		LABEL_4:
		;MOV steps
		mov eax, dword [ ebp + -8]
		;MOV2 maxSteps
		mov ebx, dword [ ebp + 12]
		;SETG EBX
		cmp eax, ebx
		setg al
		movzx eax, al
		;CJMP EAX, LABEL_INDEX_5
		cmp eax, 0
		je LABEL_5
		;OPENSCOPE 
		push ebp
		mov ebp, esp
			;MOV steps
			mov eax, dword [ ebp + 4]
			;READ maxSteps
			mov dword [ ebp + 24], eax
			;MOV testNumber
			mov eax, dword [ ebp + 16]
			;READ atNumber
			mov dword [ ebp + 20], eax
			;CLOSESCOPE 
			mov esp, ebp
			pop ebp
		;LABEL 5
		LABEL_5:
		;MOV testNumber
		mov eax, dword [ ebp + 4]
		;ADD EBX
		inc eax
		;READ testNumber
		mov dword [ ebp + 4], eax
		;CLOSESCOPE 
		mov esp, ebp
		pop ebp
	;JMP LABEL_INDEX_0
	jmp LABEL_0
	;LABEL 6
	LABEL_6:
	;MOV 0
	mov eax, 0
	;PRINT_STRING EAX
	mov ebx, eax
	add ebx, DATA_LABEL
	push ebx
	call _printf
	add esp, 4
	;MOV maxSteps
	mov eax, dword [ ebp + -4]
	;PRINT_NUMBER EAX
	push eax
	push PRINTF_NUMBER
	call _printf
	add esp, 8
	;MOV 10
	mov eax, 10
	;PRINT_STRING EAX
	mov ebx, eax
	add ebx, DATA_LABEL
	push ebx
	call _printf
	add esp, 4
	;MOV atNumber
	mov eax, dword [ ebp + -8]
	;PRINT_NUMBER EAX
	push eax
	push PRINTF_NUMBER
	call _printf
	add esp, 8
	;CLOSESCOPE 
	mov esp, ebp
	pop ebp
;RET 
ret
