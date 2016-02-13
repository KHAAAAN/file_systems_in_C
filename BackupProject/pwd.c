#include "type.h"

void pwd(){
	char tempString[512];
	char path[512];
	int dot, dot2;
	MINODE *tempMip;

	//root case- if . and .. have the same inode
	
	findino(running->cwd, &dot, &dot2);
	if(dot == dot2){
	
		printf("/\n");
		return;
	}

	tempMip = iget(running->cwd->dev, dot2);
	findmyname(tempMip, dot, tempString);		
	
			
	//once . and .. have the same number, means we're at root
	do{	
		tempMip = iget(running->cwd->dev, dot2);
		findmyname(tempMip, dot, tempString);		
		
		//copy the found name of . into path
		strcpy(path, tempString);
		//reset tempString
		strcpy(path, tempString);


		//set the new . and ..
		findino(tempMip, &dot, &dot2);

		//put the MINODE back	
		iput(tempMip);
		
		//add a / to the beginning 	
		strcpy(tempString, "/");
		strcat(tempString, path);	
		strcpy(path, tempString);
	}while(dot != dot2);
	

	printf(". = %d and .. = %d\n", dot, dot2);
	printf("%s\n", path);	
}
