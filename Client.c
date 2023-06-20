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


//Client Side
int main(){

	char usr_name[1024], pass[1024];
	int login_check,type;
	memset(usr_name, 0, sizeof(usr_name));// to clear the array
	memset(pass, 0, sizeof(pass));

	//For Socket
	struct sockaddr_in client;
	int fd, msg_length;
	char buff[60];
	char result;
	
	if( (fd=socket(AF_INET, SOCK_STREAM, 0)) ==-1){
		perror("\nERROR1: ");
		exit(1);
	}
	client.sin_family= AF_INET; //address family 
	client.sin_addr.s_addr=inet_addr("127.0.0.1"); //inet_addr("127.0.0.1"); //Internet Addres
	client.sin_port= htons(5555); //port number 
	
	write(1, "\nWaiting to Connect to the Server....", sizeof("\nWaiting to Connect to the Server...."));
	if( connect(fd, (struct sockaddr *)&client, sizeof(client))==-1 ){
		perror("\nERROR2: ");
		exit(1);
	}				
	read(fd, buff, sizeof(buff));
	printf("%s", buff);
	
	//Bank
	printf("\n***********************Welcome to BANK OF MAHARASHTRA***************************");
	printf("\nEnter your username and password to login to your account:\nUsername: ");
	scanf("%s", usr_name);
						
	printf("\nPassword: ");							
	scanf("%s", pass);	
	
	write(fd, usr_name, sizeof(usr_name));
	write(fd, pass, sizeof(pass));
	
	read(fd, &login_check, sizeof(login_check));
	if(login_check==0) 
	// if entered usr_name and pass does not  match
	{
		printf("\nIncorrect UserName/Passowrd please try again");
		close(fd);	
		exit(1);
	}
	//if username and pass correct then main code is started
	read(fd, &type, sizeof(type));
	
	if(type == 0) // For admin
	{	
		while(1){//Putting inside a while loop so it can perform operations multiple time. 
		printf("\n******************** Welcome ADMIN ********************\nSelect The options");
		printf("\n1 to Add an account");
		printf("\n2 to Delete either through Username or through account");
		printf("\n3 to Modify an account"); 
		printf("\n4 to Search for an account");
		printf("\n5 Print All the Customers Present and Their Balance");
		printf("\n6 to Exit\nEnter Your Choice: ");
		getchar();
		
		int choice;
		scanf("%d", &choice);
		
		write(fd, &choice, sizeof(choice));
		if(choice==1) 
			admin_add(fd);
		else if(choice==2) 
			admin_delete(fd);
		else if(choice==3)
			admin_modify(fd);
		else if(choice==4)
			admin_search(fd);
		else if(choice==5){
			admin_print_customers(fd);
			admin_print_accounts(fd);
			}
		else if(choice==6){
			printf("\nLogged Out Successfully!!");						
			close(fd);
			exit(1);					
		}
		}			
	}else if(type==1) // Single acc user
	{
		while(1){
		printf("\n****************** Welcome User *******************");					
		printf("\n1 to Deposit Money into account");					
		printf("\n2 to Withdraw money from account");					
		printf("\n3 To Check  Balance ");					
		printf("\n4 to Change Password ");						
		printf("\n5 to View Transactions details");					
		printf("\n6 to Exit\nEnter Your Choice: ");				
		
		struct cust check;
		
		int choice;
		scanf("%d", &choice);
		write(fd, &choice, sizeof(choice));
		read(fd, &check, sizeof(check));
					
		if(choice==1)						
			single_deposit_money(check.acc_num, fd); //sending account_number and then askinng other details 
		else if(choice==2)					
			single_withdraw_money(check.acc_num, fd); 					
		else if(choice==3)					
			print_acc(check.acc_num, 0, fd);									
		else if(choice==4)						
			pass_change(check.usr_name, fd);					
		else if(choice==5)						
			print_trans(check.acc_num, 0, fd); // print transactions details
		else if(choice==6){
			printf("\nLogged Out Successfully!!");						
			close(fd);
			exit(1);					
		}
	     }
	}
	else if(type==2) // for Joint Account Holder
	{
		while(1){
		printf("\n************** Welcome Joint Account User *******************");				
		printf("\n1 to Deposit Money into account");					
		printf("\n2 to Withdraw money from account");					
		printf("\n3 To Check  Balance ");					
		printf("\n4 to Change Password ");						
		printf("\n5 to View Transactions details");					
		printf("\n6 to Exit. Enter Your Choice: ");				
		
		struct cust check;
		int choice;
		scanf("%d", &choice);
		fflush(stdin);
		
		write(fd, &choice, sizeof(choice));
		read(fd, &check, sizeof(check));
		
		if(choice==1)
			joint_deposit_money(check.acc_num, fd);  
		else if(choice==2)
			joint_withdraw_money(check.acc_num, fd); 
		else if(choice==3)
			print_acc(check.acc_num, 1, fd);
		else if(choice==4)
			pass_change(check.usr_name, fd);
		else if(choice==5)
			print_trans(check.acc_num, 1, fd); // read lock
		else if(choice==6){
			printf("\nLogged Out Successfully!!");						
			close(fd);
			exit(1);					
			}
	
		}
	}

	close(fd);	
	return 0;
}

//For single User
void single_deposit_money(char acc_num[1024], int fd)					
{					
	double amt;			
							
	printf("\nEnter the amount you want to deposit: ");					
	scanf("%lf", &amt);						
	fflush(stdin);
	
	write(fd, &amt, sizeof(amt));
	int found;
	read(fd, &found, sizeof(found));
	
	if(found==1){
		double bal;
		read(fd, &bal, sizeof(bal)); //reading the current balance of user after making changes in the database. 
		printf("\n Rs %lf has successfully credited into account and Current balance: %lf ", amt, bal);	
	}
	
	
}

void single_withdraw_money(char acc_num[1024], int nfd)				
{							
	double amt;								
	
	printf("\nEnter the amount to withdraw: ");					
	scanf("%lf", &amt);						
	
	write(nfd, &amt, sizeof(amt));
	int found;
	read(nfd, &found, sizeof(found));
	
	if(found==1){
		double bal;
		read(nfd, &bal, sizeof(bal));
			printf("\n Rs %lf has successfully debited from account and Current balance: %lf ", amt, bal);	
		
	}else if(found==2){
		printf("\nInsufficient Balance...Please Try again!!");
		return;
	}	
						
}	

void print_trans(char acc_num[1024], int type, int nfd)
{
	write(1, "\n************* Transaction **************", sizeof("\n************* Transaction **************"));
	write(1, "\nDate\t     Account_number\t      Credit\t   Debit\t   Total_balance", sizeof("\nDate\t     Account_number\t      Credit\t   Debit\t   Total_balance"));
	
	struct trans temp;	
	if(type==1){			
		int unlock=0, r, temp1=0;
		int done;
		char buff[100];
		read(nfd, &done, sizeof(done));
		while(1){
			if(done!=0){
				
				printf("\nLocking the Joint record for Print ");
				
				read(nfd, &temp, sizeof(temp));
				
				printf("\n%s\t %s\t %lf\t %lf\t %lf", temp.date, temp.acc_num, temp.amt_credit, temp.amt_debit, temp.bal_remaining);
				fflush(stdout);
				printf("\nEnter 1 to unlock: ");
				scanf("%d", &unlock);
				fflush(stdin);
				
				write(nfd, &unlock, sizeof(unlock));
				
				read(nfd, &done, sizeof(done));
				
				/*read(nfd, &temp, sizeof(temp));
				
				printf("\n%s\t %s\t %lf\t %lf\t %lf", temp.date, temp.acc_num, temp.amt_credit, temp.amt_debit, temp.bal_remaining);
				read(nfd, &done, sizeof(done));
				*/
			}
			else{
				break;
			}
			
		}	
			
	}else{
		int done;
		read(nfd, &done, sizeof(done));
		while(1){
			if(done!=0){
				read(nfd, &temp, sizeof(temp));
				 printf("\n%s\t\t %s\t %lf\t %lf\t %lf\n", temp.date, temp.acc_num, temp.amt_credit, temp.amt_debit, temp.bal_remaining);
				read(nfd, &done, sizeof(done));
			}else{
				break;
			}
		}
	}
	
}


void print_acc(char acc_num[1024], int type, int nfd)				
{						
	printf("\n**************** Printing Account Details ***************\nAccount_number\t\tBalance");	
	fflush(stdin);
	fflush(stdout);
	
	struct acc temp;
	if(type==1){//Printing Joint Account Balance Details
		int found=0,r, unlock=0;
		read(nfd, &found, sizeof(found));
		if(found==1){
			//write(1, , sizeof("\nLocking the Joint record "));
			printf("\nLocking the Joint record ");
			read(nfd, &temp, sizeof(temp)); 
			//if the Record is locked then other User cannot write it unless the record gets unlocked 
			printf("\n%s\t %f", temp.acc_num, temp.bal); //printing the data
			
			printf("\nEnter 1 to unlock: ");
			scanf("%d", &unlock);
			fflush(stdin);
			
			write(nfd, &unlock, sizeof(unlock));//Sending the unlock the signal to the server
		}
		
	}else{///Printing Single Account Balance Details
		int done;
		read(nfd, &done, sizeof(done));
		while(1){
			if(done!=0){
				read(nfd, &temp, sizeof(temp));
				printf("\n%s\tyy %f ", temp.acc_num, temp.bal);
				read(nfd, &done, sizeof(done));
			}else{
				break;
			}
		}
	}
		
}


//******************************* FOR JOINT Account Holder ****************************
void joint_deposit_money(char acc_num[1024], int nfd)
{
	char buff[100];
	double amt;
	int found=0, r, unlock=1;
	struct acc temp;
	
	printf("\nEnter the amount to deposit: ");
	scanf("%lf", &amt);
	fflush(stdin);
	
	write(nfd, &amt, sizeof(amt));
	read(nfd, &found, sizeof(found));
	
	if(found==1){
		write(1, "\nLocking the Joint record for deposit", sizeof("\nLocking the Joint record for deposit"));
		
		read(nfd, &temp.bal, sizeof(temp.bal)); //Reading the Curent Balance.
	
		r=read(nfd, &buff, sizeof(buff));
		write(1, &buff, r); 
		
		scanf("%d", &unlock);
		fflush(stdin);
		write(nfd, &unlock, sizeof(unlock));
		
		printf("\nRs %lf has successfully Credited into the account and Current balance: %lf  ", amt, temp.bal);
	}
} 
 
void joint_withdraw_money(char acc_num[1024], int nfd)						
{							
	double amt;							
	int found=0, unlock=0, correct=0, r;
	char buff[100];									
	printf("\nEnter the amount to withdraw from the account: ");					
	scanf("%lf", &amt);						
	fflush(stdin);
	
	write(nfd, &amt, sizeof(amt));
	read(nfd, &found, sizeof(found));	
				
	struct acc temp;
	
	if(found==1){
		read(nfd, &correct, sizeof(correct));
		if(correct==1){
			printf("\nEnter Valid Amount...Please Try Again!!");
			return;
		}
	
		write(1, "\nLocking the Joint record for deposit", sizeof("\nLocking the Joint record for deposit"));
		read(nfd, &temp.bal, sizeof(temp.bal));
		
		r=read(nfd, &buff, sizeof(buff));
		write(1, &buff, r); //on to the terminal ---the unlock statement
		
		scanf("%d", &unlock);
		fflush(stdin);
		write(nfd, &unlock, sizeof(unlock));
		
		printf("\nRs %lf has successfully debited from the account and Current balance: %lf", amt, temp.bal);
	}

} 






//********************************** For ADMIN *************************** 
void pass_change(char usr_name[1024], int nfd)						
{						
	char pass1[1024], pass2[1024];						
	struct cust check;
	int found, done;
	read(nfd, &found, sizeof(found));
	
	if(found==1){
			memset(pass1, 0, sizeof(pass1)); //clearing the array before storing the password
			printf("\nEnter the new password: ");			
			scanf("%s", pass1);
			memset(pass2, 0, sizeof(pass2));//clearing the array before storing the password				
			printf("\nRe-enter the new password: ");			
			scanf("%s", pass2);
			
			write(nfd, &pass1, sizeof(pass1));
			write(nfd, &pass2, sizeof(pass2));
			
			read(nfd, &done, sizeof(done));
			if(done==1){
				printf("\n %s, Password changed successfully!!", usr_name);			
				
			}else{
				printf("\nPasswords entered does not match..Please Try again!!");			
				return;
			}
			
	}		
	admin_print_customers(nfd);
} 

void admin_add(int nfd)						
{			
	struct cust new;						
	printf("\nEnter details to add the customer into the bank record: ");	
	struct cust temp2;
	while(1){
		memset(new.usr_name, 0, sizeof(new.usr_name));							
		printf("\nEnter the username: ");  						
		scanf("%s", new.usr_name);					
		write(nfd, &new.usr_name, strlen(new.usr_name)+1);
						
		int found=0; // username is unique . that is not found
		read(nfd, &found, sizeof(found));
		if(found==1){
			printf("\nUser Name Already Exists. Please, Enter a Unique Username ");
		}
		if(found==0)
			break;
	}
	
	memset(new.pass, 0, sizeof(new.pass));					
	printf("\nEnter password : ");					
	scanf("%s", new.pass);				
	write(nfd, &new.pass, sizeof(new.pass));
					
	
	printf("\n1.single account user\n2.joint accoutn user\nEnter customer type: ");				
	scanf("%d", &new.type);				
				
	write(nfd, &new.type, sizeof(new.type));
	
	if(new.type==2) // enter joint account details
	{
		struct cust second_user;
		// extra customer addition for joint account
		printf("\nEnter details of the second user of the joint account");	
		struct cust temp2;
		while(1){
			memset(second_user.usr_name, 0, sizeof(second_user.usr_name));				
			printf("\nEnter the username: ");  						
			scanf("%s", second_user.usr_name);					
			write(nfd, &second_user.usr_name, sizeof(second_user.usr_name));
									
			int found=0; // username is unique . that is not found
			read(nfd, &found, sizeof(found));
			if(found==1){
				printf("\nUser Name Already Exists. Please, Enter a Unique Username ");
			}
			if(found==0)
				break;
		}	
		memset(second_user.pass, 0, sizeof(second_user.pass));				
		printf("\nEnter password : ");					
		scanf("%s", second_user.pass);		
		write(nfd, &second_user.pass, sizeof(second_user.pass));
		// account details already present in acc.txt so no need to add again. 
	}					
						
	admin_print_customers(nfd);				
	admin_print_accounts(nfd);				
} 				

void admin_delete_account(char account_number[], int nfd)					
{													
	write(1, "\n********** DELETE ACCOUNT NUMBER method **********", sizeof("\n********** DELETE ACCOUNT NUMBER method **********"));
	
	int found=0; 			
	struct acc temp;
	read(nfd, &found, sizeof(found));
	//write(1, "\ninside", sizeof("\ninside"));
	if(found==0){
		printf("\nNo Record exists in the database with the given Account Number ");
		return;
	}
	else{
		//write(1, "\ninside found", sizeof("\ninside found"));
		found=0;
		int choice=1;
		
		read(nfd, &temp.bal, sizeof(temp.bal));
		//printf("\n1 %d", found);
		read(nfd, &found, sizeof(found));
		
		if(found==1){
			printf("\ncurrent balance: %f. Enter 1 if you dont want to delete the account or enter 0: ", temp.bal);
			scanf("%d", &choice);
			
			write(nfd, &choice, sizeof(choice));
			
			if(choice==0){
				printf("\nAccount Deleted Successfully");
			}
			if(choice==1){
				return;
			}
		}
				
	}						
									
	// finding corresponding username and deleting  them from Customers.txt								
		struct cust temp1;						
		int done;
		read(nfd, &done, sizeof(done));
		while(1){
			if(done!=0){
				read(nfd, &temp1.usr_name, sizeof(temp1.usr_name));
				admin_delete_username(temp1.usr_name, nfd);
				read(nfd, &done, sizeof(done));
			}else{
				break;
			}
		}													
}  					
			

void admin_delete_username(char username[1024], int nfd)
{
	int found=0;
	struct cust temp;
	
	read(nfd, &found, sizeof(found));
				
	read(nfd, &temp, sizeof(temp));
								
	if(found==0){								
		printf("\nNo user exists with given username ");				
		return;
	}else if(found==1 && temp.type==1){ // record found and normal single user			
		admin_delete_account(temp.acc_num, nfd); // deleting corresponding account from account.txt
	} 
	else if(found==1 && temp.type==2) { // Record found and its a joint account 
		
		int found1=0; // joint user not found till now
		read(nfd, &found1, sizeof(found1));
						
		if(found1==1)											
			printf("\nThe Second Owner of the Joint Account is present.Not Deleting the Account Record From database");	
		else							
			admin_delete_account(temp.acc_num, nfd);						
										
	}							
				
}			
		
void admin_delete(int nfd)				
{				
	int choice, flg=0;		
										
	printf("\n1 To Delete using account number\n2 to delete using username\nEnter your choice: ");				
	scanf("%d", &choice);
	
	write(nfd, &choice, sizeof(choice));
		
	if(choice==1) // deleting record using account number
	{
		char acc_num[1024];
		memset(acc_num, 0, sizeof(acc_num));
		printf("\nEnter the account number to be deleted: ");
		scanf("%s", acc_num);

		write(nfd, &acc_num, sizeof(acc_num));
		admin_delete_account(acc_num, nfd);
	}		
	else if(choice==2) // deleting record using username 
	{
		printf("\nEnter the username to delete: ");
		char usr_name[1024];
		memset(usr_name, 0, sizeof(usr_name));
		
		scanf("%s", usr_name);
		
		write(nfd, &usr_name, sizeof(usr_name));
		admin_delete_username(usr_name, nfd);
	}
		
	admin_print_customers(nfd);
	admin_print_accounts(nfd);
}	
	
void admin_search( int nfd)
{	
	int choice;
	
	printf("\n1. To Search for Username + Account Details\n2. To Just Search for Account Details\nEnter Your Choice: ");
	scanf("%d", &choice);
	write(nfd, &choice, sizeof(choice));
	
	if(choice==1) //It Will find the account nd print details 
	{
		char usr_name[1024];						
						
		int found=0; // no record found				
		memset(usr_name, 0, sizeof(usr_name));							
		printf("\nEnter the Username of the Account to search: ");					
		scanf("%s", usr_name);							
		
		write(nfd, &usr_name ,sizeof(usr_name));				
		printf("\n************* Customer & Account details *************");
		printf("\nUser_name\tPass\t\tBank_Acc\t\tType\tBalance");			
		
		char acc_num[1024];									
						
		struct cust temp;
	 					
						
		read(nfd, &found, sizeof(found));	
		if(found==1){
			read(nfd, &temp, sizeof(temp));					
			printf("\n%s\t\t%s\t%s\t%d\t",temp.usr_name, temp.pass, temp.acc_num, temp.type);				
			strcpy(acc_num, temp.acc_num);					
		}				
		else{
			printf("\nNo Record exists With the given Username ");				
			return;
		}
				
		struct acc temp1;
					
		read(nfd, &found, sizeof(found));
		
		if(found==2){
			read(nfd, &temp1, sizeof(temp1));					
			printf("\t%f\n", temp1.bal);				
								
		}
														
						
	}					
	else if(choice==2) {									
		char account_number[1024];				
		memset(account_number, 0, sizeof(account_number));					
		printf("\nEnter the Account Number to Search: ");			
		scanf("%s", account_number);						
		
		write(nfd, &account_number ,sizeof(account_number));
							
		printf("\n************ Account Details *************");					
		printf("\nBank_Acc\t\t\tBalance");
		
		struct acc temp2;									
		int found=0;
		
		read(nfd, &found, sizeof(found));
		
		if(found==3){
			read(nfd, &temp2, sizeof(temp2));					
			printf("\n%s\t\t%f\n", temp2.acc_num, temp2.bal);			
								
		}
		else{
			printf("\nNo Record exists With the given Account");				
			return;
		}				
						
	}						
					
}				

void admin_modify(int nfd)
{
	char usr_name[1024];
	memset(usr_name, 0, sizeof(usr_name));
	printf("\nEnter the Username of the Account to Make changed: ");				
	scanf("%s", usr_name);	
	write(nfd, &usr_name, sizeof(usr_name));							
							
	int choice=0;				
	printf("\n1.To modify the single user to joint account\n2.Change Password\nEnter your Choice: ");					
	scanf("%d", &choice);												
	write(nfd, &choice, sizeof(choice));	
						
	if(choice==1){			
		struct cust temp;					
		
		int found=0; 
		read(nfd, &found, sizeof(found));
		
		if(found==1){
			printf("\nAccount is already a joint Account");
			return;	
		}else if(found==0){
			printf("\nNo record exist with the given Username\n");
			return;
		}
		
		struct cust second_user;
		
		printf("\nEnter details of the second user of the joint account");	
		struct cust temp2;
		while(1){
			memset(second_user.usr_name, 0, sizeof(second_user.usr_name));					
			printf("\nEnter the username: ");  						
			scanf("%s", second_user.usr_name);					
			write(nfd, &second_user.usr_name, sizeof(second_user.usr_name));	
								
			int found=0; // username is unique . that is not found
			read(nfd, &found, sizeof(found)); 
			if(found==1){
				printf("\nUser Name Already Exists. Please, Enter a Unique Username ");
			}else if(found==0){
				break;}
		}	
		memset(second_user.pass, 0, sizeof(second_user.pass));				
		printf("\nEnter password : ");					
		scanf("%s", second_user.pass);		
		write(nfd, &second_user.pass, sizeof(second_user.pass));
		
		
	}else{
		pass_change(usr_name, nfd);
	} 
	
	admin_print_customers(nfd); 
	admin_print_accounts(nfd);	
}	
	
	
void admin_print_customers(int nfd)
{		
							
	printf("\n************** Printing Customers ***************\nUser_name\tPass\t\tBank_Acc\t\tType");					
	int done;
	struct cust print;
		
	read(nfd, &done, sizeof(done));
	while(1){
		if(done!=0){
			read(nfd, &print, sizeof(print));
			printf("\n%s\t\t%s\t\t%s\t\t%d",print.usr_name, print.pass, print.acc_num, print.type);
			read(nfd, &done, sizeof(done));
		}else{
			break;
		}
	}
}

void admin_print_accounts(int nfd)
{		
	printf("\n********** Printing Accounts ************\nAccount_Number\t\t\tBalance");
	int done;
	int fd=open("account.txt", O_RDWR, 0744);
	struct acc print1;
	read(nfd, &done, sizeof(done));
	while(1){
		if(done!=0){
			read(nfd, &print1, sizeof(print1));
			printf("\n%s\t\t%f", print1.acc_num, print1.bal);
			read(nfd, &done, sizeof(done));
		}else{
			break;
		}
	}		
} 

















