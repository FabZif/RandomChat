#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#define MAX 80
#define PORT 8888
#define SA struct sockaddr

char nickname[32];

void *reading_thread_function(void *arg){

    int socket_descriptor = (intptr_t) arg;
    char recived_message[256];
    int size_message;
    
    while(strcmp(recived_message,"quit\n") != 0){

            read(socket_descriptor,recived_message,256);
             size_message = strlen(recived_message);
            if(strcmp(recived_message,"quit\n") != 0 && size_message > 1){
            printf("%s : %s\n",nickname,recived_message);
        }

    }
    pthread_exit();
}

void handler_sigint(int sig){

    puts("Non e' possibile uscire con i shortcut\n");
}


void handle_chat(int socket_descriptor){

     char selected_room[2];
     
     char selection[2];
     char test[5];
     char waiting_room_message[64];
     char match_occurred_message[256];
     char sd_matched[2];
     char message[256];
     char clients_in_room_1[2];
     char clients_in_room_2[2];
     char clients_in_room_3[2];
     char clients_in_room_4[2];

     int match_occurred_message_size;
     int menu_selection = 9999;;
     int sizeR1,sizeR2,sizeR3,sizeR4;
     pthread_t reading_thread;
     pthread_t writing_thread;

     //Inviamo al server il nickname scelto
     printf("[+] Inserisci nickname per la sessione...\n");
     scanf("%s",nickname);
     write(socket_descriptor,nickname,strlen(nickname));

     while(menu_selection != 0){
     printf("[1] Per conoscere quanti uteni attivi ci sono nelle varie stanze\n");
     printf("[2] Per iniziare a chattare\n");
     printf("[0] Per uscire\n");
     
     scanf("%s",selection);
     write(socket_descriptor,selection,1);
    
     menu_selection = atoi(selection);

     switch(menu_selection){
 puts(selection);
        case 1:

                sizeR1 = read(socket_descriptor,clients_in_room_1,1);
                sizeR2 = read(socket_descriptor,clients_in_room_2,1);
                sizeR3 = read(socket_descriptor,clients_in_room_3,1);
                sizeR4 = read(socket_descriptor,clients_in_room_4,1);
                clients_in_room_1[sizeR1] = '\0';
                clients_in_room_2[sizeR2] = '\0';
                clients_in_room_3[sizeR3] = '\0';
                clients_in_room_4[sizeR4] = '\0';
                printf("Client attivi nella stanza a tema politica : %s\n",clients_in_room_1);
                printf("Client attivi nella stanza a tema calcio : %s\n",clients_in_room_2);
                printf("Client attivi nella stanza a tema amicizia : %s\n",clients_in_room_3);
                printf("Client attivi nella stanza a tema incontri : %s\n",clients_in_room_4);



        break;

        case 2:

     //Inviamo al server la stanza scelta in cui chattare
     printf("[1] Politic room\n");
     printf("[2] Football room\n");
     printf("[3] Friends room\n");
     printf("[4] Dating room\n");
     scanf("%s",selected_room);
     write(socket_descriptor,selected_room,1);
     
     //Messaggio dal server di avvenuta entrata nella sala d'attesa della room scelta
     read(socket_descriptor,waiting_room_message,64);
     printf("%s\n", waiting_room_message);

     read(socket_descriptor,sd_matched,2);
     
     printf("Sei stato matchato con %s\n",sd_matched);
  
  
     strcpy(message,"\n");

     //Creazione di due thread, uno si occupera della lettura dei messaggi ricevuti dallo stranger, l'altro della scrittura
     pthread_create(&reading_thread,NULL,reading_thread_function,(void*)(intptr_t)socket_descriptor);
     while(strcmp(message,"quit\n") != 0){
        fgets(message,256,stdin);
        write(socket_descriptor,message,256);
        }

     
     break;


     case 3:
            close(socket_descriptor);
            break;

 }
}
}

int main(){

	int socket_descriptor, connection_descriptor;
	struct sockaddr_in server_address, client_address;

    signal(SIGINT,handler_sigint);

	
	socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_descriptor == -1) {
		printf("[!] Creazione socket fallita!\n");
		exit(0);
	}
	else
		printf("[+] Socket creata con successo\n");

	bzero(&server_address, sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = htons(PORT);


	if (connect(socket_descriptor, (SA*)&server_address, sizeof(server_address)) != 0) {
		printf("[!] Connessione con il server fallita!\n");
		exit(0);
	}
	else
		printf("[+] Connesso al socket %d\n",socket_descriptor);

	// handle_chattion for chat
	handle_chat(socket_descriptor);

	// close the socket
	close(socket_descriptor);
}

