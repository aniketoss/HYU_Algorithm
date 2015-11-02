extern int e;
int *ep = &e;

int x = 15;
int y;

int a(){
	return *ep + x + y;

}