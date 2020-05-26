#include <stdio.h>
#include <stdlib.h> /* pour exit */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> /* pour memset */
#include <netinet/in.h> /* pour struct sockaddr_in */
#include <arpa/inet.h> /* pour htons et inet_aton */
#define LG_MESSAGE 256
#define LENGHT_LOGIN 50
#include <poll.h>



#include <unistd.h> /* pour close */

#include <poll.h>


void menu();
void reception(char messageRecu[LG_MESSAGE]);
int sendMessage(int descripteurSocket);
int list(int descripteurSocket);
int deconnection(int descripteurSocket);
void getMessage(char messageRecu[LG_MESSAGE]);
void detectCommande(char message[LG_MESSAGE], int socket);
void getList(char messageRecu[LG_MESSAGE]);


int main()
{
  int descripteurSocket;
  struct sockaddr_in pointDeRencontreDistant;
  socklen_t longueurAdresse;
  char messageEnvoi[LG_MESSAGE]; /* le message de la couche Application ! */
  char messageRecu[LG_MESSAGE]; /* le message de la couche Application ! */
  char commandeClient[LG_MESSAGE]; /* le message de la couche Application ! */
  char* index = &commandeClient[0];
  int ecrits, lus = 0; /* nb d’octets ecrits et lus */
  int retour;
  struct pollfd pollfds[2];

  //--- Début de l’étape n°1 :
  // Crée un socket de communication
  descripteurSocket = socket(PF_INET, SOCK_STREAM, 0); /* 0 indique que l’on utilisera le
  protocole par défaut associé à SOCK_STREAM soit TCP */
  // Teste la valeur renvoyée par l’appel système socket()
  if(descripteurSocket < 0) /* échec ? */
  {
    perror("socket"); // Affiche le message d’erreur
    exit(-1); // On sort en indiquant un code erreur
  }
  //--Fin de l’étape n°1 !

  printf("Socket créée avec succès ! (%d)\n", descripteurSocket);

  //--- Début de l’étape n°2 :
  // Obtient la longueur en octets de la structure sockaddr_in
  longueurAdresse = sizeof(pointDeRencontreDistant);
  // Initialise à 0 la structure sockaddr_in
  memset(&pointDeRencontreDistant, 0x00, longueurAdresse);
  // Renseigne la structure sockaddr_in avec les informations du serveur distant
  pointDeRencontreDistant.sin_family = PF_INET;
  // On choisit le numéro de port d’écoute du serveur
  pointDeRencontreDistant.sin_port = htons(IPPORT_USERRESERVED); // = 5000
  // On choisit l’adresse IPv4 du serveur
  inet_aton("127.0.0.1", &pointDeRencontreDistant.sin_addr); // à modifier selon ses besoins
  // Débute la connexion vers le processus serveur distant
  if((connect(descripteurSocket, (struct sockaddr *)&pointDeRencontreDistant,
  longueurAdresse)) == -1)
  {
    perror("connect"); // Affiche le message d’erreur
    close(descripteurSocket); // On ferme la ressource avant de quitter
    exit(-2); // On sort en indiquant un code erreur
  }
  //--- Fin de l’étape n°2 !

  printf("Connexion au serveur réussie avec succès !\n");



  pollfds[1].fd = descripteurSocket;
  pollfds[1].events = POLLIN;

  pollfds[0].fd = 0;
  pollfds[0].events = POLLIN;

  menu();

  while(1){
    int nevents =0;
    //--- Début de l’étape n°4 :
    pollfds[1].revents = 0;
    pollfds[0].revents = 0;

    // Initialise à 0 les messages
    memset(messageEnvoi, 0x00, LG_MESSAGE*sizeof(char));
    memset(messageRecu, 0x00, LG_MESSAGE*sizeof(char));
    memset(commandeClient, 0x00, LG_MESSAGE*sizeof(char));

    nevents = poll(pollfds, 2, -1);

    if(nevents>0){
      // Envoie un message au serveur
      if (pollfds[0].revents > 0) {

        lus = read(0, commandeClient, LG_MESSAGE*sizeof(char)); /* attend un message
        de TAILLE fixe */
      //  printf("lus = %d\n", lus);
        commandeClient[strlen(commandeClient)-1]='\0';
    //    printf("Voici le message : %s\n", commandeClient);
        switch(lus)
        {
          case -1 : /* une erreur ! */
          perror("read");
          close(descripteurSocket);
          exit(-4);
          case 0 : /* la socket est fermée */
          fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
          close(descripteurSocket);
          return 0;
        /*  default:
      //    printf("Message reçu du serveur : %s (%d octets)\n\n", messageRecu, lus);*/
        }



        detectCommande( commandeClient, descripteurSocket);

      }
      else if (pollfds[1].revents > 0) {

        /* Reception des données du serveur */
        lus = read(descripteurSocket, messageRecu, LG_MESSAGE*sizeof(char)); /* attend un message
        de TAILLE fixe */
        switch(lus)
        {
          case -1 : /* une erreur ! */
          perror("read");
          close(descripteurSocket);
          exit(-4);
          case 0 : /* la socket est fermée */
          fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
          close(descripteurSocket);
          return 0;
          default: /* réception de n octets */
          reception(messageRecu);
        //  printf("Message reçu |%s| envoyé avec succès (%d octets)\n\n", messageRecu, lus);

        }
        //--- Fin de l’étape n°4 !
        // On ferme la ressource avant de quitter
      }
    }
  }
  close(descripteurSocket);
  return 0;
}



void menu(){
  printf("Voici les différentes possibilitées :\n");
  printf("\t1 - Changer de nom d'utilisateur\n");
  printf("\t2 - Avoir la liste des utilisateurs\n");
  printf("\t3 - Envoyer un message (type : message)\n");
  printf("\t4 - Se déconnecter\n");
  printf("\nEntrez le numéro de la commande souhaitée : \n");
}

int sendMessage(int descripteurSocket){
  char destinataire[LENGHT_LOGIN];
  char message[LG_MESSAGE]; /* le message de la couche Application ! */
  char contenu[LG_MESSAGE]; /* le message de la couche Application ! */
  int ecrits;
  char *ptcontenu = &contenu[0];
  memset(destinataire, 0, LENGHT_LOGIN);
  memset(contenu, 0, LG_MESSAGE);
  memset(message, 0,LG_MESSAGE);

  printf("Entrez le nom du destinataire : \n");
  fgets(destinataire,sizeof(destinataire), stdin);
  printf("Entrez le message que vous souhaiter envoyer : \n");
  fgets(contenu,sizeof(contenu), stdin);
  printf("Contenu = %s\n", contenu);
  destinataire[(strlen(destinataire) )-1]='\0';

  sprintf(message, "!msg ");
  strcat(message, destinataire);

  contenu[(strlen(contenu) )-1]='\0';

  strcat(message, " ");
  strcat(message, contenu);
/*  printf("message = %s\n", message);
  printf("strlen(message) = %ld \n", strlen(message));*/

  ecrits = write(descripteurSocket, message, strlen(message)); // message à TAILLE variable
  switch(ecrits)
  {
    case -1 :
    perror("write");
    close(descripteurSocket);
    exit(-3);
    case 0 :
    fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
    close(descripteurSocket);
    return 0;
    /*  default:
  //    printf("Message reçu du serveur : %s (%d octets)\n\n", messageRecu, lus);*/
  }
}


void detectCommande(char message[LG_MESSAGE], int socket){

//printf("commande : |%s|\n",message );
  if(strcmp(message, "message") == 0 || strcmp(message, "3") == 0){
    sendMessage(socket)   ;
  }
  else if(strcmp(message, "list") == 0 || strcmp(message, "2") == 0){
    list(socket)   ;
  }
  else if(strcmp(message, "exit") == 0 || strcmp(message, "4") == 0){
    deconnection(socket)   ;
  }
  else if(strcmp(message, "login") == 0 || strcmp(message, "1") == 0){
    deconnection(socket)   ;
  }
  else {
    printf("Commande non reconnue\n");
  }
}

int list(int descripteurSocket){
  int ecrits;
  ecrits = write(descripteurSocket, "!list ", strlen("!list ")); // message à TAILLE variable
  switch(ecrits)
  {
    case -1 :
    perror("write");
    close(descripteurSocket);
    exit(-3);
    case 0 :
    fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
    close(descripteurSocket);
    return 0;
    /*  default:
  //    printf("Message reçu du serveur : %s (%d octets)\n\n", messageRecu, lus);*/
  }
}






int deconnection(int descripteurSocket){
  int ecrits;
  ecrits = write(descripteurSocket, "!exit", strlen("!exit")); // message à TAILLE variable
  switch(ecrits)
  {
    case -1 :
    perror("write");
    close(descripteurSocket);
    exit(-3);
    case 0 :
    fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
    close(descripteurSocket);
    return 0;
    /*  default:
  //    printf("Message reçu du serveur : %s (%d octets)\n\n", messageRecu, lus);*/
  }
}

int login(int descripteurSocket){
  char login[LENGHT_LOGIN];
  char message[LG_MESSAGE];
  int ecrits;
  memset(login, 0, LENGHT_LOGIN);
  memset(message, 0,LG_MESSAGE);
  printf("Quel nom voulez vous (le caractère & est interdit): ");
  fgets(login,sizeof(login), stdin);
  login[(strlen(login) )-1]='\0';
  sprintf(message, "!msg ");
  strcat(message, login);

  ecrits = write(descripteurSocket, message, strlen(message)); // message à TAILLE variable
  switch(ecrits)
  {
    case -1 :
    perror("write");
    close(descripteurSocket);
    exit(-3);
    case 0 :
    fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
    close(descripteurSocket);
    return 0;
    /*  default:
  //    printf("Message reçu du serveur : %s (%d octets)\n\n", messageRecu, lus);*/
}
}

void reception(char messageRecu[LG_MESSAGE]){
  if(strncmp(messageRecu,"!msg",4)==0){
    getMessage(messageRecu);
  }
  else if(strncmp(messageRecu,"!list",4)==0){
    getList(messageRecu);
  }

}

void getMessage(char messageRecu[LG_MESSAGE]){
  //printf("CA MARCHE !!!!!\n");
  char *rest = NULL;
  char *mot;
  int cpt=0;
  char contenu[LG_MESSAGE];
  memset(contenu, 0, LG_MESSAGE);
  for (mot = strtok_r(messageRecu, " ", &rest);
  mot != NULL;
  mot = strtok_r(NULL, " ", &rest)) {

    switch(cpt){
      case 0:
      break;
      case 1:{
        printf("Vous avez reçu un message de %s ", mot);
        break;
      }

      case 2:
      {
        if(strcmp(mot, "|") == 0){
          printf("(messgae personnel) : ");
        }
        else if(strcmp(mot, "&") == 0){
          printf("(messgae global) : ");
        }
        break;
      }
      default:
        strcat(strcat(contenu, " "), mot);
        break;
    }

    cpt++;

  }
  printf("%s\n",contenu );
}

void getList(char messageRecu[LG_MESSAGE]){
  //printf("CA MARCHE !!!!!\n");
  char *rest = NULL,*rest2 = NULL;
  char *mot, *mot2;
  int cpt=0;
  char contenu[LG_MESSAGE];
  memset(contenu, 0, LG_MESSAGE);
  for (mot = strtok_r(messageRecu, " ", &rest);
  mot != NULL;
  mot = strtok_r(NULL, " ", &rest)) {

    switch(cpt){
      case 0:
      printf("Voici les utilisateurs connecte(e)s :\n");

      break;
      case 1:{
        for (mot2 = strtok_r(mot, "&", &rest2);
        mot2 != NULL;
        mot2 = strtok_r(NULL, "&", &rest2)) {
          printf("\t - %s\n",mot2);
        }
        break;
      }


    }

    cpt++;

  }
}
