#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>

#include "cust_struct.c"

void main(){
	
	int fd=open("customer.txt", O_CREAT|O_RDWR|O_TRUNC, 0744);
	
	struct cust admin; 
	strcpy(admin.usr_name,"admin");												
	strcpy(admin.pass, "admin123");							
	strcpy(admin.acc_num, "MAHB-PUN-NP-00001-admin");  
	
	admin.type=0;
	
	write(fd, &admin, sizeof(admin));
	close(fd);
	
	fd=open("account.txt", O_CREAT|O_RDWR|O_TRUNC, 0744);
	
	struct acc ad;
	strcpy(ad.acc_num, "MAHB-PUN-NP-00001-admin");
	ad.bal=0.0;
	
	write(fd, &ad, sizeof(ad));
	close(fd);
	
	fd=open("transaction.txt", O_CREAT|O_RDWR|O_TRUNC, 0744);
	close(fd);

}
