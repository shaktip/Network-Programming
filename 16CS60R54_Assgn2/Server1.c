#include"stdio.h"
#include"stdlib.h"
#include"sys/types.h"
#include"sys/socket.h"
#include"string.h"
#include"netinet/in.h"
#include<sys/ipc.h>
#include<sys/shm.h>
#include<signal.h>
#include "SearchPattern.h"
#include<unistd.h>
#include<time.h>

#include <sys/wait.h>
#include <semaphore.h>


#define PORT 4444
#define BUF_SIZE 2000
#define CLADDR_LEN 100
#define ID_LEN 7
#define IDNAME_LEN 12
#define EXITSTR "exit"
#define MAX_CLIENTS 5
#define MSG_LEN 1024


/*
   The message is stored with all details about the sender, receiver ids and names
*/
struct MessageDetails
{
    char FromId[ID_LEN], FromName[IDNAME_LEN];
    char msg[MSG_LEN]; 
    char ToId[ID_LEN], ToName[IDNAME_LEN];
};

/*
   The data structure of shared queue is as follows
*/
struct MessageQueue
{
   int front , rear;
   struct MessageDetails *md;
};
/*
   Enqueue(struct MessageQueue *mqueue, struct MessageDetails m)
   --is to enqueue a new message m into a message queue structure mqueue
*/
void Enqueue(struct MessageQueue *mqueue , struct MessageDetails m)
{
   mqueue->md[mqueue->rear] = m;
   mqueue->rear++;   
}
/*
    MessageDetails Dequeue(struct MessageQueue *mqueue)
     -- is to dequeue the front element from the mqueue 
*/
struct MessageDetails  Dequeue( struct MessageQueue *mqueue)
{
   struct MessageDetails m = mqueue->md[mqueue->front++];
   return m;
}
/*
   int isEmpty(struct MessageQueue *mqueue)
    -- is to return true if mqueue is empty otherwise false.
*/
int isEmpty(struct MessageQueue *mqueue)
{
  return mqueue->front == mqueue->rear;
}


/*
char IDs[MAX_CLIENTS][6] = {"clt01", "clt02", "clt03","clt04","clt05"};
char Names[MAX_CLIENTS][11] = {"client0001", "client0002" ,"client0003", "client0004", "client0005"};
*/
char IDs[MAX_CLIENTS][6];
char Names[MAX_CLIENTS][11];
int pid;
int *visit;
time_t *timeInfo;


//struct sockaddr_in *ClientAddr;

/**
   int split(char *buff, char ch,  char *text, char *pattern)
   it splits buffer into first occurrence of ch and left side is transferred into text
   and right into pattern.
   it returns number of occurrences of ch into buffer.
*/

int split(char *buff , char ch , char *id , char *idname , char *text)
{
   memset(text, 0, BUF_SIZE);
   memset(id, 0, ID_LEN);
   memset(idname , 0 , IDNAME_LEN);
    
   char *ptr = strchr(buff , ch);
   
   int i,j;
   char *temp;
   temp = (char *) malloc(IDNAME_LEN * sizeof(char));
   
   if(ptr == NULL)
     return 0;
   for(i = 0 ; (buff+i) != ptr ; i++)
      temp[i] = buff[i];
   temp[i] = '\0'; 
   temp = trimwhitespace(temp);  
   if(strlen(temp) == 5)
   {
      strcpy(id , temp);
      for(j = 0 ; j < MAX_CLIENTS ; j++)
      {
         if(strcmp(id , IDs[j]) == 0)
         {
            strcpy(idname , Names[j]);
            break;
         }
      }
   }
   else
   {
      strcpy(idname , temp);   
      for(j = 0 ; j < MAX_CLIENTS ; j++)
      {
         if(strcmp(idname , Names[j]) == 0)
         {
            strcpy(id, IDs[j]);
            break;
         }
      }
   }   
   int k = 0;
   int counter = 1;
   for(++i; i < strlen(buff) ; i++)
   {
      if(buff[i] == ch)
        counter++;
      text[k++] = buff[i];
   }
   text[k] = '\0';  
   char *s = trimwhitespace(text);     
   strcpy(text , s);
   printf("Text is %s \n " , text);  
   return counter;
}

int sockfd, newsockfd;
/** 
     void sig_handler1(int signum)
     is close the socket on ^C hit.
*/
void sig_handler1(int signum)
{
    printf( "Attempt to Close the connection\n");
    int i;
    /*
    for(i = 0 ; i < MAX_CLIENTS ; i++)
    {
       
    }
    for(i = MAX_CLIENTS - 1 ; i >= 0 ; i--)
    {
       printf("\n Value of i is %d ",i);
       if(ProcIds[i] != -1)
       {
          printf("Kill %d procid \n " , ProcIds[i]);
          kill(ProcIds[i], SIGTERM);
          
       }
    }
    */
    //sleep(3);     
    kill(pid, SIGTERM);
    close(sockfd);
    exit(1);
}


/** 
  Error codes sent to client 
  -1 : Id is missing
  -2 : text is missing
  -3 : text length is exceeding
  -4 : Invalid Id
  -5 : Receiver and Sender is Same
  0  : on success
*/

int checkSpecification(char *text, char *id, char *idname , int clientpid)
{
    int i ;
    int m = strlen(text);
    int n = strlen(id);
    if(m == 0) return -2;
    if(n == 0) return -1;
    if(m > MSG_LEN) return -3;
    if(n > 2*MAX_CLIENTS) return -4;
    if(strcmp(id , IDs[clientpid]) == 0 || strcmp(idname, Names[clientpid]) == 0)
      return -5;
    int flag = 0;
    for(i = 0 ; i < MAX_CLIENTS ; i++)
    {
       if(visit[i] == 0)
       {
          if(strcmp(IDs[i], id) == 0 && strcmp(Names[i] , idname) == 0)
          {
             flag = 1;
             break;
          }
       }       
    }
    if(flag == 0)
      return -4;
    return 0;
}

/**
   void broadCast(char *message, struct MessageQueue *AllMsgQueue)
   --to  enqueue the message to all active clients 
*/

void broadCast(char *message, struct MessageQueue *AllMsgQueue)
{
   int i;
   for(i =  0 ; i < MAX_CLIENTS ; i++)
   {
     if(visit[i] == 0)
     {
       struct MessageDetails q;
       strcpy(q.FromId , "Server");
       strcpy(q.FromName , "Server");
       strcpy(q.msg , message);
       strcpy(q.ToId , IDs[i]);
       strcpy(q.ToName , Names[i]);
       Enqueue(AllMsgQueue , q);    
     }
   }
}

int main() 
{
  struct sockaddr_in addr, cl_addr;
  int i,j;
  
  int ret, len;
  char buffer[BUF_SIZE];
  pid_t childpid;
  char clientAddr[CLADDR_LEN];
  int clientpid=0;
  char broadCastMsg[BUF_SIZE];
  
  /*
    Following code is to generate 5 digit random client id and 10 digit random client name
  */
  
  int k;
  int counterArrayIDs[100];
  int counterArrayNames[10000];
  char aa[10];
  
  for(k=0;k<100;k++)
    counterArrayIDs[k] = 0;
  for(k = 0  ; k < 10000 ;k++)
    counterArrayNames[k] = 0;
      
  for(k=0;k<5;k++)
  {
     strcpy(IDs[k],"clt");
     strcpy(Names[k],"client");
     int r = 0;
     do
     {
         r = 10 + rand() % 90;
     }while(counterArrayIDs[r] != 0);
     counterArrayIDs[r]++;
     sprintf(aa ,"%d", r);
     strcat(IDs[k] , aa);
     do
     {
        r = 1000 + rand() % 9000;
     }while(counterArrayNames[r] != 0);
     counterArrayNames[r]++;
     sprintf(aa, "%d" , r);
     strcat(Names[k] , aa);
     printf(" %s %s ",IDs[k], Names[k]);
  }
  
  /*
     Following code is for shared memory
  */
  
     
  sem_t *mutex;
  int shmid0 = shmget(IPC_PRIVATE, sizeof(sem_t), IPC_CREAT | 0660);
  if (shmid0 < 0) exit(1);
  mutex = (sem_t *)shmat(shmid0, NULL, 0);
  if (mutex == (void *) (-1)) exit(1);
  //printf("shmid0 is %d \n", shmid0); 
  sem_init(mutex, 1, 1);
    
  int ShmID;
  int *NoOfClients;
  ShmID = shmget(IPC_PRIVATE , sizeof(int) , IPC_CREAT | 0666);
  NoOfClients = (int *)shmat(ShmID , NULL , 0);
  
  if(NoOfClients == NULL)
  {
     printf("Error! in creating shared variable");
     exit(0);
  }
  
  
  int VisitID;
  VisitID = shmget(IPC_PRIVATE , MAX_CLIENTS * sizeof(int) , IPC_CREAT | 0666);
  visit = (int *)shmat(VisitID , NULL , 0);
  if(visit == NULL)
  {
     printf("Error! in creating shared variable");
     exit(0);
  }
  for(i = 0 ; i < MAX_CLIENTS ; i++)
  {
     visit[i] = -1;
  }
  
  
  int TimeID;
  TimeID = shmget(IPC_PRIVATE , MAX_CLIENTS * sizeof(time_t) , IPC_CREAT | 0666);
  timeInfo = (time_t *)shmat(TimeID , NULL , 0);
  if(timeInfo == NULL)
  {
       printf("Error! in creating shared variable for Time info");
       exit(0);
  }
  

  int ShmID1;
  int *NoOfActiveClients;
  ShmID1 = shmget(IPC_PRIVATE , sizeof(int) , IPC_CREAT | 0666);
  NoOfActiveClients = (int *)shmat(ShmID1 , NULL , 0);
  if(NoOfActiveClients == NULL)
  {
     printf("Error! in creating shared variable");
     exit(0);
  }
  
  
  
  int AllMsgQueueID;
  struct MessageQueue *AllMsgQueue;
  AllMsgQueueID = shmget(IPC_PRIVATE , sizeof(struct MessageQueue) , IPC_CREAT | 0666);
  AllMsgQueue = (struct MessageQueue *)shmat(AllMsgQueueID , NULL , 0);
  if(AllMsgQueue == NULL)
  {
     printf("Error! in creating shared variable");
     exit(0);
  }
  
  int MessageLenId;
  MessageLenId = shmget(IPC_PRIVATE , 16000 * sizeof(struct MessageDetails) , IPC_CREAT | 0666);
  AllMsgQueue->md = (struct MessageDetails *) shmat(MessageLenId , NULL , 0);
  if(AllMsgQueue->md == NULL)
  {
     printf("Error! in Creating shared variable");
     exit(0);
  } 
  AllMsgQueue->front = AllMsgQueue->rear = 0;
  
  struct MessageQueue *AllMsgForClient[MAX_CLIENTS];
  for(i = 0 ; i < MAX_CLIENTS ; i++)
  {
    int MsgID1;
    MsgID1 = shmget(IPC_PRIVATE , sizeof(struct MessageQueue) , IPC_CREAT | 0666);
    AllMsgForClient[i] = (struct MessageQueue *)shmat(MsgID1 , NULL , 0);
    if(AllMsgForClient[i] == NULL)
    {
     printf("Error! in creating shared variable");
     exit(0);
    }
    
    MsgID1 = shmget(IPC_PRIVATE , 16000  * sizeof(struct MessageDetails) , IPC_CREAT | 0666);
    AllMsgForClient[i]->md = (struct MessageDetails *) shmat(MsgID1, NULL , 0);
    if(AllMsgForClient[i]->md == NULL)
    {
     printf("Error! in Creating shared variable");
     exit(0);
    } 
    AllMsgForClient[i]->front = AllMsgForClient[i]->rear = 0;
    
  }
  
  
  /**
    NoOfClients is a shared variable to handle number of active clients connected to 
    server
  */
  char *text, *id, *idname;
  text = (char *) malloc(BUF_SIZE * sizeof(char));
  id = (char *) malloc(ID_LEN * sizeof(char));
  idname = (char *) malloc(IDNAME_LEN * sizeof(char));
  
  /**
      to create socket.
  */ 
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    printf("Error creating socket!\n");
    exit(1);
  }
  printf("Socket created...\n");
 
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = PORT;
  /**
    To bind the server to client on given port
  */
  ret = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
  if (ret < 0) 
  {
    printf("Error binding!\n");
    exit(1);
  }
  printf("Binding done\n");

  printf("Waiting for a connection\n");
  listen(sockfd, 5);
  *NoOfClients = 0;
  *NoOfActiveClients = 0;
  /**
     A queue which is looking for a shared queue and forwarding the message to respective client's queue, to whome it must be forwarded.
  */
  FILE *fp;
  fp = fopen("Data.csv","w");
  /*
     Following parent process takes care of distributing the messages to respective clients
  */
  if((pid = fork()) != 0)
  {
      signal(SIGINT, sig_handler1);
      for(;;)
      {   
          sem_wait(mutex);
          if(!isEmpty(AllMsgQueue))
          {
              struct MessageDetails pqr = Dequeue(AllMsgQueue);
              
              time_t mytime;
              mytime = time(NULL);
              //ctime by default contains \n which makes new entry in CSV file hence the field is kept at last
              if(strcmp(pqr.FromId , "Server") != 0)
                fprintf(fp, "%s , %s , %s , %s , %s , %s",pqr.FromId, pqr.FromName, pqr.ToId, pqr.ToName, pqr.msg , ctime(&mytime));
              for(j = 0 ; j < *NoOfClients ; j++)
                 if(strcmp(pqr.ToId , IDs[j]) == 0)
                 {
                    Enqueue(AllMsgForClient[j] , pqr);
                    break;
                 }
          }
          sem_post(mutex);
          sleep(1);
      }
  }
  
  
  /**
     Server runs infinite to serve to client.
  */
  
  for (;;)
  { //infinite loop
    
    len = sizeof(cl_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cl_addr, &len);
    
    
    
    if (newsockfd < 0)
    {
       printf("Error accepting connection!\n");
       exit(1);
    }
    inet_ntop(AF_INET, &(cl_addr.sin_addr), clientAddr, CLADDR_LEN);
    //printf("NewSockfd : %d ClientAddr : %p", newsockfd , &clientAddr);
    
    if(*NoOfClients < MAX_CLIENTS)
    {
      timeInfo[clientpid] = time(NULL);  
      clientpid++; 
      sem_wait(mutex);
        (*NoOfClients)++;
        (*NoOfActiveClients)++;
      sem_post(mutex);
      printf("Connection Accepted,  No of Active Clients = %d, No of Session Clients = %d\n", (*NoOfActiveClients) , (*NoOfClients));
      sprintf(buffer, "\n\nWelcome To ChatShat! Your Id  %s, Name %s Time %s\n", IDs[*NoOfClients - 1] , Names[*NoOfClients - 1], ctime(&timeInfo[*NoOfClients-1]) );
      
      
      sprintf(broadCastMsg , "New Client with id : %s and Name : %s has Arrived",IDs[*NoOfClients-1],Names[*NoOfClients - 1]);
      broadCast(broadCastMsg , AllMsgQueue);
      
      
      sem_wait(mutex);
         visit[*NoOfClients - 1] = 0;
      sem_post(mutex);
      
      
      ret = sendto(newsockfd, buffer , BUF_SIZE , 0, (struct sockaddr *) &cl_addr, len);
      if(ret < 0)
      {
          printf("Error in Sending Data from server\n");
          exit(1);
      }
      
     
    /**
       For every connected client a new thread is created(child process) for
       communication using fork system call
    */
    
      if ((childpid = fork()) == 0)   //Child process
      { //creating a child process
        // signal(SIGINT, sig_handler1);
         close(sockfd); 
      //stop listening for new connections by the main process. 
      //the child will continue to listen. 
      //the main process now handles the connected client.

         
          int pid;
          if((pid = fork()) != 0)   //Parent Process
          {   
            
            for(; ;)
            {
              
              memset(buffer, 0, BUF_SIZE);
              ret = recvfrom(newsockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &cl_addr, &len);
              if(ret == 0)
              { 
                printf("Error receiving data!\n");
                
                visit[clientpid - 1] = 2;
                sprintf(broadCastMsg , "Client id %s Name %s has quit" , IDs[clientpid-1] , Names[clientpid-1]);
                sem_wait(mutex);
                    broadCast(broadCastMsg , AllMsgQueue);
                sem_post(mutex);    
                
                kill(pid, SIGTERM);
                close(newsockfd);
                
                sem_wait(mutex);
                   printf("Connection is closed,  No of Active Clients = %d\n", --(*NoOfActiveClients));
                sem_post(mutex);
                  
                exit(1);
              }
              
              printf("Received data from %s: %s\n", IDs[clientpid-1], buffer); 
              char *orgmsg;
              orgmsg = (char *) malloc(strlen(buffer) * sizeof(char));
              strcpy(orgmsg , buffer);
              /*
                 To handle the Exit of client
              */
              if(strcmp(buffer , EXITSTR) == 0)
              {
                sem_wait(mutex);
                  visit[clientpid - 1] = 2;
                  sprintf(broadCastMsg , "Client id %s Name %s has quit" , IDs[clientpid-1] , Names[clientpid-1]);
                  broadCast(broadCastMsg , AllMsgQueue);
                
                
                  kill(pid, SIGTERM);
                  close(newsockfd);
                  printf("Connection is closed,  No of Active Clients = %d\n", --(*NoOfActiveClients));
                sem_post(mutex);
                return 0;
              }
              /*
                 To handle broadcast message of all the active clients
              */
              if(strcmp(buffer , "A") == 0)
              {
                  for(i =  0 ; i < MAX_CLIENTS ; i++)
                  {
                     if(visit[i] == 0 && i != clientpid - 1)
                     {
                       struct MessageDetails q;
                       strcpy(q.FromId , "Server");
                       strcpy(q.FromName , "Server");
                       char message[MSG_LEN];
                       sprintf(message, "Client Id : %s Client Name : %s",IDs[i],Names[i]);
                       strcpy(q.msg , message);
                       strcpy(q.ToId , IDs[clientpid - 1]);
                       strcpy(q.ToName , Names[clientpid - 1]);
                       sem_wait(mutex);
                          Enqueue(AllMsgQueue , q);    
                       sem_post(mutex);   
                     }
                  }
              }
              else
              {      
                 int no = split(buffer , ':', id , idname, text);
                 int errorNo;
                 if(no != 0)
                 {
                    printf("Id is : %s Text is : %s" , id , text);
                    errorNo = checkSpecification(text , id , idname , clientpid-1);
                 }   
                 /*
                   To check the error conditions in the message
                 */
                 if(errorNo != 0 || no == 0)
                 {
                    if(no == 0)
                       sprintf(buffer , "Error! Colon missing in %s",orgmsg);
                    else if(errorNo == -1)
                       sprintf(buffer , "Error! Id or Name is missing in %s",orgmsg);
                    else if(errorNo == -2)
                       sprintf(buffer, "Error! Text is missing in %s",orgmsg);
                    else if(errorNo == -3)
                       sprintf(buffer, "Error! Text length exceeding 30 charachters in %s",orgmsg);
                    else if(errorNo == -4)
                       sprintf(buffer, "Error! Invalid Id or Name in %s",orgmsg);  
                    else if(errorNo == -5)
                       sprintf(buffer , "Error! Sender And Receiver cannot be of same id in %s", orgmsg);    
                    struct MessageDetails q;
                    strcpy(q.FromId , "Server");
                    strcpy(q.FromName , "Server");
                    strcpy(q.msg , buffer);
                    strcpy(q.ToId , IDs[clientpid - 1]);
                    strcpy(q.ToName , Names[clientpid - 1]);
                    sem_wait(mutex);
                       Enqueue(AllMsgQueue , q);    
                    sem_post(mutex);   
                 }
                 /*
                   To send the message to the corresponding client by enqueue in allmsgqueue
                 */
                 else
                 {
                    //sprintf(buffer , "Id is : %s \nText is : %s\n" , id , text);
                    struct MessageDetails q;
                    strcpy(q.FromId , IDs[clientpid - 1]);
                    strcpy(q.FromName , Names[clientpid - 1]);
                    strcpy(q.msg , text);
                    strcpy(q.ToId , id);
                    strcpy(q.ToName , idname);
                    sem_wait(mutex);
                       Enqueue(AllMsgQueue , q);
                    sem_post(mutex);   
                  }
               }  
               sleep(0.5);
             }   //Infinite for
           } //Parent process handling continuous Receiving 
          
           else
           {
             for(;;)
             {
                /*
                   Handling sending of the messages.
                */
                sem_wait(mutex);
                if(!isEmpty(AllMsgForClient[clientpid - 1]))
                {
                   struct MessageDetails q = Dequeue(AllMsgForClient[clientpid - 1]);
                   //sprintf(buffer , " FromId %s FromName %s ToId %s ToName %s Msg %s",q.FromId, q.FromName, q.ToId, q.ToName, q.msg);
                   sprintf(buffer , " From(Id %s, Name %s) - %s\n",q.FromId, q.FromName, q.msg);
                   ret = sendto(newsockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &cl_addr, len);   
                   if (ret < 0)
                   {  
                       printf("Error sending data!\n");  
                       exit(1);  
                   }  
                   printf("Sent data to %s: %s\n", clientAddr, buffer);
                   memset(buffer, 0, BUF_SIZE);
                }
                sem_post(mutex);
                sleep(0.5);
             }  //Infinite sending
           }  // Closing of Sender    
    } //Child process closed for sender and receiver
    close(newsockfd);
   } //If MAX_CLIENTs are not yet created.
   else // No of client Not less than MAX_CLIENT, so deny the connection
   {
      sprintf(buffer, "Server: -1");
      ret = sendto(newsockfd, buffer , BUF_SIZE , 0, (struct sockaddr *) &cl_addr, len);
      if(ret < 0)
      {
          printf("Error in Sending Error Data from server\n");
          exit(1);
      }
      close(newsockfd);
   }
 }//Infinite loop of Parent process.
 return 0;
}

