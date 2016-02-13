#include <stdio.h>

typedef unsigned int u32;
int BASE;
char *table = "0123456789ABCDEF";
char *cp;
int *ip;

int rpu(u32 x) {
	char c;
	if (x){
		c = table[x % BASE];
		rpu(x / BASE);
		putchar(c);
	}
}

int printu(u32 x)
{
	BASE = 10;
	if (x == 0)
		putchar('0');
	else
		rpu(x);
	putchar(' ');
}

int printd(int x)
{
	BASE = 10;
	if(x == 0)
		putchar('0');
	else
	{
		if (x < 0)
		{
			putchar('-');
			x = -x;
		}

		rpu(x);
	}
	putchar(' ');
}

int printo(u32 x)
{
	BASE = 8;
	if(x == 0)
		putchar('0');
	else
	{
		putchar('0');
		rpu(x);
	}
	putchar(' ');
}

int printx(u32 x)
{
	BASE = 16;
	if(x == 0)
		putchar('0');
	else
	{
		printf("0x");
		rpu(x);
	}
	putchar(' ');
}

int prints(char *s)
{
	while(*s != 0){
		putchar(*s++);
	}	
}

int myprintf(char *fmt, ...)
{	
	//for most instructions that reference memory you must move it to/from a register, since we have an offset of 4 bytes we move into ecx
	//ecx is called the counter register. It is used as a loop counter and for shifts, in this case shift.
	
	//asm("movl 8(%ebp), %ecx"); 
	//asm("movl %ecx, cp"); 
	
	//^ can do it either way
	
	asm("movl %ebp, ip");
	ip += 2;
	cp = (char *) *ip;

	asm("movl %ebp, ip");
	
	ip += 3; // + 12 bytes to get to first parameter
		
	while(*cp != 0){
		
		if(*cp == '%' && *(cp+1) != 0){ //check that cp+1 isn't the end of the string. i.e if you have a string "you got 100%"	
			switch(*(cp + 1)){
				case 'c':
					putchar(*ip);
					cp += 2;
					ip++;
					break;
				case 's':
					prints((char *)*ip);
					cp += 2;
					ip++;
					break;
				case 'u':
					printu(*ip);
					cp += 2;
					ip++;
					break;
				case 'd':
					printd(*ip);
					cp += 2;
					ip++;
					break;
				case 'o':
					printo(*ip);
					cp += 2;
					ip++;
					break;
				case 'x':
					printx(*ip);
					cp += 2;
					ip++;
					break;
				default:
					putchar(*cp++); //incase it's some other letter like %m or %a
			}	
		}
		else if(*cp == '\n'){
			putchar('\n');
			putchar('\r');
			cp++;
		}
		else{
			putchar(*cp++);
		}
	}  
		

}

mymain(int argc, char *argv[ ], char *env[ ])
{
  int i;

  myprintf("in mymain(): argc=%d\n", argc);

  for (i=0; i < argc; i++)
      myprintf("argv[%d] = %s\n", i, argv[i]);
  
  // WRITE CODE TO PRINT THE env strings

  myprintf("---------- testing YOUR myprintf() ---------\n");
  myprintf("this is a test\n");
  myprintf("testing a=%d b=%x c=%c s=%s\n", 123, 123, 'a', "testing");
  myprintf("string=%s, a=%d  b=%u  c=%o  d=%x\n",
           "testing string", -1024, 1024, 1024, 1024);
  myprintf("mymain() return to main() in assembly\n"); 
}

/*int main(void)
{
	//myprintf("testing %c\ntesting %s\ntesting %u\ntesting %d\ntesting %o\ntesting %x\n", '1', "two", 34, -55, 100, 15);	

	return 0;
}*/
