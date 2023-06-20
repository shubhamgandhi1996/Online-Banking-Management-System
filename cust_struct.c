#include<stdio.h>
#include<stdlib.h>

struct cust{
	char usr_name[1024];
	char pass[1024];
	char acc_num[1024];
	int  type;
	/*
		0 --> Admin
		1 --> single Account user
		2 --> joint Account user
	*/
};

struct acc{
	char acc_num[1024];
	double bal;
};

struct trans{
	char date[1024];				
	char acc_num[1024];				
	double amt_credit;			
	double amt_debit;									
	double bal_remaining;												
};

