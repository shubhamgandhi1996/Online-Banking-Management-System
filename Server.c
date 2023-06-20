#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdbool.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<time.h>
#include "cust_struct.c"

//For admin
void admin_add(int nfd);	
void admin_delete(int nfd);				
void admin_delete_account(char arr[1024], int nfd);				
void admin_delete_username(char username[1024], int nfd);					
void admin_modify(int nfd);				
void admin_search(int nfd);	
void admin_print_customers(int nfd);
void admin_print_accounts(int nfd);
void admin_print_transactions(int nfd);					

//For single
void single_deposit_money(char acc_num[1024], int fd);
void single_withdraw_money(char acc_num[1024], int nfd);

//comman to Single and joint
void print_trans(char acc_num[1024], int type, int nfd);
void print_acc(char acc_num[1024], int type, int nfd);	
void pass_change(char usr_name[1024], int nfd);

//For joint
void joint_deposit_money(char acc_num[1024], int nfd);
void joint_withdraw_money(char acc_num[1024] , int nfd);


//SERVER SIDE
int main(){

	char usr_name[1024], pass[1024];
	memset(usr_name, 0, sizeof(usr_name));
	memset(pass, 0, sizeof(pass));
		
	struct sockaddr_in server, client;
	int fd, nfd, client_len;
	char buff[60],c;
	
	if( (fd=socket(AF_INET, SOCK_STREAM, 0))==-1 ){
		perror("\nERROR1: ");
		exit(1);
	}
	server.sin_family= AF_INET;
	server.sin_addr.s_addr= INADDR_ANY;
	server.sin_port= htons(5555); 
	if( bind(fd, (struct sockaddr *)&server, sizeof(server))==-1 ){
		perror("\nERROR2: ");
		exit(1);
	}
	if( listen(fd, 2)==-1 ){ //queue length
		perror("\nERROR3: ");
		exit(1);
	}
	write(1, "\nWaiting for the Client to send connect to the server....", sizeof("\nWaiting for the Client to send connect to the server...."));
	
	while(1){
	
		client_len=sizeof(client);
		nfd=accept(fd, (struct sockaddr *)&client, &client_len); 
		write(1, "\nConected to the Client...", sizeof("\nConected to the Client..."));
		
		if(!fork()){//child 
			close(fd);  //child does not require
			write(nfd, "\nConnected to the Server...", sizeof("\nConnected to the Server..."));
			/*
				nfd ---> socket desc.
			*/
			int fd=open("customer.txt", O_RDWR, 0744);
			
			int login_check=0; 
			int choice;
			struct cust check;
			
			read(nfd, usr_name, sizeof(usr_name)); // Reading UserName and Password from the Client.
			read(nfd, pass, sizeof(pass));
			
	while( read(fd, &check, sizeof(check)) != 0){ //To check is the username exists or not
		if(strcmp(check.usr_name, usr_name)==0 && strcmp(check.pass, pass)==0 )
		{
				
				login_check=1;
				break;
		}
	} 
	
	close(fd);
	if(login_check==0) // if entered usr_name and pass does not  match
	{
		write(nfd, &login_check, sizeof(login_check));
	}else{ //**********************FUN BEGINS******
	 
	 write(nfd, &login_check, sizeof(login_check));
	 write(nfd, &check.type, sizeof(check.type));			
	
	if(check.type == 0) // For admin
	{
		int choice;
		while(1){
		read(nfd, &choice, sizeof(choice));
		
		if(choice==1) 
			admin_add(nfd);
		else if(choice==2) 
			admin_delete(nfd);
		else if(choice==3)
			admin_modify(nfd);
		else if(choice==4)
			admin_search(nfd);
		else if(choice==5)
			{
			admin_print_customers(fd);
			admin_print_accounts(fd);
			}
		else if(choice==6){
			close(nfd);
			exit(1);
		}}
	} 		
	else if(check.type==1) // Single Account user
	{
		int choice;
		while(1){
		read(nfd, &choice, sizeof(choice));
		write(nfd, &check, sizeof(check));
			
		if(choice==1)						
			single_deposit_money(check.acc_num, nfd); //sending account_number and then askinng other details 
		else if(choice==2)					
			single_withdraw_money(check.acc_num, nfd); 					
		else if(choice==3)					
			print_acc(check.acc_num, 0, nfd);									
		else if(choice==4)						
			pass_change(check.usr_name, nfd);					
		else if(choice==5)						
			print_trans(check.acc_num, 0, nfd); // print transactions details
		else if(choice==6){
			close(nfd);
			exit(1);
		}}						
	}
	else if(check.type==2) // for Joint Account Holder
	{
		int choice;
		
		while(1){
		read(nfd, &choice, sizeof(choice));
		write(nfd, &check, sizeof(check));
		
		if(choice==1)
			joint_deposit_money(check.acc_num, nfd);  
		else if(choice==2)
			joint_withdraw_money(check.acc_num, nfd); 
		else if(choice==3)
			print_acc(check.acc_num, 1, nfd);
		else if(choice==4)
			pass_change(check.usr_name, nfd);
		else if(choice==5)
			print_trans(check.acc_num, 1, nfd); // read lock
		else if(choice==6){
			close(nfd);
			exit(1);
			}	
	 	
		}	
	   }
	
	}
			exit(1);
	}else{ //for Parent
			close(nfd);//child has these parent does not require it
		}
	}	
	close(fd);
	return 0;
}	

//for Single Account User	
void single_deposit_money(char acc_num[1024], int nfd)					
{					
	double amt;								
	read(nfd, &amt, sizeof(amt));	//Reading the amount to be deposited from the user
								
	int fd=open("account.txt", O_RDWR, 0744);

	int found=0;
	struct acc temp;				
	while(read(fd, &temp, sizeof(temp)) != 0){
	
		if(strcmp(temp.acc_num, acc_num)==0){
			temp.bal = temp.bal + amt; //performing addition
			lseek(fd, -sizeof(temp), SEEK_CUR);
			write(fd, &temp, sizeof(temp)); //writing to the database
			int tfd=open("transaction.txt", O_RDWR, 0744);
			
			//Generating the Date And Time of operation
			char cur_time[1024];
			memset(cur_time, 0, sizeof(cur_time));				
  		  	time_t t;					
		  	struct tm* ptm;					
		   	time(&t);		
		  	ptm = localtime(&t);
			strftime(cur_time, 1024, "%d-%b-%Y %H:%M:%S", ptm);
			
			struct trans trans1;	//Adding the Record to the Transaction Database 						
			lseek(tfd, 0, SEEK_END);							
			strcpy(trans1.date, cur_time);		
			strcpy(trans1.acc_num, acc_num);
			trans1.amt_credit=amt;
			trans1.amt_debit=0;
			trans1.bal_remaining=temp.bal;
			write(tfd, &trans1, sizeof(trans1));
			close(tfd);
			found=1;
			break;
		}	
	} 
	write(nfd, &found, sizeof(found));
	if(found==1){
		write(nfd, &temp.bal, sizeof(temp.bal));
	}
	close(fd);
	
}

void single_withdraw_money(char acc_num[1024], int nfd)				
{							
	double amt;								
						
	read(nfd, &amt, sizeof(amt));
	
	int found=0;
	int fd=open("account.txt", O_RDWR, 0744);
	struct acc temp;
	while(read(fd, &temp, sizeof(temp)) != 0)
	{
		if(strcmp(temp.acc_num, acc_num)==0)
		{
			if(temp.bal < amt){ //To check whether money to be withdrawn is less then available balance
				found=2;
				break;
			}
			temp.bal=temp.bal - amt; //Performing Subtraction
			lseek(fd, -sizeof(temp), SEEK_CUR);
			write(fd, &temp, sizeof(temp));//writing to the database
			
			int tfd=open("transaction.txt", O_RDWR, 0744);
			
			//Generating the Date And Time of operation
			char cur_time[1024];
			memset(cur_time, 0, sizeof(cur_time));		
  		  	time_t t;			
		  	struct tm* ptm;			
		    	t = time(NULL);			
		  	ptm = localtime(&t);					
			strftime(cur_time, 1024, "%d-%b-%Y %H:%M:%S", ptm);				
										
			struct trans trans1; //Adding the Record to the Transaction Database 
			
			lseek(tfd, 0, SEEK_END);							
			strcpy(trans1.date, cur_time);		
			strcpy(trans1.acc_num, acc_num);
			trans1.amt_credit=0;
			trans1.amt_debit=amt;
			trans1.bal_remaining=temp.bal;
			write(tfd, &trans1, sizeof(trans1));
			close(tfd);
			found=1;
			break;
		}		
	}
	write(nfd, &found, sizeof(found));
	if(found==1){
		write(nfd, &temp.bal, sizeof(temp.bal));
	}
	close(fd);						
}	
	

void print_trans(char acc_num[1024], int type, int nfd)
{
	
	int fd=open("transaction.txt", O_RDWR, 0744);
	
	struct trans temp;	
	if(type==1){
		int found=0, unlock=0, done=0;
			
		while( (done=read(fd, &temp, sizeof(temp))) != 0){ //not done yet
								
			if( strcmp(temp.acc_num, acc_num)==0 ){
				
				write(nfd, &done, sizeof(done));
				done=0;
				
				lseek(fd, -sizeof(temp), SEEK_CUR);						 
				struct flock lock;
				lock.l_type = F_RDLCK;
				lock.l_start = 0; 
				lock.l_whence = SEEK_CUR;
    				lock.l_len = sizeof(temp);
    				lock.l_pid = getpid();	
    				
    				
    				fcntl(fd, F_SETLKW, &lock);
    				
    				write(nfd, &temp, sizeof(temp));
    				
    				
					read(nfd, &unlock, sizeof(unlock));
					
					if(unlock==1){
						//write(1, "sdasd",sizeof("sdasd") );
						lock.l_type = F_UNLCK;
    						fcntl(fd,F_SETLK,&lock);
    						unlock=0;
    					}
    				lseek(fd, sizeof(temp), SEEK_CUR);
    				fflush(stdout);
    				//write(nfd, &temp, sizeof(temp));
			}
		}
		write(nfd, &done, sizeof(done));
		
	}else{
		int done=0;
		while( (done=read(fd, &temp, sizeof(temp))) != 0){					
			if( strcmp(temp.acc_num, acc_num)==0 ){
				write(nfd, &done, sizeof(done));
				write(nfd, &temp, sizeof(temp));
				done=0;
				
			}
		} 
		write(nfd, &done, sizeof(done));
		
	}
	close(fd);
}

void print_acc(char acc_num[1024], int type, int nfd)				
{								
	int fd=open("account.txt", O_RDWR, 0744);
	int found=0, unlock=0;
	struct acc temp;
	if(type==1){ //Printing Joint Account Balance Details
		while(read(fd, &temp, sizeof(temp)) != 0){				
			if(strcmp(temp.acc_num, acc_num) ==0){
				found=1;
				break;					
			}			
		}
		write(nfd, &found, sizeof(found));
		if(found==1){
			//Locking The Joint Account Record
			lseek(fd, -sizeof(temp), SEEK_CUR); // Moving one Record Back				
			struct flock lock;			
			lock.l_type = F_RDLCK;				
			lock.l_start = 0;  // current plus 0 bytes			
			lock.l_whence = SEEK_CUR;				
    			lock.l_len = sizeof(temp);			
    			lock.l_pid = getpid();		
    			
    			fcntl(fd, F_SETLKW, &lock);				
    			write(nfd, &temp, sizeof(temp)); //Sending the Data to the client to print
    			
    			while(1){//Waiting Till Client Sends Unlock Signal
				read(nfd, &unlock, sizeof(unlock));
				if(unlock==1){
					lock.l_type = F_UNLCK;
    					fcntl(fd,F_SETLK,&lock);
    					break;
    				}
			}	
		}
		
	
	}else{////Printing Single Account Balance Details
		int done=0;
		while( (done=read(fd, &temp, sizeof(temp))) != 0)
		{
			if(strcmp(temp.acc_num, acc_num) ==0)
			{
				write(nfd, &done, sizeof(done));
				write(nfd, &temp, sizeof(temp));
				done=0;
			}	
		}
		write(nfd, &done, sizeof(done)); 
	}
	close(fd);	
}


////****************************FOR JOINT ACCOUNT***********************************
void joint_deposit_money(char acc_num[1024], int nfd)
{
	double amt;
	
	read(nfd, &amt, sizeof(amt));	
	
	int fd=open("account.txt", O_RDWR, 0744);
	struct acc temp;
	int found=0, unlock=0;
	while(read(fd, &temp, sizeof(temp)) != 0){
		if(strcmp(temp.acc_num, acc_num)==0){
			found=1;//seraching through the database untill the record is not found to get the account number
			break;
		}	
	} 
	
	write(nfd, &found, sizeof(found));
	if(found==1){
		
			lseek(fd, -sizeof(temp), SEEK_CUR); 
			struct flock lock;
			lock.l_type = F_WRLCK;
			lock.l_start = 0;  // current plus 0 bytes
			lock.l_whence = SEEK_CUR;
    			lock.l_len = sizeof(temp);
    			lock.l_pid = getpid();
    			
    			
    			fcntl(fd, F_SETLKW, &lock); //Entering into Critical Section
			
			temp.bal=temp.bal + amt; 
			write(fd, &temp, sizeof(temp));
			write(nfd, &temp.bal, sizeof(temp.bal));
			
			write(nfd ,"\nEnter 1 to unlock: ", sizeof("\nEnter 1 to unlock: "));
			while(1){//Waiting Till Client Sends Unlock Signal
				read(nfd, &unlock, sizeof(unlock));
				if(unlock==1){
					lock.l_type = F_UNLCK;
    					fcntl(fd,F_SETLK,&lock);
    					break;
    					}
			}
			
			int tfd=open("transaction.txt", O_RDWR, 0744);
			
			//Generating the Date And Time of operation
			char cur_time[1024];
			memset(cur_time, 0, sizeof(cur_time));
  		  	time_t t;
		  	struct tm* ptm;
		   	t = time(NULL);
		  	ptm = localtime(&t);
			strftime(cur_time, 1024, "%d-%b-%Y %H:%M:%S", ptm);

			struct trans trans1; //Entering the Record into the Transaction database
			
			lseek(tfd, 0, SEEK_END);							
			strcpy(trans1.date, cur_time);		
			strcpy(trans1.acc_num, acc_num);
			trans1.amt_credit=amt;
			trans1.amt_debit=0;
			trans1.bal_remaining=temp.bal;
			write(tfd, &trans1, sizeof(trans1));
			close(tfd);
	}
	close(fd);
		

} 


void joint_withdraw_money(char acc_num[1024] , int nfd)						
{							
	double amt;							
										
	read(nfd, &amt, sizeof(amt));							
	int fd=open("account.txt", O_RDWR, 0744);
	int found=0, unlock=0, correct=0;
	struct acc temp;
	
	while(read(fd, &temp, sizeof(temp)) != 0){								
								
		if(strcmp(temp.acc_num, acc_num)==0){		
			found=1;
			break;
		}
				
	}
	write(nfd, &found, sizeof(found));
	if(found==1){
	
			if(temp.bal< amt){
				correct=1;
				return;
			}
			write(nfd, &correct, sizeof(correct));
			
			lseek(fd, -sizeof(temp), SEEK_CUR); 
			struct flock lock;
			lock.l_type = F_WRLCK;
			lock.l_start = 0;  
			lock.l_whence = SEEK_CUR;
    			lock.l_len = sizeof(temp);
    			lock.l_pid = getpid();
		
			fcntl(fd, F_SETLKW, &lock);
			temp.bal=temp.bal-amt;
			write(fd, &temp, sizeof(temp));
			write(nfd, &temp.bal, sizeof(temp.bal));
			
			write(nfd ,"\nEnter 1 to unlock", sizeof("\nEnter 1 to unlock"));
			while(1){
				read(nfd, &unlock, sizeof(unlock));
				if(unlock==1){
					lock.l_type = F_UNLCK;
    					fcntl(fd,F_SETLK,&lock);
    					break;
    					}
			}
			
			int tfd=open("transaction.txt", O_RDWR, 0744);
		
			char cur_time[1024];
			memset(cur_time, 0, sizeof(cur_time));
			
  		  	time_t t;
		  	struct tm* ptm;
		    	t = time(NULL);
		  	ptm = localtime(&t);
			strftime(cur_time, 1024, "%d-%b-%Y %H:%M:%S", ptm);
		
			struct trans trans1;
			
			lseek(tfd, 0, SEEK_END);
								
			strcpy(trans1.date, cur_time);		
			strcpy(trans1.acc_num, acc_num);
			trans1.amt_credit=0;
			trans1.amt_debit=amt;
			trans1.bal_remaining=temp.bal;
			write(tfd, &trans1, sizeof(trans1));
			close(tfd);
			
	}
	close(fd);
} 


//for admin
void pass_change(char usr_name[1024], int nfd)						
{						
							
	int fd=open("customer.txt", O_RDWR, 0744);
	int found=0;						
	struct cust check;							
	while(read(fd, &check, sizeof(check)) != 0){
															
		if( strcmp(check.usr_name, usr_name)==0 ){					
			
			found=1;
			break;
		}
	}
	if(found==1){
		char pass1[1024], pass2[1024];
		int done=0;
		write(nfd, &found, sizeof(found));
		
		read(nfd, &pass1, sizeof(pass1));
		read(nfd, &pass2, sizeof(pass2));
		
		if( strcmp(pass1, pass2) == 0) {
				strcpy(check.pass, pass1);		
				lseek(fd, -sizeof(check), SEEK_CUR);			
				write(fd, &check, sizeof(check));		
				//printf("\n %s, Password changed successfully!!", usr_name);
				done=1;				
		}
		write(nfd, &done, sizeof(done));
		
		if(done!=1){
			return;
		}			
	}
	close(fd);
	admin_print_customers(nfd);
} 
	
void admin_search(int nfd)
{	
	int choice;
	
	read(nfd, &choice, sizeof(choice));
	
	if(choice==1) // print customer + account details
	{
		char usr_name[1024];							
		int found=0; 				
		char acc_num[1024];
		read(nfd, &usr_name ,sizeof(usr_name));						
		
		int fd=open("customer.txt", O_RDWR, 0744);						
		
		struct cust temp;
		while(read(fd, &temp, sizeof(temp)) != 0){								
			if(strcmp(temp.usr_name, usr_name)==0 && temp.type!=5){ //finding the record				
				found=1;				
				break;			
			}			
		} 					
		close(fd);
		write(nfd, &found, sizeof(found));
		if(found==1){					
			write(nfd, &temp, sizeof(temp));
			strcpy(acc_num, temp.acc_num);					
		}else{
			close(nfd);
			exit(1);
		}				
					
		struct acc temp1;			
		fd=open("account.txt", O_RDWR, 0744);			
		found=0;
		int done=0;					
		while(read(fd, &temp1, sizeof(temp1)) != 0){									
			if( strcmp(temp1.acc_num, acc_num) ==0){					
				//printf("\t%f\n", temp1.bal);
				found=2;
				break;
			}					
		}
		
		write(nfd, &found, sizeof(found));										
		if(found==2){					
			write(nfd, &temp1, sizeof(temp1));
								
		}
		close(fd);	
	}					
	else if(choice==2) {									
		char account_number[1024];				
							
		read(nfd, &account_number ,sizeof(account_number));
		
		struct acc temp2;						
		int fd=open("account.txt", O_RDWR, 0744);					
		int found=0;
		while(read(fd, &temp2, sizeof(temp2)) != 0){							
			if( strcmp(temp2.acc_num, account_number) ==0){					
				//printf("\n%s\t\t%f\n", temp2.acc_num, temp2.bal);
				found=3;
				break;
			}							
		}
		write(nfd, &found, sizeof(found)); 				
		
		if(found==3){					
			write(nfd, &temp2, sizeof(temp2));
								
		}			
		close(fd);				
	}						
					
}		

void admin_modify(int nfd)
{
	char usr_name[1024];
	int choice=0;	
				
	read(nfd, &usr_name, sizeof(usr_name));							
	read(nfd, &choice, sizeof(choice));					
	if(choice==1){					
		int fd=open("customer.txt", O_RDWR, 0744);						
		struct cust temp;					
		int found=0; 
		
		while(read(fd, &temp, sizeof(temp)) != 0){				
			if( strcmp(temp.usr_name, usr_name)==0 && temp.type==2){				
				found=1;
				break;
			}
			else if( strcmp(temp.usr_name, usr_name)==0 && temp.type==1){					
				found=2;
				break;
			}
		} 
		write(nfd, &found, sizeof(found));
		
		if(found==1){
			return;	
		}else if(found==0){
			return;
		}
		if(found==2){
				temp.type=2; 
				lseek(fd, -sizeof(temp), SEEK_CUR);
				write(fd, &temp, sizeof(temp));
		}
		close(fd);
		
		struct cust second_user;
		// extra customer addition for joint account
			
		struct cust temp2;
		while(1){		
			read(nfd, &second_user.usr_name, sizeof(second_user.usr_name));
								
			int found=0; // username is unique . that is not found
			int fd=open("customer.txt", O_RDWR, 0744);
		
			while(read(fd, &temp2, sizeof(temp2)) != 0){
				if(strcmp(temp2.usr_name, second_user.usr_name)==0){
					found=1;
					break;
				}		
			} 
			write(nfd, &found, sizeof(found)); 
			close(fd);
			if(found==0)
				break;
			
			
		}	
						
		read(nfd, &second_user.pass, sizeof(second_user.pass));		
		
		strcpy(second_user.acc_num, temp.acc_num);
		second_user.type=2;
		
		fd=open("customer.txt", O_RDWR, 0744);
		
		found=0;
		//struct customer loop;
		while(read(fd, &temp, sizeof(temp)) != 0){
			if(temp.type==5){
				//printf("deleted account/ duplicate account found and modifying it\n");
				found=1;
				break;
			}			
		} 
		
		if(found==0) 
			write(fd, &second_user, sizeof(second_user));
		else {	
			lseek(fd, -sizeof(second_user), SEEK_CUR);
			write(fd, &second_user, sizeof(second_user));
		}		
		close(fd);
	}else{
		pass_change(usr_name, nfd);
	} 
	
	admin_print_customers(nfd); 
	admin_print_accounts(nfd);	
}	

void admin_delete_username(char username[1024], int nfd)
{
	int fd=open("customer.txt", O_RDWR, 0744);
	int found=0;
	struct cust temp;
	while(read(fd, &temp, sizeof(temp)) != 0){
		//Acc type should be 1 or 2 then only we can delete them
		if( strcmp(temp.usr_name, username)==0 && (temp.type==1 || temp.type==2) ){						
			found=1;				
			break;			
		}						
	} 					
	write(nfd, &found, sizeof(found));
	write(nfd, &temp, sizeof(temp));
								
	if(found==0){
		close(fd);
		return;
			
	}else if(found==1 && temp.type==1){ // record found and normal single user
		temp.type=5; // 5 in type denotes that account is  deleted 
		lseek(fd, -sizeof(temp), SEEK_CUR);			
		write(fd, &temp, sizeof(temp));			
		admin_delete_account(temp.acc_num, nfd); // deleting corresponding account from account.txt
		close(fd);
	} 
	else if(found==1 && temp.type==2) { // Record found and its a joint account 
		
		char joint_acc_num[1024];
		
		strcpy(joint_acc_num, temp.acc_num); 
		temp.type=5;
		
		lseek(fd, -sizeof(temp), SEEK_CUR);
		write(fd, &temp, sizeof(temp));
		close(fd);
		
		//finding if the joint user still in the database or deleted prior to this current user deletion
		int fd1=open("customer.txt", O_RDWR, 0744);
		
		int found1=0; // joint user not found till now
		
		struct cust temp1;			
		while(read(fd1, &temp1, sizeof(temp1)) != 0){						
			//Acc type should be 1 or 2 then only we can delete them					
			if(strcmp(temp1.acc_num, joint_acc_num)==0  && temp1.type==2 ){							
				found1=1;					
				break;					
			}									
		}									
		write(nfd, &found1, sizeof(found1));	
					
		if(found1==1)											
			return;						
		else							
			admin_delete_account(joint_acc_num, nfd);						
										
	}							
				
}			
		

void admin_delete_account(char account_number[], int nfd)					
{								
	int fd=open("account.txt", O_RDWR, 0744);					
	int found=0; 			
						
	struct acc temp;
	//write(1, "\ninside", sizeof("\ninside"));
	while(read(fd, &temp, sizeof(temp)) != 0){
		if(strcmp(temp.acc_num, account_number)==0 ){
			//printf("\nthe record to be deleted is found\n");
			found=1;
			break;
		}		
	} 
	//write(1, "\noutside", sizeof("\noutside"));
	write(nfd, &found, sizeof(found));
	
	if(found==0){
		
		return;
	}
	else{
		write(nfd, &temp.bal, sizeof(temp.bal));
		int choice=1; 
		found=0;
		if(temp.bal>0.0){// reconfirming if the acc to be deleted has some balance
			
			found=1;
			write(nfd, &found, sizeof(found));
			read(nfd, &choice, sizeof(choice));
			
		}else{
			write(nfd, &found, sizeof(found));
			choice=0;
		}
		
		if(choice==0){
			strcpy(temp.acc_num, "0000");    // 4 zeroes in account number signifies deleted account				
			temp.bal=0.0;					
			lseek(fd, -sizeof(temp), SEEK_CUR);				
			write(fd, &temp, sizeof(temp));					
		}
		else{
			close(fd);
			return;
		}			
	}						
	close(fd);			
							
	// finding corresponding username and deleting  them from Customers.txt			
				
	fd=open("customer.txt", O_RDWR, 0744);						
	struct cust temp1;						
	int done=0;					
	while( (done=read(fd, &temp1, sizeof(temp1))) != 0){							
		if( strcmp(account_number, temp1.acc_num)==0 && (temp1.type==1 || temp1.type==2) ){						
			write(nfd, &done, sizeof(done));
			write(nfd, &temp1.usr_name, sizeof(temp1.usr_name));
			admin_delete_username(temp1.usr_name, nfd);					
		}								
	}
	write(nfd, &done, sizeof(done));							
	close(fd);							
}  

void admin_delete(int nfd)				
{				
	int choice, flg=0;		
	
	read(nfd, &choice, sizeof(choice));
	
	
	if(choice==1) // deleting record using account number
	{
		char acc_num[1024];
		read(nfd, &acc_num, sizeof(acc_num));
		admin_delete_account(acc_num, nfd);
	}		
	else if(choice==2) // deleting record using username 
	{
		char usr_name[1024];
		read(nfd, &usr_name, sizeof(usr_name));
		
		admin_delete_username(usr_name, nfd);
	}
		
	admin_print_customers(nfd);
	admin_print_accounts(nfd);
}

void admin_add(int nfd)						
{			
	struct cust new;						
	struct cust temp2;
	while(1){
														
		read(nfd, &new.usr_name, sizeof(new.usr_name));
									
		int found=0; // username is unique . that is not found
		int fd=open("customer.txt", O_RDWR, 0744);
		while(read(fd, &temp2, sizeof(temp2)) != 0){	
			if(temp2.type==5){
				continue;}					
			if(strcmp(temp2.usr_name, new.usr_name)==0 ){								
				found=1;
				break;
			}		
		} 
		write(nfd, &found, sizeof(found));
		close(fd);
		if(found==0)
			break;
	}
						
	read(nfd, &new.pass, sizeof(new.pass));			
	char acc[1024];
	memset(acc, 0, sizeof(acc));
	memset(new.acc_num, 0, sizeof(new.acc_num));								
	strcat(acc, "MAHB-PUN-NP-00351-"); 
	// state-district-branchcode-starting_Sequence-username
	strcat(acc, new.usr_name);
	strcpy(new.acc_num, acc);
	
	read(nfd, &new.type, sizeof(new.type));				
				
	int fd=open("customer.txt", O_RDWR, 0744);	
	// find the first record that is duplicate or deleted. customer_type= 5				
	int found=0; // no duplicate or deleted record as of now						
	struct cust temp;						
	while(read(fd, &temp, sizeof(temp)) != 0){						
		if(temp.type==5){														
			//printf("deleted account/ duplicate account found and modifying it\n");				
			found=1;	
			break;			
		}							
	} 				
						
	if(found==0) // if no deleted rec found  				
		write(fd, &new, sizeof(new));					
	else {						
		lseek(fd, -sizeof(temp), SEEK_CUR);					
		write(fd, &new, sizeof(new));						
	}		
	close(fd);
	
	
	struct acc new_acc; ///enter ing the record in account.txt
	struct acc temp1;
	memset(new_acc.acc_num, 0, sizeof(new_acc.acc_num));
	strcpy(new_acc.acc_num, new.acc_num);
	new_acc.bal=0.0;
	
	fd=open("account.txt", O_RDWR, 0744);
	
	found=0; 				
	while(read(fd, &temp1, sizeof(temp1)) != 0){				
		if(strcmp(temp1.acc_num,"0000")==0 )// 4 zeroes in account_number signifies that deleted account				
		{			
			//printf("deleted account/ duplicate account found and modifying it\n");
			found=1;				
			break;			
		}									
	} 							
								
	if(found==0) // if no deleted record is found				
		write(fd, &new_acc, sizeof(new_acc));
	else {
		lseek(fd, -sizeof(temp1), SEEK_CUR);
		write(fd, &new_acc, sizeof(new_acc));
	}		
	close(fd);
	
	if(new.type==2) // enter joint account details
	{
		struct cust second_user;
		// extra customer addition for joint account
		
		struct cust temp2;
		while(1){										
							
			read(nfd, &second_user.usr_name, sizeof(second_user.usr_name));
									
			int found=0; // username is unique . that is not found
			int fd=open("customer.txt", O_RDWR, 0744);
		
			while(read(fd, &temp2, sizeof(temp2)) != 0){
				if(temp2.type==5){
					continue;}
				if(strcmp(temp2.usr_name, second_user.usr_name)==0){
					//printf("\nUser Name Already Exists. Please, Enter a Unique Username ");
					found=1;
					break;
				}		
			} 
			write(nfd, &found, sizeof(found));
			
			close(fd);
			if(found==0)
				break;
		}		
		read(nfd, &second_user.pass, sizeof(second_user.pass));
		
		strcpy(second_user.acc_num, new.acc_num);
		second_user.type=2;
		
		fd=open("customer.txt", O_RDWR, 0744);
		
		found=0;
		//struct customer loop;
		while(read(fd, &temp, sizeof(temp)) != 0){
			if(temp.type==5){
				//printf("deleted account/ duplicate account found and modifying it\n");
				found=1;
				break;
			}			
		} 
		
		if(found==0) 
			write(fd, &second_user, sizeof(second_user));
		else {	
			lseek(fd, -sizeof(second_user), SEEK_CUR);
			write(fd, &second_user, sizeof(second_user));
		}		
		close(fd);
		
		// account details already present in acc.txt so no need to add again. 
	}					
						
	admin_print_customers(nfd);				
	admin_print_accounts(nfd);				
} 	
	
void admin_print_customers(int nfd)
{		
	
	int fd=open("customer.txt", O_RDWR, 0744);				
	int done=0;						
	struct cust print;
	while( (done=read(fd, &print, sizeof(print))) != 0){

		if(print.type!=5 && print.type!=0){
			write(nfd, &done, sizeof(done));
			write(nfd, &print, sizeof(print));
			done=0;
		}
	 }			
	 write(nfd, &done, sizeof(done));			
	close(fd);
}

void admin_print_accounts(int nfd)
{		
	int fd=open("account.txt", O_RDWR, 0744);
	int done=0;
	struct acc print1;
	
	read(fd, &print1, sizeof(print1));
	while( (done=read(fd, &print1, sizeof(print1))) != 0){
		if(strcmp(print1.acc_num, "0000") !=0 ){
			write(nfd, &done, sizeof(done));
			write(nfd, &print1, sizeof(print1));
			done=0;
		}		
	} 
	write(nfd, &done, sizeof(done));			
	close(fd);	
} 


	
	
