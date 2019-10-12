#include<stdio.h>
#include<string.h>
#include<mpi.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


int main(int argc, char const *argv[])
{
	int psn; //nombre de processus.
	int rank,size;
	int clock=0,recepClock,src,dest;
	FILE *fp;
	char buff[100],buff1[100],str[100][100];
	char *ptr,*token;
	char ack;

	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Status st;

	//open file
	fp = fopen(argv[1],"r");
	if (fp==NULL)
	{
		printf("%s\n", "cannot open file");
	}
	//processus 0 est un analyseur syntaxique et responsable d'affichage.
	int i;
	if (rank==0)
	{
		fgets(buff,100,fp);
		psn = atoi(buff);
		printf("il y a %d processus\n", psn);

		while(!feof(fp))
		{
			fgets(buff,100,fp);
			strcpy(buff1,buff);
			ptr=strtok(buff, " ");//strtok is used to tokenize the strings untill a specified delimiter is found.
			i=0;
			
			while(ptr!=NULL)
			{
				strcpy(str[i],&ptr[0]);
				ptr=strtok(NULL," ");
				i++;
			}

			//evenement interne.
			if(!strncmp(str[0],"exec",4))
			{
				src = atoi(str[1]);
				ack='x';
				MPI_Send(&ack,1,MPI_CHAR,src,1,MPI_COMM_WORLD);
			}
			//evenements d'envoi.
			else if(!strncmp(str[0],"envoi",4))
			{
				src= atoi(str[1]);
				dest= atoi(str[2]);
				ack='s';
				MPI_Send(&ack,1,MPI_CHAR,src,1,MPI_COMM_WORLD);
				MPI_Send(&dest,1,MPI_INT,src,2,MPI_COMM_WORLD);
			}
			else if (!strncmp(str[0],"fin",3))
			{
				ack='t';
				for (int i = 1; i < size; i++)
				{
					MPI_Send(&ack,1,MPI_CHAR,i,1,MPI_COMM_WORLD);
					MPI_Recv(&clock,1,MPI_INT,i,101,MPI_COMM_WORLD,&st);
					printf( "l'heure logique du %d est : %d\n", st.MPI_SOURCE, clock);
				}
			}
		}
	}
	if (rank!=0)
	{
		for(;;)
		{
			MPI_Recv(&ack,1,MPI_CHAR,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&st);

			//Si un evenement interne se produit
			if(ack == 'x')
			{
				clock++;
				printf( "Execution d'evenement interne processus n : %d\n", rank);
				printf("nouvelle heure du %d : %d\n",rank,clock);
			}
			//s'il y a envoie de message.
			else if(ack == 's')
			{
				clock++;
				MPI_Recv(&dest,1,MPI_INT,MPI_ANY_SOURCE,2,MPI_COMM_WORLD,&st);	
				printf("envoi de message du %d vers %d\n", rank, dest);
				printf("nouvelle heure du %d : %d\n",rank,clock);
				ack = 'r';
				MPI_Send(&ack,1,MPI_CHAR,dest,1,MPI_COMM_WORLD);
				MPI_Send(&clock,1,MPI_INT,dest,25,MPI_COMM_WORLD);
			}
			//s'il y a reception du message.
			else if(ack == 'r')
			{
				clock++;
				MPI_Recv(&recepClock,1,MPI_INT,MPI_ANY_SOURCE,25,MPI_COMM_WORLD,&st);
				printf("reception du message du %d a %d\n",st.MPI_SOURCE,rank);
				if(clock<=recepClock)
				{
					clock=recepClock+1;
				}
				printf("nouvelle heure du %d : %d\n",rank,clock);

			}
			//a la fin de l'echange
			else if(ack == 't')
			{
				MPI_Send(&clock,1,MPI_INT,0,101,MPI_COMM_WORLD);
				MPI_Finalize();
				return 0;
			}
		}
	}
	fclose(fp);
	MPI_Finalize();
	return 0;
}