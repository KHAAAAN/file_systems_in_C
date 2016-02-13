/****************** t.c file **********************/
#include <stdio.h>
#include <stdlib.h>

int *FP; //global uninitialized

main(int argc, char *argv[], char *env[])
{
	int a, b, c; //uninitialized locals
	printf("enter main\n");
	printf("&argc=%x argv=%x env=%x\n", &argc, argv, env);
	printf("&a=%8x &b=%8x &c=%8x\n", &a, &b, &c);
	
	a=1; b=2; c=3;
	A(a, b);
	printf("exit main\n");
}

int A(int x, int y)
{
	int d, e, f;
	printf("enter A\n");
	// PRINT ADDRESS OF d, e, f
	printf("&d=%8x &e=%8x &f=%8x\n", &d, &e, &f);
	d=4; e=5; f=6;
	B(d, e);
	printf("exit A\n");
}

int B(int x, int y)
{	int g,h,i;
	printf("enter B\n");
	// PRINT ADDRESS OF g,h,i
	printf("&g=%8x &h=%8x &i=%8x\n", &g, &h, &i);
	g=7; h=8; i=9;
	C(g, h);
	printf("exit B\n");
}

int C(int x, int y)
{
	int u, v, w, i, *p;
	printf("enter C\n");
	// PRINT ADDRESS OF u,v,w;
	printf("&u=%8x &v=%8x &w=%8x\n", &u, &v, &w);	
	u=10; v=11; w=12;
	
	//(1) Print the stack frame link list
	printf("****Stack frame linked list****\n");

	asm("movl %ebp, FP");	// CPU's ebp register is the FP (Frame Pointer)
	printf("FP = %8x\n", FP);

	p = FP;
	do
	{
		p = (int *) *p;

		printf("FP = %8x\n", p);

	}while (p != 0); //0 is the value of crt0.o's ebp.		
	
	//(2) print stack contents from FP-8 to the frame of main()
	printf("****Stack contents from FP-8 to the frame of main()****\n");
	for(i = -2; i < 98; i++)
	{
		printf("Stack Contents at %d(FP): %8x with value %8x.\n", i*4, (int *) (FP + i), (int *) *(FP + i));
	}
}

