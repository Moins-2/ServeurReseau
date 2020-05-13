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
#define LG_MESSAGE   256
#define LENGHT_LOGIN 50

typedef struct User {
  int socketClient;
  char login[LENGHT_LOGIN];
}user;

void initName(user users[MAX_USERS], int i);
void detectCommande(char messageRecu[LG_MESSAGE]);


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

    /*  for(int i =0; i<LENGHT_LOGIN; i++){
    login_tampon[i]="";
  }*/
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
  printf("avant le nevent\n");
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

              /*
              login par default
              */
              initName(users, i);




              break;
            }
          }
        }
        else {
          printf("else\n");
          for(int i = 0; i < MAX_USERS; i++) {
            if(pollfds[u].fd == users[i].socketClient) {
              lus = read(users[i].socketClient, messageRecu, LG_MESSAGE*sizeof(char));

              //fct detectCommande

              detectCommande( messageRecu);

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
      printf("login = %s\n", login_tampon);
      strcpy(users[i].login,login_tampon);
      break;
    }
    login_test=1;
  }
}


void detectCommande(char messageRecu[LG_MESSAGE]){

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

    strcpy(messageCopy,messageRecu);
    for (mot = strtok_r(messageCopy, " ", &rest);
    mot != NULL;
    mot = strtok_r(NULL, " ", &rest)) {
      if(cpt==0){
        printf("commande : %s\n", mot);

        if(strcmp(mot, "!message") == 0){
          mode = 1;
        }
        else {
          printf("Commande non reconnue\n");
        }

      }
      else if(cpt==1){
        if(mode==1){
          printf("destinataire : %s\n", mot);
        }
      }
      else if(cpt==2){
        if(mode==1){
          strcat(contenu, mot);
        }
      }
      else {
        if(mode==1){
          strcat(strcat(contenu, " "), mot);
        }
      }

      cpt++;

    }
    if(mode==1){
      printf("message : |%s|", contenu);

      printf("\n Fin du message \n\n");
    }
    cpt=0;

  }

  j=-2;
}


