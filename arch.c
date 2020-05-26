

//TODO 1: La commande exit et resoudre le problème d'une commande en un mot
//TODO 2: Résoudre le problème du dernier message quand quelqu'un se connecte
//TODO 3: Faire la fonction !login et afficher le login dans le message !hello
//TODO 4: Faire les options







#include <stdio.h>
#include <stdlib.h> /* pour exit */
#include <unistd.h> /* pour close */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> /* pour memset */
#include <netinet/in.h> /* pour struct sockaddr_in */
#include <arpa/inet.h> /* pour htons et inet_aton */
#include <unistd.h> /* pour sleep */
#include <poll.h>

#define PORT IPPORT_USERRESERVED // = 5000
#define MAX_USERS 5
#define LG_MESSAGE   500
#define LENGHT_LOGIN 50

#define VERSION "1.0"

typedef struct User {
  int socketClient;
  char login[LENGHT_LOGIN];
}user;

void initName(user users[MAX_USERS], int i);
void detectCommande(char messageRecu[LG_MESSAGE],  user *emetteur, user users[MAX_USERS], struct pollfd pollfds[MAX_USERS+1]);
void sendMessage(char destinataire[LENGHT_LOGIN], user *emetteur, char message[LG_MESSAGE], user users[MAX_USERS]);
void version(int socket);
void hello(int socket);
void help(int socket);
void list(int socket, user users[MAX_USERS]);
void login(char messageRecu[LG_MESSAGE],int socket,user users[MAX_USERS]);
void end(user users[MAX_USERS], int socket,struct pollfd pollfds[MAX_USERS+1]);



int main()
{
  int socketEcoute;
  struct sockaddr_in pointDeRencontreLocal;
  socklen_t longueurAdresse;
  int socketDialogue;
  struct sockaddr_in pointDeRencontreDistant;
  char messageEnvoi[LG_MESSAGE]; /* le message de la couche Application ! */
  char messageRecu[LG_MESSAGE]; /* le message de la couche Application ! */
  int ecrits, lus; /* nb d'octets ecrits et lus */
  int retour;
  user users[MAX_USERS];
  struct pollfd pollfds[MAX_USERS + 1];

  memset(users, '\0', MAX_USERS*sizeof(user));

  // Crée un socket de communication
  socketEcoute = socket(PF_INET, SOCK_STREAM, 0); /* 0 indique que l'on utilisera le protocole par défaut associé à SOCK_STREAM soit TCP */

  // Teste la valeur renvoyée par l'appel système socket()
  if(socketEcoute < 0) /* échec ? */
  {
    perror("socket"); // Affiche le message d'erreur
    exit(-1); // On sort en indiquant un code erreur
  }

  printf("Socket créée avec succès ! (%d)\n", socketEcoute);

  // On prépare l'adresse d'attachement locale
  longueurAdresse = sizeof(struct sockaddr_in);
  memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
  pointDeRencontreLocal.sin_family        = PF_INET;
  pointDeRencontreLocal.sin_addr.s_addr   = htonl(INADDR_ANY); // toutes les interfaces locales disponibles
  pointDeRencontreLocal.sin_port          = htons(PORT);

  // On demande l'attachement local de la socket
  if((bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse)) < 0)
  {
    perror("bind");
    exit(-2);
  }

  printf("Socket attachée avec succès !\n");

  // On fixe la taille de la file d'attente à 5 (pour les demande de connexion non encore traitées)
  if(listen(socketEcoute, 5) < 0)
  {
    perror("listen");
    exit(-3);
  }

  // Fin de l'étape n°6 !
  printf("Socket placée en écoute passive ...\n");

  // boucle d'attente de connexion : en théorie, un serveur attend indéfiniment !
  while(1)
  {
    int nevents;
    int nfds = 0, qui = -1;


    // Liste des sockets à écouter
    // socketEcoute + users[].socket => pollfds[]
    pollfds[nfds].fd = socketEcoute;
    pollfds[nfds].events = POLLIN;
    pollfds[nfds].revents = 0;
    nfds++;

    for(int i = 0; i < MAX_USERS; i++) {
      if(users[i].socketClient > 0) {
        pollfds[nfds].fd = users[i].socketClient;
        pollfds[nfds].events = POLLIN;
        pollfds[nfds].revents = 0;
        nfds++;
      }
    }
    nevents = poll(pollfds, nfds, -1);
    if (nevents > 0) {
      // parcours de pollfds[] à la recherche des revents != 0
      for(int u = 0; u < nfds; u++) {
        if(pollfds[u].revents != 0) {
          if(u == 0) {
            for(int i = 0; i < MAX_USERS; i++) {
              if(users[i].socketClient == 0) {
                users[i].socketClient = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, & longueurAdresse);
                if (users[i].socketClient < 0) {
                  perror("accept");
                  close(users[i].socketClient);
                  close(socketEcoute);
                  exit(-4);
                }


                /*fonction bienvue
                verion et !hello
                */
                version(users[i].socketClient);

                hello(users[i].socketClient);



                /*
                login par default
                */
                initName(users, i);




                break;
              }
            }
          }
          else {
            for(int i = 0; i < MAX_USERS; i++) {
              if(pollfds[u].fd == users[i].socketClient) {
                lus = read(users[i].socketClient, messageRecu, LG_MESSAGE*sizeof(char));

                //fct detectCommande
                detectCommande(messageRecu, &users[i], users, pollfds);

                //Fin de la foncion




                switch(lus) {
                  case -1 :
                  perror("read");
                  close(users[i].socketClient);
                  exit(-5);

                  case 0 :
                  fprintf(stderr, "La socket a été fermée par le client !\n\n");
                  close(users[i].socketClient);
                  users[i].socketClient = 0;
                  break;
                  default:
                  printf("Message envoyé par %s : %s (%d octets)\n\n",users[i].login, messageRecu, lus);
                }
              }
            }
          }
          pollfds[u].revents = 0;
        }
      }
      // si c'est la socket socketEcoute => accept() + création d'une nouvelle entrée dans la table users[]

      // sinon c'est une socket client => read() et gestion des erreurs pour le cas de la déconnexion
    } else {
      printf("poll() returned %d\n", nevents);
    }
  }

  // On ferme la ressource avant de quitter
  close(socketEcoute);

  return 0;
}


void initName(user users[MAX_USERS], int i){
  char login_tampon[LENGHT_LOGIN];
  int login_test=1;

  for(int k=1; k <= MAX_USERS; k++){
    sprintf(login_tampon, "Anonyme_%d", k);
    for(int j=0; j < MAX_USERS; j++){

      if(strcmp(login_tampon,users[j].login) == 0 ){
        login_test++;
        break;
      }

    }
    if(login_test==1){
      strcpy(users[i].login,login_tampon);
      break;
    }
    login_test=1;
  }
}


void detectCommande(char messageRecu[LG_MESSAGE],  user *emetteur, user users[MAX_USERS], struct pollfd pollfds[MAX_USERS+1]){ 
	
  int j;
  j = strcspn(messageRecu,"!");
  int mode = 0;
  //test nom commande
  if(j==0){
    char *rest = NULL;
    char *mot;
    int cpt=0;
    char messageCopy[LG_MESSAGE]; /* le message de la couche Application ! */
    char contenu[LG_MESSAGE] = ""; /* le message de la couche Application ! */

    //pour message
    char destinataire[LENGHT_LOGIN] = "";
    char message[LG_MESSAGE] = "";


    /*
    *
    *
    *
    *                       TO DO : Detetcer si il y a un seul mot. Car sans espaces (ex : !version), la commande n'est pas détectée
    *
    *
    *
    *
    */



    strcpy(messageCopy,messageRecu);
    if(strcmp(messageRecu, "!version") == 0 || strcmp(messageRecu, "!version\n") == 0){
          version(emetteur->socketClient);
        }
    else if(strcmp(messageRecu, "!help") == 0 || strcmp(messageRecu, "!help\n") == 0){
          help(emetteur->socketClient);
        }
    else if(strcmp(messageRecu, "!list")== 0 || strcmp(messageRecu, "!list\n") == 0) {
          list(emetteur->socketClient, users);
        }
    else if(strcmp(messageRecu, "!exit")== 0 || strcmp(messageRecu, "!exit\n") == 0) {
          end(users,emetteur->socketClient,pollfds);
        }
    else {
		for (mot = strtok_r(messageCopy, " ", &rest);
		mot != NULL;
		mot = strtok_r(NULL, " ", &rest)) {

		  switch(cpt){
			case 0:
			printf("commande : %s\n", mot);
			printf("messsage reçu  = |%s|\n",messageRecu);
			if(strcmp(mot, "!msg") == 0){
			  mode = 1;
			}
			else if(strcmp(mot, "!login") == 0){
				mode=2;
			}

			else {
			  printf("Commande non reconnue\n");
			}
			break;

			case 1:
			switch (mode) {
			  case 1:
			  strcat(destinataire, mot);
			  break;
			  
			  case 2:
			  strcat(destinataire, mot);
			  login(destinataire,emetteur->socketClient,users);
			}
			break;

			case 2:
			switch (mode) {
			  case 1:

			  strcat(contenu, mot);
			  break;
			}
			break;

			default:
			switch (mode) {
			  case 1:
			  strcat(strcat(contenu, " "), mot);
			  break;
			}
		  }

			cpt++;
		}
    }
    if(mode==1){

      sendMessage(destinataire, emetteur, contenu, users);

    }
    cpt=0;

  }

  j=-2;
}


void sendMessage(char destinataire[LENGHT_LOGIN], user *emetteur, char message[LG_MESSAGE], user users[MAX_USERS]){
  /*
  Forme du message a envoyer au client(s) concerné(es):  !msg login_from type le_message
  Type = | (msg direct) ou & (msg broadcast)
  */

  char envoi[LG_MESSAGE]="!msg ";
  strcat(envoi, emetteur->login);

  if(strcmp(destinataire, "&") == 0){
    strcat(envoi, " & ");
  }
  else{
    strcat(envoi, " | ");
  }

  strcat(envoi, message);
  printf("\nMessage transmis à %s : %s\n", destinataire, envoi);

  ///////////////////////////////// message groupé /////////////////////
  if(strcmp(destinataire, "&") == 0){
    for(int i=0; i<MAX_USERS; i++){
      if(strcmp("", users[i].login)!=0){
        printf("Envoyé à %s\n",users[i].login);
        write(users[i].socketClient, envoi, strlen(envoi));  //TO DO : les erreurs, voir serveur TCP
      }
    }
  }


  ////////////////////////////// message perso /////////////////////////

  for(int i=0; i<MAX_USERS; i++){
    if(strcmp(destinataire, users[i].login)==0){
      write(users[i].socketClient, envoi, strlen(envoi));  //TO DO : les erreurs, voir serveur TCP
    }
  }
}

void version(int socket){
  char message[LG_MESSAGE]="!version ";

  strcat(message, VERSION);

  write(socket, message, strlen(message));  //TO DO : les erreurs, voir serveur TCP
}

void hello(int socket){
  char message[LG_MESSAGE]="!hello \n\t----------------------------------------\n\t| Salut, bienvenu(e) sur notre serveur |\n\t----------------------------------------\n\nPour prendre connaissances des possibilitées que vous avez, utilisez la commande : !help\n\n\n";

  write(socket, message, strlen(message));  //TO DO : les erreurs, voir serveur TCP
}

void help(int socket){
  char message[LG_MESSAGE]="Liste des commandes :\n\t !login mon_login : Se connecter et choisir son username (& interdit) \n\t !msg login le_message : Saisir le destinataire et le message qui lui est adressé \n\t !exit : Sortir du client\n\t !list : Affiche toutes les personnes connectées\n";

  write(socket, message, strlen(message));  //TO DO : les erreurs, voir serveur TCP
}

void list(int socket, user users[MAX_USERS]){
  char message[LG_MESSAGE] = "!list ";
  int i=0;
  printf("users[i].login = %s\n",users[i].login);
  if(strcmp(users[i].login, "") != 0){
    strcat(message, users[i].login);
  }
  i++;
  for(i; i<MAX_USERS; i++){
    printf("i =  %d\n",i);

    printf("users[i].login = %s\n",users[i].login);

    if(strcmp(users[i].login, "") != 0){
      strcat(strcat(message,"&"), users[i].login);

    }
  }
  printf("Fin\n");

  write(socket, message, strlen(message));  //TO DO : les erreurs, voir serveur TCP

}

/*
void login(int socket, char login[LENGHT_LOGIN], users user[MAX_USERS]){
    for(int j=0; j < MAX_USERS; j++){
      if(strcmp(login,users[j].login) == 0 ){
        login_test++;
        break;
      }
    }
    if(login_test==1){
      printf("login = %s\n", login_tampon);
      strcpy(users[i].login,login_tampon);
      break;
    }
    login_test=1;
  }
}*/

void end(user users[MAX_USERS], int socket,struct pollfd pollfds[MAX_USERS+1]){
	close(socket);
	for(int i=0; i<MAX_USERS;i++){
		if(users[i].socketClient==socket){
			users[i].socketClient=0;
			strcpy(users[i].login,"");
			for(int j=0; j<MAX_USERS+1;j++){
				if(pollfds[j].fd==socket){
					pollfds[j].fd=users[i].socketClient;
				}
			}	
		}
	}
}

void login(char messageRecu[LG_MESSAGE],int socket,user users[MAX_USERS]){
	for(int i=0; i<MAX_USERS;i++){
		if(users[i].socketClient==socket){
			strcpy(users[i].login,messageRecu);
		}
	}
}
