.text

.global ws2812_send

.equ  mainled_bit0, 0x0
.equ  mainled_bit1, 0x1

bitnum  .req r6
bitmask .req r5
totpix  .req r3
	
// r0  uint8_t *fb
// r1  uint32_t	len

	// r2 is GPIO dest location
	// r0 is "0" bit number code
	// r1 is "1" bit number code
	// r3 is loop counter for total number of pixels
	// r4 is the current pixel value
	// r5 is the test value for the top bit of current pixel
	// r6 is the loop counter for bit number in a pixel
	// r7 is current pointer to the pixel array
	
ws2812_send:
	push {r4,r5,r6,r7}

	mov r7, r0       // r7 gets to be the pointer to the pixel array

	mov totpix, r1             // r3 gets number of pixels to go through
	
	ldr r2, GPIOB_BSRR
	ldr r0, MAINLED_BIT0
	ldr r1, MAINLED_BIT1
	ldr bitmask, TOP24_MASK

	// hoist this outside the loop and complete within iters
	ldr r4, [r7]      // load the word at the pointer location to r4
	b looptop
	
test1i_d:
	nop
	nop
test1i:	
	str r1, [r2]
	lsl r4, #1      
	sub bitnum, bitnum, #1  
	beq exit_pix_1   // beq branches if zbit is 1
	nop
	nop
	nop
	nop
	str r0, [r2]
	tst r4, bitmask     // zbit = !(r4 AND r5), so zbit set if bit is zero
	bne test1i     // beq branches if bit is zero; bne branches if bit is one
	nop            // fill in a delay if branch not taken

test0i:	
	str r1, [r2]  // 1
	lsl r4, #1      
	str r0, [r2]  // 0
	nop
	sub bitnum, bitnum, #1  
	beq exit_pix
	
	tst r4, bitmask   // dispatch to on or off bit
	bne test1i_d
	nop
	nop
	b   test0i
	
exit_pix:
	sub totpix, totpix, #1
	bne looptop
	b exit

exit_pix_1:
	nop
	nop
	nop
	str r0, [r2]
	sub totpix, totpix, #1
	bne looptop
	b exit
	
looptop:	
	ldr r4, [r7]      // load the word at the pointer location to r4
	mov bitnum, #24
	add r7, r7, #4     // increment the pointer to the next word

	tst r4,bitmask
	bne test1i
	beq test0i  // redundant branch to balance outclocks

exit:	
	
	pop {r4,r5,r6,r7}
	bx lr
	
.balign 4
GPIOB_BSRR:
.word 0x40010c10
MAINLED_BIT0:
.word 0x10000
MAINLED_BIT1:
.word 0x1
TOP8_MASK:
.word 0x80
TOP24_MASK:
.word 0x800000
	
/*
test1:	
	str r1, [r2]
	mov r0, r0
	mov r0, r0
	mov r0, r0
	mov r0, r0
	mov r0, r0
	mov r0, r0
	mov r0, r0
	str r0, [r2]
	sub r3, r3, #1
	bne test1

	mov r3, #24
test0:	
	str r1, [r2]
	mov r0, r0
	str r0, [r2]
	mov r0, r0
	mov r0, r0
	mov r0, r0
	mov r0, r0
	mov r0, r0
	mov r0, r0
	sub r3, r3, #1
	bne test0
*/
	
.end
	
