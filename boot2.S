//boot2.s
.intel_syntax noprefix
.global show_eax
.global clearScr
.global startup

eax_msg: .ascii "The value in the EAX register is: "
eax_val: .asciz "0000000000"  // ten spaces, the max number of digits in an unsigned int
cursor_location:.short 0
color: .byte 31
/*
show_eax:
     	set up ebp
       push offset of eax_val onto stack
       push val of eax onto stack
       call the C convert_num function
       clean up stack
       push parameters for writescr onto stack to print eax_msg
       call writescr
       clean up stack */




	

show_eax:
	push ebp		//Setup stack frame
	mov ebp, esp
	pushf

	call clearScr
	push offset eax_val	//Setup stack for convert num, send eax value, and str buffer.

	push eax
	call convert_num	//call C convert num func.

	add esp, 8		//"clean" stack.

	push 19			//push on col, row, and str ptr.
	push 11
	push offset eax_msg
	call writeScr
	add esp, 12		// call writeScr, then clean up stack.
	
	
	popf
	pop ebp			//Clear stack frame.
	
       ret


writeScr:
	push ebp
	mov ebp, esp	//Setup stack frame
	pushf
	
	push esi			//PUSH to preserve esi
	mov esi, [ebp+8]		//Load esi with the contents in memory location ebp+8; The address to the start of the mt string.
	push eax			//PUSH eax to preserve state.
	mov eax, [ebp+12]		//Load eax with the contents of memory in location ebp+12; the row.
	imul eax, 80			//multiple the row number by 80 to get the current row.
	add eax, [ebp+16]		//Add eax with the contents of memory in location ebp+16; the col;
	imul eax, 2			//multiply by two to get the correct offset, accounting for the color bits of each char.
	add eax, 0xb8000		//Offest to video memory.
	push edi			//PUSH to preserve edi
	mov edi, eax
	mov ebx, offset color
	mov eax, [ebx]
writeScr_char:				//Write individual characers to the screen, from a null terminated string.
	mov bl, [esi]			//Load the char from he address pointed to by esi.
	cmp bl, 0			//Compare that will 0, or null, to determine if end of string.
	je writeScr_done		//If end of string jump to cleanup procedure, otherwise write contine.
	movsb				//Actually write a char from the string to memory address for video.
	
			 		//Use eax register as temp variable to hold color byte.
	mov [edi], al			//move temp variable into memory address referenced by edi.
	inc edi				//increment edi so that movsb doesn't overwrite color bit.
	jmp writeScr_char		//Repeat till null termination is found.
writeScr_done:				//While string written, clean up.
	pop edi
	pop eax				//Return stored values from stack.
	pop esi
	
	popf
	pop ebp				//clean up frame buffer.
	ret




clearScr:
	push ebp			//Setup stack frame
	mov ebp, esp
	pushf

	push eax			//PUSH2 preserve eax, and ebx.
	push ebx
	
	mov eax, offset End_of_S	//Load eax with the value of the memory location that corresponds to mt space used for temp variables by Kernel.
	mov ebx, offset 2001		//Use ebx as counter to keep up with current location in string building process.
clearScr_build_string_loop:		//Build a string of 2000 spaces.
	dec ebx				//Update counting variable.
	cmp ebx, 0			//Compare counting variable to zero, if zero, end of string reached.
	je clearScr_build_string_end	//Jump to "printing" step of clearScr.
	movb [eax], 32			//Mov 32('space') into the memory address pointed to by eax.
	inc eax				//Increment eax to update current location for writing spaces.
	jmp clearScr_build_string_loop	//Repeat till 2000 spaces loaded into memory.

clearScr_build_string_end:		//Calls write screen to print spaces, and cleans up.
	movb [eax], 0			//Move a zero into eax for null termination.
	mov eax, 0			//Move the zero into the end of the string so that it is null terminated.
	push eax			//PUSH 0 for col number.
	push eax			//PUSH 0 for row number.
	mov eax, offset End_of_S	//Load eax with the address of the begginning of the string.
	push eax			//PUSH the address for the begginning of the string, or char buffer.
	call writeScr			//call writeScr to write the mt string to the screen.
	pop eax
	pop eax				//pop passed values from the stack.
	pop eax

	pop ebx				//return original state to ebx, and eax.
	pop eax

	popf
	pop ebp				//clean up stack frame
	ret
Stay:

	jmp Stay
	ret	
	

End_of_S:
.fill 5000, 1, 0






