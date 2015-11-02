extern int BUF[];

void swap()
{
	int temp;

	temp = BUF[1];
	BUF[1] = BUF[0];
	BUF[0] = temp;
}