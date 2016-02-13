#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
//NODE type:

typedef struct node{
	char name[64];
	char type;
	struct node *childPtr;
	struct node *siblingPtr;
	struct node *parentPtr;
}NODE;

//Globals 
NODE *root, *cwd; /* root and CWD pointer */
char line[128]; /* user input line */
char command[32], pathname[64]; /*user inputs */
char dirname[64], basename[64]; /*string holders */

NODE *makeNode(){
	NODE *pNew;
	pNew = (NODE *) malloc(sizeof(NODE));
	pNew->childPtr = 0;
	pNew->siblingPtr = 0;
	pNew->parentPtr = 0;
return pNew;
}

int mkdir(){
	char *s, dirName[64], buffer[64];
	NODE *pCur, *pPrev;
	int i, flag;
	//cannot create another root directory
	if(strcmp(pathname,"/") == 0){	
		printf("mkdir: cannot create directory '%s': No such file or directory\n", dirname);	
		return 3;
	}

	if(pathname[strlen(pathname) - 1] == '/'){ //gets rid of extra forward slash
		pathname[strlen(pathname) - 1] = 0;
	}
	
	i = strlen(pathname) - 1;

	//absolute pathname
	if(pathname[0] == '/'){	
		while(pathname[i] != '/'){ //go to the first foward slash
			--i; //keep decreasing i
		}
		i+= 1; //add one more i because that's what is after the forward slash (parthname + i) below
			
		strcpy(basename, pathname + i); //add i to the address contained in pathname to get the basename
		strcpy(dirname, pathname); 	

		dirname[i] = 0; //i is where the string is split between directory and base name	
		
		pCur = root; 
	}
	else{ //relative

		//first we will check to see if
		strcpy(buffer, pathname);

		while(pathname[i] != '/' && i > 0){
			--i;
		}

		if(pathname[i] == '/'){
			i+=1;
			strcpy(basename, pathname + i);
			strcpy(dirname, pathname);

			dirname[i] = 0; //i is where the string is split between directory and base name	
		}

		else{ //no '/' was found meaning that it was only one word so mkdir in cwd
			strcpy(basename, pathname);
			strcpy(dirname, cwd->name);
		}
		pCur = cwd;
	}

	pPrev = pCur;	
	strcpy(buffer, dirname); //this is here to keep track of original dirname
	s = strtok(buffer, "/");
	
	if(s != 0)
	{
		//go down a level because we aren't going for root (only in the case of /)
		if(pathname[0] == '/' || (pCur == cwd && strcmp(cwd->name, "/") == 0)){ //second case is if the cwd is root
			pCur = pCur->childPtr; }
		if(pCur == 0){
			printf("mkdir: cannot create directory '%s': No such file or directory\n", dirname);
			return 1;
		}
		strcpy(dirName, s);
		//first find the dirnode
		do
		{
			flag = 0;
			//look for the node represented by dirName
		
			do{	
				pPrev = pCur;
				pCur = pCur->siblingPtr;
				if(strcmp(pPrev->name, dirName) == 0){ //we have found what we are looking for
					flag = 1;	
					break;
				}
			}while(pCur != pCur->parentPtr->childPtr);
			
			if(flag == 0){
				printf("mkdir: cannot create directory '%s': No such file or directory\n", dirname);
				return 1;
			}

			if((s = strtok(0, "/")) != 0){
				strcpy(dirName, s);
				pCur = pPrev->childPtr; //set our current to our previous's child pointer to go to next level
				if(pCur == 0) {//this means that it does not aline with our tokens, therefore dirname doesnt exist.
					printf("mkdir: cannot create directory '%s': No such file or directory\n", dirname);	
					return 1;
				}
			}
		}while(s);	
	}

	//a file node cant have a child node.	
	if(pPrev->type == 'F'){
		printf("cannot create directory '%s': Is a File\n", pathname);
		return 4;
	}	
	//add a new sibling to the family
	//first check for childPtr then siblingPtr
	if(pPrev->childPtr == 0){ //no childPtr or start of linked list is present
		pPrev->childPtr = makeNode();
		pCur = pPrev->childPtr; //move our current position to here
		pCur->parentPtr = pPrev;
		pCur->siblingPtr = pCur; //since we are the only sibling, point back to ourself.
	}
	else{
		//find the last node of the sibling linked list and add it there.
		pCur = pPrev->childPtr;
		
		do{ //while pCur is not equal to the oldest child.
			pPrev = pCur;
			pCur = pCur->siblingPtr;
			
			//we found the dirNode now lets check to see if the basename already exists
			if(strcmp(pPrev->name,basename) == 0){
			//we found the dirnode, but is it a directory or a file?
	
			if(pPrev->type == 'F'){
				printf("cannot create directory '%s': File exists", pathname);
				return 2;
			}
				printf("mkdir: cannot create directory '%s': Directory exists\n", pathname);
				return 3;	
			}
		}while(pCur != pCur->parentPtr->childPtr);

		pPrev->siblingPtr = makeNode();
		pCur = pPrev->siblingPtr;	
		pCur->parentPtr = pPrev->parentPtr;
		pCur->siblingPtr = pCur->parentPtr->childPtr;//point back to oldestchild.
	}
	
	pCur->type = 'D';
	strcpy(pCur->name, basename);

	return 0; //successfully made a directory
}

int rmdir(){
	NODE *pCur, *pPrev;
	char *s;
	int flag;
	char dirName[64], buffer[64];

	//if you want to remove root pointer, stop user from doing so
	if(strcmp(pathname, "/") == 0){
		printf("For your own good, please don't remove root.\n");	
		return 4;
	}

	strcpy(buffer, pathname);
	if(pathname[0] == '/'){
		pCur = root;
	}
	else{
		
		if(cwd->childPtr != 0){
			pCur = cwd->childPtr; //we go down a level because say we are in /a and want to cd to /a/b from /a it must mean that a has
			// a child where in that level we will look for b because there can also b /a/c /a/d etc
		}
		else{
			printf("cd: '%s': No such file or directory\n", pathname);	
			return 1;
		}
	}
	
	pPrev = pCur;
	//search for pathname node:
	s = strtok(buffer, "/");
	
	if(s != 0){ 
		//go down a level because we aren't going for root (only in the case of /)
		if(pathname[0] == '/' || (pCur == cwd && strcmp(cwd->name, "/") == 0)){
			pCur = pCur->childPtr;
		}
		if(pCur == 0){	
			printf("rmdir: cannot remove '%s': No such file or directory\n", pathname);	
			return 1;
		}
		strcpy(dirName, s);
		do
		{
			flag = 0;
			do{
				pPrev = pCur;
				pCur = pCur->siblingPtr;
				if(strcmp(pPrev->name, dirName) == 0){ //we have found what we were looking for
					flag = 1;
					break;
				}
			}while(pCur != pCur->parentPtr->childPtr); //while it's not equal to oldest child
			
			if(flag == 0){
				printf("rmdir: cannot remove '%s': No such file or directory\n", pathname);	
				return 1;
			}

			if((s = strtok(0, "/")) != 0){
				strcpy(dirName, s);
				pCur = pPrev->childPtr;
				if(pCur == 0){	
					printf("rmdir: cannot remove '%s': No such file or directory\n", pathname);	
					return 1;
				}
			}

		}while(s);
	}
	//at this point we know path exists.
	//check if it's a DIR type
	if(pPrev->type == 'F'){
		printf("rmdir: cannot remove '%s': Not a directory\n", pathname);
		return 2;
	}	

	if(pPrev->childPtr != 0){
		printf("rmdir: cannot remove '%s': Directory not empty\n", pathname);
		return 3;
	}
	
	if(pPrev == pPrev->siblingPtr){ //if there is only a childPtr, no siblingPtrs
		pCur = pPrev->parentPtr;
		free(pPrev);
		pCur->childPtr = 0;	

	}
	else if(pPrev == pPrev->parentPtr->childPtr){ //the childPtr is the one to delete
		pCur = pPrev->parentPtr;	
		pCur->childPtr = pCur->childPtr->siblingPtr; //set the childPtr equal to our new oldestChild
		free(pPrev);
	}
	else{ //default case is it's just some random sibling Ptr 
		pCur = pPrev;
		do{ //circle around to make pPrev equal the pointer that is currently being pointed to pPrev
			pPrev = pCur;
			pCur = pCur->siblingPtr;
		}while(strcmp(pCur->name, dirName) != 0);

		pPrev->siblingPtr = pCur->siblingPtr; //have pPrev's sibling be the next sibling in succession, could even be childPtr
		free(pCur);// kill this sibling, slaughter him have him bathe in blood.
	}

	return 0; //successfully removed directory
}

int creat(){

	char *s, dirName[64], buffer[64];
	NODE *pCur, *pPrev;
	int i, flag;
	//cannot create root file in actual linux
	if(strcmp(pathname,"/") == 0){	
		printf("creat: setting times of '%s': Permission denied\n", dirname);	
		return 3;
	}

	if(pathname[strlen(pathname) - 1] == '/'){ //gets rid of extra forward slash
		pathname[strlen(pathname) - 1] = 0;
	}
	
	i = strlen(pathname) - 1;

	//absolute pathname
	if(pathname[0] == '/'){	
		while(pathname[i] != '/'){ //go to the first foward slash
			--i; //keep decreasing i
		}
		i+= 1; //add one more i because that's what is after the forward slash (parthname + i) below
		
			
		strcpy(basename, pathname + i); //add i to the address contained in pathname to get the basename
		strcpy(dirname, pathname); 	

		dirname[i] = 0; //i is where the string is split between directory and base name	

		printf("dirname = %s, basename = %s\n", dirname, basename); 	
		
		pCur = root; 
	}
	else{ //relative

		//first we will check to see if
		strcpy(buffer, pathname);

		while(pathname[i] != '/' && i > 0){
			--i;
		}

		if(pathname[i] == '/'){
			i+=1;
			strcpy(basename, pathname + i);
			strcpy(dirname, pathname);

			dirname[i] = 0; //i is where the string is split between directory and base name	
		}

		else{ //no '/' was found meaning that it was only one word so mkdir in cwd
			strcpy(basename, pathname);
			strcpy(dirname, cwd->name);
		}
		pCur = cwd;
	}


	pPrev = pCur;	
	strcpy(buffer, dirname); //this is here to keep track of original dirname
	s = strtok(buffer, "/");
	
	if(s != 0)
	{
		//go down a level because we aren't going for root (only in the case of /)
		if(pathname[0] == '/' || (pCur == cwd && strcmp(cwd->name, "/") == 0)){
			pCur = pCur->childPtr;
		}
		if(pCur == 0){
			printf("creat: cannot creat '%s': No such file or directory\n", dirname);
			return 1;
		}
		strcpy(dirName, s);
		//first find the dirnode
		do
		{
			flag = 0;
			//look for the node represented by dirName
		
			do{	
				pPrev = pCur;
				pCur = pCur->siblingPtr;
				if(strcmp(pPrev->name, dirName) == 0){ //we have found what we are looking for
					flag = 1;	
					break;
				}
			}while(pCur != pCur->parentPtr->childPtr);
			
			if(flag == 0){
				printf("creat: cannot creat '%s': No such file or directory\n", dirname);
				return 1;
			}

			if((s = strtok(0, "/")) != 0){
				strcpy(dirName, s);
				pCur = pPrev->childPtr; //set our current to our previous's child pointer to go to next level
				if(pCur == 0) {//this means that it does not aline with our tokens, therefore dirname doesnt exist.
					printf("creat: cannot creat '%s': No such file or directory\n", dirname);	
					return 1;
				}
			}
		}while(s);	
	}

	//add a new sibling to the family
	//first check for childPtr then siblingPtr
	if(pPrev->childPtr == 0){ //no childPtr or start of linked list is present
		pPrev->childPtr = makeNode();
		pCur = pPrev->childPtr; //move our current position to here
		pCur->parentPtr = pPrev;
		pCur->siblingPtr = pCur; //since we are the only sibling, point back to ourself.
	}	
	else{
		//find the last node of the sibling linked list and add it there.
		pCur = pPrev->childPtr;
		
		do{ //while pCur is not equal to the oldest child.
			pPrev = pCur;
			pCur = pCur->siblingPtr;
			
			//we found the dirNode now lets check to see if the basename already exists
			if(strcmp(pPrev->name,basename) == 0){
				if(pPrev->type == 'D'){
					printf("creat: cannot creat '%s' : Directory exists\n", pathname);
					return 2;
				}
				printf("creat: cannot creat '%s': File exists\n", pathname);
				return 3;	
			}
		}while(pCur != pCur->parentPtr->childPtr);

		pPrev->siblingPtr = makeNode();
		pCur = pPrev->siblingPtr;	
		pCur->parentPtr = pPrev->parentPtr;
		pCur->siblingPtr = pCur->parentPtr->childPtr;//point back to oldestchild.
	}
	
	pCur->type = 'F';
	strcpy(pCur->name, basename);

	return 0; //successfully made a directory
}

int rm(){	
	NODE *pCur, *pPrev;
	char *s;
	int flag;
	char dirName[64], buffer[64];
	
	//dont have to worry about removing '/' since thats a directory type
	strcpy(buffer, pathname);
	if(pathname[0] == '/'){
		pCur = root;
	}
	else{
		pCur = cwd;
	}
	
	pPrev = pCur;
	//search for pathname node:
	s = strtok(buffer, "/");
	
	if(s != 0){ 
		//go down a level because we aren't going for root (only in the case of /)
		if(pathname[0] == '/' || (pCur == cwd && strcmp(cwd->name, "/") == 0)){
			pCur = pCur->childPtr;
		}
		if(pCur == 0){	
			printf("rm: cannot remove '%s': No such file\n", pathname);	
			return 1;
		}
		strcpy(dirName, s);
		do
		{
			flag = 0;
			do{
				pPrev = pCur;
				pCur = pCur->siblingPtr;
				if(strcmp(pPrev->name, dirName) == 0){ //we have found what we were looking for
					flag = 1;
					break;
				}
			}while(pCur != pCur->parentPtr->childPtr); //while it's not equal to oldest child
			
			if(flag == 0){
				printf("rm: cannot remove '%s': No such file\n", pathname);	
				return 1;
			}

			if((s = strtok(0, "/")) != 0){
				strcpy(dirName, s);
				pCur = pPrev->childPtr;
				if(pCur == 0){	
					printf("rm: cannot remove '%s': No such file\n", pathname);	
					return 1;
				}
			}

		}while(s);
	}
	//at this point we know path exists.
	//check if it's a DIR type
	if(pPrev->type == 'D'){
		printf("rm: cannot remove '%s': Not a file\n", pathname);
		return 2;
	}	
	
	if(pPrev == pPrev->siblingPtr){ //if there is only a childPtr, no siblingPtrs
		pCur = pPrev->parentPtr;
		free(pPrev);
		pCur->childPtr = 0;	

	}
	else if(pPrev == pPrev->parentPtr->childPtr){ //the childPtr is the one to delete
		pCur = pPrev->parentPtr;	
		pCur->childPtr = pCur->childPtr->siblingPtr; //set the childPtr equal to our new oldestChild
		free(pPrev);
	}
	else{ //default case is it's just some random sibling Ptr 
		pCur = pPrev;
		do{ //circle around to make pPrev equal the pointer that is currently being pointed to pPrev
			pPrev = pCur;
			pCur = pCur->siblingPtr;
		}while(strcmp(pCur->name, dirName) != 0);

		pPrev->siblingPtr = pCur->siblingPtr; //have pPrev's sibling be the next sibling in succession, could even be childPtr
		free(pCur);// kill this sibling, slaughter him have him bathe in blood.
	}

	return 0; //successfully removed directory
}

int cd(){
	char *s;
	char dirName[64], buffer[64];
	NODE *pCur, *pPrev;	
	int flag;
	//find pathname node.
		
	if(strcmp(pathname, "") == 0 || strcmp(pathname, "/") == 0){
		cwd = root; //set cwd to '/'	
		return 0;
	}

	strcpy(buffer, pathname);
	if(pathname[0] == '/'){	
		pCur = root;
		
	}
	else{
		
		if(cwd->childPtr != 0){
			pCur = cwd->childPtr; //we go down a level because say we are in /a and want to cd to /a/b from /a it must mean that a has
			// a child where in that level we will look for b because there can also b /a/c /a/d etc
		}
		else{
			printf("cd: '%s': No such file or directory\n", pathname);	
			return 1;
		}
	}


	
	pPrev = pCur;
	//search for pathname node:
	s = strtok(buffer, "/");
	
	if(s != 0){ 
		//go down a level because we aren't going for root (only in the case of /) or if we're at cwd and it is root
		if(pathname[0] == '/' || (pCur == cwd && strcmp(cwd->name, "/") == 0)){
			pCur = pCur->childPtr;
		}
		if(pCur == 0){	
			printf("1.cd: '%s': No such file or directory\n", pathname);	
			return 1;
		}
		strcpy(dirName, s);
		do
		{   
			flag = 0;
			do{
				pPrev = pCur;
				pCur = pCur->siblingPtr;
				if(strcmp(pPrev->name, dirName) == 0){ //we have found what we were looking for
					flag = 1;
					break;
				}
			}while(pCur != pCur->parentPtr->childPtr); //while it's not equal to oldest child
			
			if(flag == 0){
				printf("2.cd: '%s': No such file or directory\n", pathname);	
				return 1;
			}

			if((s = strtok(0, "/")) != 0){
				strcpy(dirName, s);
				pCur = pPrev->childPtr;
				if(pCur == 0){	
					printf("3.cd: '%s': No such file or directory\n", pathname);	
					return 1;
				}
			}

		}while(s);
	}

	if(pPrev->type == 'F'){
		printf("cd: '%s': Not a directory\n", pathname);	
		return 2;
	}
		
	cwd = pPrev;
	return 0;	
}

int ls(){	
	char *s;
	char dirName[64], buffer[64];
	NODE *pCur, *pPrev;	
	int flag;
	//find pathname node.
	
	if(strcmp(pathname, "") == 0){ //user inputed only ls
		pPrev = cwd;
	}
	
	else{
		strcpy(buffer, pathname);
		if(pathname[0] == '/'){
			pCur = root;
		}
		else{
			pCur = cwd;
		}
		
		pPrev = pCur;
		//search for pathname node:
		s = strtok(buffer, "/");
		
		if(s != 0){ 
			//go down a level because we aren't going for root (only in the case of /)
			if(pathname[0] == '/' || (pCur == cwd && strcmp(cwd->name, "/") == 0)){
				pCur = pCur->childPtr;
			}
			if(pCur == 0){	
				printf("ls: cannot access '%s': No such file or directory\n", pathname);	
				return 1;
			}
			strcpy(dirName, s);
			do
			{
				flag = 0;
				do{
					pPrev = pCur;
					pCur = pCur->siblingPtr;
					if(strcmp(pPrev->name, dirName) == 0){ //we have found what we were looking for
						flag = 1;
						break;
					}
				}while(pCur != pCur->parentPtr->childPtr); //while it's not equal to oldest child
				
				if(flag == 0){
					printf("ls: cannot access '%s': No such file or directory\n", pathname);	
					return 1;
				}

				if((s = strtok(0, "/")) != 0){
					strcpy(dirName, s);
					pCur = pPrev->childPtr;
					if(pCur == 0){	
						printf("ls: cannot access '%s': No such file or directory\n", pathname);	
						return 1;
					}
				}

			}while(s);
		}
	}
	
	pCur = pPrev->childPtr;
	
	if(pCur != 0){
		do{
			printf("%c %s\n", pCur->type, pCur->name); 
			pCur = pCur->siblingPtr;
		}while(pCur != pPrev->childPtr);	
	}

	return 0;
}

void rpwd(NODE *p){
	if(strcmp(p->name, "/") == 0){
		return;
	}
	rpwd(p->parentPtr);
	printf("/%s", p->name);
}

int pwd(){
	if(cwd != root)
		rpwd(cwd);
	else
		printf("/");
	printf("\n");
}

int save(char *filename){
	FILE *pFile = 0;
	int i;
	pFile = fopen(filename,"w+");
	NODE *pPrev, *pCur;
	char buffer[64], buffer2[64];

	pPrev = root;
	pCur = pPrev->childPtr;
	
	fprintf(pFile, "%c\t%s\n", pPrev->type, pPrev->name);

	if(pCur != 0){
		do
		{
			//strcpy(buffer, pPrev->name);
			while(pCur != 0){
				strcat(buffer, "/");
				strcat(buffer, pCur->name);

				fprintf(pFile, "%c\t%s\n", pCur->type, buffer);
				pPrev = pCur;
				pCur = pCur->childPtr;
			}
			//finally we have hit pCur zero
			//set pCur up one level to the sibling
			//we have to print ou that whole level now because post order
			pCur = pPrev->siblingPtr; //dont want pPrev because we've already printed that one
			
			//put the parent name in our buffer and get rid of the very last file
			i = strlen(buffer) - 1;
			while(buffer[i] != '/'){
				--i;
			}
			
			buffer[i] = 0;

			while(pCur != pCur->parentPtr->childPtr){
				strcpy(buffer2, buffer); // since we're dealing with siblings, all will have the same parent
				strcat(buffer2, "/");
				strcat(buffer2, pCur->name);

				fprintf(pFile, "%c\t%s\n", pCur->type, buffer2);
				pPrev = pCur;
				pCur = pCur->siblingPtr;
			}	
			//now go back up one level
			pCur = pCur->parentPtr;
			pPrev = pCur; //set our previous to where our parent is since we've already printed this parent out
			
			if(pCur->siblingPtr == pCur->parentPtr->childPtr){ // this means we are at the end of the linked list.
				pCur = pCur->parentPtr;
			}
			else{	
				pCur = pCur->siblingPtr; //we haven't printed this guy out, get ready for the next loop unless we are infact in root
			}

			strcpy(buffer, ""); //reset our buffer
		}while(pCur != root);
	}
	return 0; //successfully saved.	
}

int reload(char *filename){
		
	FILE *pFile = 0;
	char *s;
	
	initialize(); //re-init, chngl8r
	
	pFile = fopen(filename, "r");
	fgets(line, sizeof(line), pFile); //accounts for the root node

	while(fgets(line, sizeof(line), pFile)){	
		line[strlen(line) - 1] = 0;
		s = strtok(line, "\t");	
		

		strcpy(command, s);

		if((s = strtok(0, " ")) != 0){
			strcpy(pathname, s);
		}			
		else{
			strcpy(pathname, "");
		}
		if(strcmp(command, "D") == 0){
			mkdir();
		}
		else{
			creat();
		}
	}

	return 0; //successfully saved.	
}

int initialize(){
	root = makeNode();
	root->siblingPtr = root;
	root->parentPtr = root;	
	root->type = 'D';
	strcpy(root->name, "/");
	cwd = root;//point current working directory to root
}

int (*fptr[])(char *) = {(int (*)())mkdir, rmdir, ls, cd, pwd, creat, rm, save, reload};

int findCommand(char *command){
	int ID = 0;
	char *ar[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "save", "reload"};

	while(ID < 9){
		
		if(strcmp(command, ar[ID]) == 0){
			return ID;
		}
		ID++;	
	}

	return -1; //no command has been found.
}

int main(int argc, char* argv[], char* env[]){
	int ID, r;
	int i = 0;
	NODE *p, *childPtr;
	char *s;

	initialize();
	while(1){
		printf("input a command : ");
		fgets(line, 128, stdin); //get input for line
		line[strlen(line) - 1] = 0;
		s = strtok(line, " ");	
		if(s != 0){
			strcpy(command, s);
			if((s = strtok(0, " ")) != 0){
				strcpy(pathname, s);
			}
			else{
				strcpy(pathname, "");
			}
		}	
		//sscanf(line, "%s %s", command, pathname);
		ID = findCommand(command);
		if(ID >= 0 && ID <= 8){
			r = fptr[ID](pathname);
		}
		else if(strcmp("quit", command) == 0 || strcmp("exit", command) == 0){
			printf("exiting shell...\n");
			break;
		}
		else {
			printf("shell simulator: %s: command not found\n", command);
		}	

	}	

	return 0;
}
