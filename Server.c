#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
	
#define TRUE 1
#define FALSE 0
#define PORT 8888

typedef struct{
int socket_descriptor;
char nickname[32];
int last_stranger_matched;
int room_joined;
}client;

typedef struct{
  int client_1;
  int client_2;
  int clients_in_session;
  int termination_status;
  pthread_cond_t cond;
}session;     

typedef struct{
int active_client;
session sessions[100];
pthread_mutex_t sem;
}room;



room Room[5];




void init_client(client *Client,int socket_descriptor,char *nickname){

   Client->socket_descriptor = socket_descriptor;
   strcpy(Client->nickname,nickname);
   Client->last_stranger_matched = -1;
   Client->room_joined = 0;

}

void set_client_room(client *Client,int room_selected){

  Client->room_joined = room_selected;
}


int joinable_session(client Client,int room_selected){

        int i = 0;
        int trovato = -1;

        while(trovato == -1 && i < 99){

             printf("Quanti sono in session %d? %d\n",i,Room[room_selected].sessions[i].clients_in_session);
            if(Room[room_selected].sessions[i].clients_in_session == 1 && Room[room_selected].sessions[i].client_1 != Client.last_stranger_matched){

                  trovato = i;

             }
              
                i++;
         }
             
       return trovato;
    }


 int join_session(client Client, int room_selected,int index){
 
      Room[room_selected].sessions[index].client_2 = Client.socket_descriptor;
      Room[room_selected].sessions[index].clients_in_session++;
      
      return index;
 
 }


 int open_new_session(client Client,int room_selected){

      int i = 0;
      
      
      while(Room[room_selected].sessions[i].clients_in_session != 0 && i<100){
        i++;
      }

      Room[room_selected].sessions[i].client_1 = Client.socket_descriptor;
      Room[room_selected].sessions[i].clients_in_session++;
      return i;
    }


int pick_stranger_socket_descriptor(client Client,int index_session, int room_selected){

  if(Room[room_selected].sessions[index_session].client_1 != Client.socket_descriptor){
    
     return Room[room_selected].sessions[index_session].client_1; 
  
  }

  else{
  
    return Room[room_selected].sessions[index_session].client_2;
  
  }
 }


 void chat_with_stranger(int C1,int stranger,int index_session,int room_selected){

     int recived_message_size = 9999;;
     char message[256];
     char test[10];
     
    while(strcmp(message,"quit\n") != 0){
      
      read(C1,message,256);
      
      if(strcmp(message,"quit\n") != 0){
        if(strlen(message) > 1){
         write(stranger,message,256);
    }
      }
      else{

      write(C1,message,256);
    }
    }
    
    pthread_mutex_lock(&Room[room_selected].sem);
    if(Room[room_selected].sessions[index_session].termination_status == 0){

      write(stranger,"Utente disconnesso digita quit per uscire dalla chat\n",256);
      Room[room_selected].sessions[index_session].termination_status = 1;
    }
    pthread_mutex_unlock(&Room[room_selected].sem);

       
 }


void delete_session(int index_session,int room_selected){

     if(Room[room_selected].sessions[index_session].termination_status == 1){
     Room[room_selected].sessions[index_session].client_1 = 0;
     Room[room_selected].sessions[index_session].client_2 = 0;
     Room[room_selected].sessions[index_session].clients_in_session = 0;
   }
   else{
     Room[room_selected].sessions[index_session].termination_status = 0;
   }

}






  
    

       void *handle_client(void *arg){

         int socket_descriptor = (intptr_t)arg; 
         int room_selected_bytes;
         int menu_selection_bytes;
         int nicknameSize;
         int menu_selection = 9999;
         int room_selected;
         int index_client;
         int index_session;
         int result;
         int stranger;
         char match_occurred_message[256] = "Match avvenuto con ";
         char nickname[32];
         char room[2];
         char menu[2];
         char SD[2];
         char clients_in_room_1[2];
         char clients_in_room_2[2];
         char clients_in_room_3[2];
         char clients_in_room_4[2];

         client Client;
          

         //Prendo dal client il nickname dell'utente
         nicknameSize = read(socket_descriptor,nickname,256);
         nickname[nicknameSize] = '\0';

         //Inizializza la variabile locale Client
         init_client(&Client,socket_descriptor,nickname);


         while(menu_selection != 0){

           menu_selection_bytes = read(socket_descriptor,menu,1);
           menu[menu_selection_bytes] = '\0';
           menu_selection = atoi(menu);

           switch(menu_selection){

              case 1:
                       sprintf(clients_in_room_1,"%d",Room[1].active_client,1);
                       sprintf(clients_in_room_2,"%d",Room[2].active_client,1);
                       sprintf(clients_in_room_3,"%d",Room[3].active_client,1);
                       sprintf(clients_in_room_4,"%d",Room[4].active_client,1);
                       puts(clients_in_room_1);
                       puts(clients_in_room_2);
                       puts(clients_in_room_3);
                       puts(clients_in_room_4);
                       write(socket_descriptor,clients_in_room_1,1);
                       write(socket_descriptor,clients_in_room_2,1);
                       write(socket_descriptor,clients_in_room_3,1);
                       write(socket_descriptor,clients_in_room_4,1);

              break;

              case 2:

              //Leggo in che stanza vuole chattare il client
             room_selected_bytes = read(socket_descriptor,room,1);
             room[room_selected_bytes] = '\0';
             

             //Casto la stanza selezionata da char* a int
            room_selected = atoi(room);
            
            //Aggiungo tra le informazioni del client la stanza nel quale vuole chattare al momento
            set_client_room(&Client,room_selected); 

            //Aggiorniamo il numero di client attivi nella determinata stanza selezionata
            pthread_mutex_lock(&Room[room_selected].sem);
            Room[room_selected].active_client ++;

                        
            write(socket_descriptor,"[+] Sei nella sala d'attesa della room scelta",64);

            result = joinable_session(Client,room_selected);

            if(result != -1){
        
                    index_session = join_session(Client,room_selected,result);
                    pthread_cond_signal(&Room[room_selected].sessions[index_session].cond);
            }

            else{

                     index_session = open_new_session(Client,room_selected);
                     pthread_cond_wait(&Room[room_selected].sessions[index_session].cond,&Room[room_selected].sem);

            }




           stranger = pick_stranger_socket_descriptor(Client,index_session,room_selected);

           
           int k;
         
           
           write(socket_descriptor,SD,2);

         pthread_mutex_unlock(&Room[room_selected].sem);

           
           chat_with_stranger(Client.socket_descriptor,stranger,index_session,room_selected);
           pthread_mutex_lock(&Room[room_selected].sem);
           delete_session(index_session,room_selected);  
           Room[room_selected].active_client--;
           pthread_mutex_unlock(&Room[room_selected].sem);
           Client.last_stranger_matched = stranger;
           
            break;  

         }
      }

      close(socket_descriptor);
  } 
  	
int main()
{
	
	int master_socket , addrlen , new_socket;
  int clientSize = 0;
  int i,j;

	pthread_t tid;
	struct sockaddr_in address;
  
  client Client[10];
  
   pthread_mutex_init(&Room[1].sem,NULL);
   pthread_mutex_init(&Room[2].sem,NULL);
   pthread_mutex_init(&Room[3].sem,NULL);
   pthread_mutex_init(&Room[4].sem,NULL);

   for(i=1;i<5;i++){
          Room[i].active_client = 0;
      for(j=0;j<100;j++){
        Room[i].sessions[j].client_1 = 0;
        Room[i].sessions[j].client_2 = 0;
        Room[i].sessions[j].clients_in_session = 0;
        Room[i].sessions[j].termination_status = 0;
        pthread_cond_init(&Room[i].sessions[j].cond,NULL);
      }
   }
   
   
   

   Room[1].active_client = 0;
   Room[2].active_client = 0;
   Room[3].active_client = 0;
   Room[4].active_client = 0;


  bzero(&address,sizeof(address));
	
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
		

	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}


	printf("Listener on port %d \n", PORT);
		

	if (listen(master_socket, 10) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
		

	addrlen = sizeof(address);
	puts("Waiting for connections ...");
		
	while(TRUE)
	{
		
			if ((new_socket = accept(master_socket,
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}
			
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
			pthread_create(&tid,NULL,handle_client,(void *)(intptr_t)new_socket);
			pthread_detach(tid);
			
			
		
	}
		
	return 0;
}
