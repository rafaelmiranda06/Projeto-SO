#include <time.h>
#include"apiagenda.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<stdbool.h>

#define BUFFER1	512
#define BUFFER2	512

Execucao listaexecucao[1024];
int nexecucao = 0;
bool haTarefas=false;

void mudar_estado(int sinal){
	int pid=-1, estado=-1;
	if(sinal == SIGCHLD){
		pid = waitpid(-1,&estado,0);
		for(int i=0; i!=nexecucao; i++){
			if(listaexecucao[i].pid == pid){
				strcpy(listaexecucao[i].estado,"terminada");
			}
		}
	}
}

void sinais(int sign){}

int main(){
	int fatc = mkfifo("agendaservidor", 0600);
    	if(fatc==-1){
      		perror("Erro ao criar o fifo de entrada...");
       		return -1;
    	}
	
	int fcta = mkfifo("servidoragenda",0600);
	if(fcta==-1){
		perror("Erro ao criar o fifo de saida...");
		return -1;
	}
	int atc = open("agendaservidor",O_RDONLY);
	int cta = open("servidoragenda",O_WRONLY);
	int opc = -1,nbytes,h = 0;
	char * central = malloc(BUFFER2);
	
	if(!fork()){
		int h = 0;
		char* data2;
		char* hora2;
		char accao[BUFFER1], data[BUFFER1], hora[BUFFER1], estado[BUFFER1], id[BUFFER1], argv[20][64];
		
		signal(SIGCHLD,mudar_estado);

		while(1){
			nbytes=read(atc,central,BUFFER2);
			opc = comando(central);
			
			if(opc == 1){//agendar
				int out_fork = -1;
				sscanf(central,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",accao,data,hora,argv[0],argv[1],argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], argv[10], argv[11],argv[12], argv[13], argv[14], argv[15], argv[16], argv[17], argv[18], argv[19]);
				data2=&data[0];
				hora2=&hora[0];

				dup2(cta,1);
								
				int date[3], hour[3], kd, lh, tempo;
				for (kd=0; kd<=2; kd++){
					date[kd]=atoi(strsep(&data2, "-"));
				}
				for (lh=0; lh<=2; lh++){
					hour[lh]=atoi(strsep(&hora2, ":"));
				}
	
				tempo=segundosdata_now(date[0], date[1], date[2], hour[0], hour[1], hour[2]);
				if(tempo==0){
					char c[32] = "Erro na data e/ou hora errado\n";
					write(cta, &c, sizeof(c));
				}else{
					Execucao novaExecucao;
					novaExecucao.id = nexecucao;
					strcpy(novaExecucao.estado,"agendada");
					strcpy(novaExecucao.programa,argv[0]);
					novaExecucao.ano=date[0];
					novaExecucao.mes=date[1];
					novaExecucao.dia=date[2];
					novaExecucao.hora=hour[0];
					novaExecucao.minuto=hour[1];
					novaExecucao.segundo=hour[2];
					listaexecucao[nexecucao]=novaExecucao;
					dadosexecucao(novaExecucao,cta);
					
					char nexec[1];
					sprintf(nexec,"%d",nexecucao);
					char nfile = nexec[0];					

					int file = open(&nfile,O_CREAT|O_WRONLY|O_TRUNC,0666);
					write(file,"ID: ", 4);					
					write(file, &nexec, sizeof(nexec));
					write(file, "\n", 2);
					write(file, "Data: ", 6);
					char ano[4];
					sprintf(ano,"%d", date[0]);
					write(file, &ano, sizeof(ano));
					write(file, "-", 1);
					char mes[2];
					sprintf(mes,"%d", date[1]);
					write(file, &mes, sizeof(mes));
					write(file, "-", 1);
					char dia[2];
					sprintf(dia,"%d", date[2]);
					write(file, &dia, sizeof(dia));
					write(file, "\n", 2);
					write(file, "Hora: ", 6);
					char hor[2];
					sprintf(hor,"%d", hour[0]);
					write(file, &hor, sizeof(hor));
					write(file, ":", 1);
					char minuto[2];
					sprintf(minuto,"%d", hour[1]);
					write(file, &minuto, sizeof(minuto));
					write(file, ":", 1);
					char segundo[2];
					sprintf(segundo,"%d", hour[2]);
					write(file, &segundo, sizeof(segundo));
					write(file, "\n", 2);
					
					if((out_fork = fork())==0){
						signal(SIGALRM,sinais);
						alarm(tempo);		
						pause();
							
						while(strcmp(argv[h],"/0")!=0){
							h++;
						}

						char** task = malloc((h+1)*sizeof(char*));
						int k = 0;

						write(file, "Comando: ", 9);

						while(strcmp(argv[k],"/0")!=0){
							task[k]=argv[k];
							write(file, &argv[k], sizeof(argv[k]));
							write(file, " ", 1);
							k++;
						}
						h = 0;
						write(file, "\n", 2);

						write(file, "Resultado: ", 11);
						dup2(file,1);
						dup2(file,2);
						execv(task[0],task);
						write(file, "\nError: ",9);
						perror("Erro no execv...");
						return -1;
					
					}
					else{
						listaexecucao[nexecucao].pid=out_fork;
						nexecucao++;
						haTarefas=true;
					}
				
				}				
				
			}
			else if(opc == 2){//cancelar
				sscanf(central,"%s %s",accao,id);
				int id_tarefa = atoi(id);
				if(haTarefas){
					for(int i=0; i!=nexecucao; i++){
						if(listaexecucao[i].id==id_tarefa){
							kill(listaexecucao[i].pid,SIGTERM);
						}
					}
				}else{
					write(cta," Primeiro tem de agendar uma tarefa\n",37);					
				}
			}
			else if(opc == 3){//consultar
				sscanf(central,"%s %s",accao,id);
				if(haTarefas){
					if(fork()==0){

						execlp("cat","cat",id,NULL);
						perror("Erro no execlp...");
						return -1;
					}
				}else{
					write(cta," Primeiro tem de agendar uma tarefa\n",37);
				}
			}
			else if(opc == 4){//listar
				sscanf(central,"%s",accao);
				if(haTarefas){
					for(int i=0; i!=nexecucao; i++){
						dadosexecucao(listaexecucao[i],cta);					
					}
				}else{
					write(cta," Primeiro tem de agendar uma tarefa\n",37);
				}
			}else{ 	
				write(cta,"Não e possivel realizar esta accao\n",36);
			}
			
		}
		free(central);
		_exit(0);
	}	
	wait(NULL);
	unlink("agendaservidor");
	unlink("servidoragenda");
	return 0;	
}