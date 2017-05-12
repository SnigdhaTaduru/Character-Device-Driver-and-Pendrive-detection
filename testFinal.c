#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>


#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
#define MAX_SIZE 80
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int main(){

   int ret,fd,choice,choice1,n,t,ch=1,ch1=1;
   FILE *fp;
   int fd1;
   char command[100];
   char stringToSend[BUFFER_LENGTH],buffer[MAX_SIZE];
   printf("Starting device test code example...\n");
   while(ch1==1)
   {
   	printf("\n**********************************************\n");
   	printf("MENU:\n 1)Reading and Writing into Simple Character Device\n2)Detection of USB\n");
   	printf("\nEnter you choice\n");
   	scanf("%d",&choice1);
   	switch(choice1)
   	{
   		case 1:
		   fd = open("/dev/ebbchar", O_RDWR);             // Open the device with read/write access
		   if (fd < 0){
		      perror("Failed to open the device...");
		      return errno;
		   }
		   while(ch==1)
		   {
		   	
		   	printf("\nMENU ::\n1) Enter characters into the device\n2) Write the file contents to the kernel\nEnter your choice\n\n");
		   	scanf("%d",&choice);
		   	
		   	getchar();
		   	system("clear");
		   	switch(choice)
		   	{
		   		case 1:
						   printf("Type in a short string to send to the kernel module:\n");
						   scanf("%[^\n]%*c", stringToSend);              // Read in a string (with spaces)
						   printf("Writing message to the device [%s].\n", stringToSend);
						   //printf("before write in test\n");
						   ret = write(fd, stringToSend, strlen(stringToSend)); // Send the string to the LKM
						   //printf("after write in test\n");
						   if (ret < 0){
						      perror("Failed to write the message to the device.");
						      return errno;
						   }

						   printf("Press ENTER to read back from the device...");
						   getchar();

						   printf("Reading from the device...\n");
						   ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the LKM
						   if (ret < 0){
						      perror("Failed to read the message from the device.");
						      return errno;
						   }
						   printf("The received message is: [%s]\n", receive);
						   break;
					case 2:
						   
						   //printf("dskjbcdsbcsd\n");
						   printf("Enter file name\n");
						   scanf("%[^\n]%*c", stringToSend);
						   //strcpy(stringToSend,"abc");
						   fp=fopen(stringToSend,"r");
						   while(fgets(buffer,MAX_SIZE,fp)!=NULL)
						   {
							n=strlen(buffer);
							write(fd,buffer,n);
						   }
						   printf("file contents written into kernel\n");
						   printf("Press ENTER to view messages from the kernel...");
						   getchar();
						   strcpy(command,"sudo tail -f /var/log/kern.log");
						   system(command);
						      		
						   break;
					default: 
							printf("Enter valid option\n");		  
		
				}
				printf("do you want to continue with this simple Character device\n");
				scanf("%d",&ch);
			   }
			   break;
		case 2:
			fd=open("/dev/pen0",O_RDWR);
			 if(fd<0)
	 		 {
	 			printf("/dev/pen0 not created (usb not attached)\n");
	 			exit(0);
	 		 }
	 		 else
	 		 {
	 		 	printf("Pen drive detected\n");
			 	printf("device /dev/pen0 opened....\n");
			 	printf("press ENTER to view messages from the kernel");
			 	getchar();
			 	strcpy(command,"sudo tail -f /var/log/kern.log");
				system(command);
			 }
			 break;
		case 3:
			exit(0);
			break;
		default:
			printf("Invalid option\n");
	   }
	   printf("do you want to continue this program\n");
	   scanf("%d",&ch1);
	}
	printf("End of the program\n");
   return 0;
}
