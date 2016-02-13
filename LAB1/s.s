#--------------------- s.s file --------------------------
         .global main, mymain, myprintf
main:    
		pushl  %ebp
		movl   %esp, %ebp
	
# (1). Write ASSEMBLY code to call myprintf(FMT)
#      HELP: How does mysum() call printf() in the class notes.
		
		pushl $FMT

		call myprintf
		addl $4, %esp #only add 4 back to the stack because we only pushed a single
					# 4 byte address
		

# (2). Write ASSEMBLY code to call mymain(argc, argv, env)
#      HELP: When crt0.o calls main(int argc, char *argv[], char *env[]), 
#            it passes argc, argv, env to main(). 
#            Draw a diagram to see where are argc, argv, env?
		
		pushl 16(%ebp) #env
		pushl 12(%ebp) #argv
		pushl 8(%ebp) #argc

		call mymain
		addl $12, %esp 


# (3). Write code to call myprintf(fmt,a,b)	
#      HELP: same as in (1) above
		
		pushl b
		pushl a
		pushl $fmt

		call myprintf
		addl $12, %esp

# (4). Return to caller
		movl  %ebp, %esp #this is same as popl %ebp? ask teacher
		popl  %ebp
		ret

#---------- DATA section of assembly code ---------------
	.data
FMT:	.asciz "main() in assembly call mymain() in C\n"
a:	.long 1234
b:	.long 5678
fmt:	.asciz "a=%d b=%d\n"
#---------  end of s.s file ----------------------------
