//boot2.s
.intel_syntax noprefix
.global show_eax
.global clearScr
.global outportb
.global default_handler
.global lidtr
.global kdb_enter
.global writeScr
.global causeFault
.global go
.global schedule
.global init_timer_device
.global color
//Kernel variables section.
eax_msg: .ascii "The value in the EAX register is: "
eax_val: .asciz "0000000000"  // ten spaces, the max number of digits in an unsigned int
keyscan_msg: .ascii "Got a key: "
keyscan_val: .asciz "0000000000"
cursor_location:.short 0
color: .byte 31
Kernel_Panic_msg:	.ascii "The kernel has experienced a panic, "
					.asciz "sorry.... System will now halt, reccomend taking a core dump."
Schedule_Fault_msg1:	.asciz "The kernel fault was caused by a problem with the scheduler & addProcess!"
Schedule_Fault_msg2:	.asciz "The kernel fault was caused by a problem with the scheduler & getProcess!"

IntCounter: .short 0

ms_delay:
	push ebp
	mov ebp, esp
	pusha
	mov eax, [ebp+4]
	cmp eax, 0
	jb _ms_delay_end

_ms_delay_outer_loop:
	cmp eax, 0
	jz _ms_delay_end
	dec eax

	mov edx, 100000
_ms_delay_loop:
	cmp edx, 0
	jz _ms_delay_end
	dec edx
	nop
	nop
	pusha
	popa
	pusha
	popa
_ms_delay_end:
	popa
	pop ebp
	ret

causeFault:
	push eax
	push ebx
	mov eax, 50
	mov ebx, 0
	div bl
	pop ebx
	pop eax
	ret

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


//Utilities

lidtr:
	 push ebp
	 mov ebp, esp				//Stack fram
	 push eax
	 mov eax, [ebp+8]		//preserve and load eax with passed mem location for idt table
	 lidt [eax]			//Load hardware IDT with passed idt table.
	 pop eax
	 pop ebp
	//lidt [esp+4]
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
	push ebx
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
	inc edi				//increment edi so that movsb doesn''t overwrite color bit.
	jmp writeScr_char		//Repeat till null termination is found.
writeScr_done:				//While string written, clean up.
	mov eax, edi
	sub eax, 0xb8000
	ror eax, 1
	push eax
	call updateCursor
	add esp, 4
	pop ebx
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
	dec ebx						//Update counting variable.
	cmp ebx, 0					//Compare counting variable to zero, if zero, end of string reached.
	je clearScr_build_string_end	//Jump to "printing" step of clearScr.
	movb [eax], 32				//Mov 32('space') into the memory address pointed to by eax.
	inc eax						//Increment eax to update current location for writing spaces.
	jmp clearScr_build_string_loop	//Repeat till 2000 spaces loaded into memory.

clearScr_build_string_end:		//Calls write screen to print spaces, and cleans up.
	movb [eax], 0				//Move a zero into eax for null termination.
	mov eax, 0					//Move the zero into the end of the string so that it is null terminated.
	push eax					//PUSH 0 for col number.
	push eax					//PUSH 0 for row number.
	mov eax, offset End_of_S	//Load eax with the address of the begginning of the string.
	push eax					//PUSH the address for the begginning of the string, or char buffer.
	call writeScr				//call writeScr to write the mt string to the screen.
	pop eax
	pop eax						//pop passed values from the stack.
	pop eax

	pop ebx						//return original state to ebx, and eax.
	pop eax

	popf
	pop ebp						//clean up stack frame
	ret




Stay:

	jmp Stay
	ret	


outportb:
	push ebp
	mov ebp, esp;			//SEtup stack frame.
	push eax				//Preserve register state, and clear them.
	push edx
	pushf
	mov eax, [ebp+12]		//Load byte arg for port.
	mov edx, [ebp+8]		//Load the port address.
	lock
	out dx, al
	popf
	pop edx
	pop eax
	pop ebp
	ret

go:
	//I''m never returning to main... remove return call cs, and flags.
	//add esp, 4						
	mov eax, offset process_queue		//load address of PQ into eax
	push eax							//Pass pq to getProcess, dequeue
	call getProcess						//Call to get next process.
	# mov ebx, offset currentProcess		//Load pointer curproc with new current process that was dequeued.
	# mov [ebx], eax
	mov [currentProcess], eax						
	add esp, 4							//Clean stack.
	mov esp, [eax]						//Move processes sp into esp.
	pop gs								//Return state of new curProc.
	pop fs 
	pop es
	pop ds
	popad
	iret								//Return to cur proc.



init_timer_device:
	push ebp
	mov ebp, esp						//Setup Stack frame.
	pushf
	push eax							//Preserve existing register state.
	push ebx
	push edx
	mov edx, [ebx+8]					//Load passed time constant into edx.
	mov eax, offset 1196				//Move 1196 into eax for mul with dx.
	mul dx								//AX now holds lower 16 bits for timer device, that all we care about...
										//DX holds MSW Most signigicant word, not using it...
	xchg eax, ebx						//need to move eax val into ebx, need al for out instructions.
	mov al, 0b00110110
	lock
	out 0x43, al						//Send 0x43 to pit, the control byte.
	xchg ebx, eax						//Need old value of eax back.
	lock
	out 0x40, al						//Write LSB for PIT.
	xchg al, ah
	lock 
	out 0x40, al						//Write MSB for PIT.

	pop edx
	pop ebx
	pop eax
	popf
	pop ebp								//Clean state, and return.
	ret


	


//Interupt Section.

default_handler:
	push offset 0						//Push starting address of Kernel panic message.
	push offset 5						
	//Push location on screen to start writing the message.
	push offset Kernel_Panic_msg
	call writeScr
	add esp, 12
	iret

schedule:
	pushad								//Preserve Current program state
	push ds
	push es
	push fs
	push gs

	mov eax, [currentProcess]			//Move the pointer to the current process''s PCB into eax.
	mov [eax], esp						//Move the stack pointer value into sp for current process.
	mov ebx, offset process_queue		//Load b with address of the process_queue.
	push eax
	push ebx							//Push addProcess args onto stack in rev order.
	call addProcess						//Call addProcess to place currently running process onto stack.
	add esp, 8							//Clean Stack.
	cmp eax, 0							//Check for errors in adding process.
	jnz _schedule_cont_causeFault		//If no errors move to grabbing next process of process_queue
	//call causeFault					//Cause a div 0 fault.
	mov eax, offset Schedule_Fault_msg1	//Write error message to screen and halt the system.
	push 77
	push 0
	push eax
	call writeScr
	hlt

_schedule_cont_causeFault:
	push offset process_queue			//Push process_queue pointer onto stack.
	call getProcess						//Grab next process off the process_queue
	add esp, 4
	cmp eax, 0							//Check for errors.
	jnz _schedule_cont_getProcess		//cont if no errors.
	//call causeFault
	mov eax, offset Schedule_Fault_msg2	
	push 77
	push 0
	push eax
	call writeScr
	hlt
_schedule_cont_getProcess:
	mov [currentProcess], eax
	mov esp, [eax]
	pop gs
	pop fs
	pop es
	pop ds
	popad
	push eax
	mov al, 0x20
	out 0x20, al
	pop eax
	iret







kdb_enter:
	pushf
	cli
	pusha
	xor eax, eax				//Clear values in eax register.
	in al, 0x64					//Check if anything available in physically mapped keyboard port.
	and al, 0x01				//Clear upper bits to check keyboard status for available keys.
	jz _kdb_skip				//If there isn''t anything to be read from the keyboard continue...
	in al, 0x60					//Read value from keyboard mem
	push eax 					//Push value of scancode to the stack
	call kdb_handler			//Add character to keyboard buffer. In Theory.
	add esp, 4					//Cleanup stack from buffer call.
_kdb_skip:
	mov al, 0x20				//Load al with hex 20, which symbolizes end of interupt to keyboard.
	out 0x20, al				//output End-of_Interrupt to keyboard.
	popa
	popf						//Return initial values to registers, and return from interrup.tsetup.office365.com

	iret

kdb_handler:
	push ebp
	mov ebp, esp
	pusha
	mov eax, [ebp+8]
	cmp eax, 0
	jz _kdb_handler_skip
	mov ebx, offset keyboard_buffer
	mov ecx, [ebx+6]
	mov edx, [ebx+8]
	cmp ecx, edx
	jz _kdb_handler_skip
	push eax
	call translate_scancode
	add esp, 4
	push eax
	push offset keyboard_buffer
	call putItem
	add esp, 8
_kdb_handler_skip:
	popa
	pop ebp
	ret



//Space reserved for OS data structures, Do not declare any functions past this point.
End_of_S:
.fill 5000, 1, 0			//MT space for filling strings.






