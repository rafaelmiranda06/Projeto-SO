#include <time.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <math.h>

struct execucao{
	int id;
	char estado[32];
	char programa[32];
	int ano;
	int mes;
	int dia;
	int hora;
	int minuto;
	int segundo;
	int pid;
};
typedef struct execucao Execucao;

int comando(char *accao){	
	char aux[10];
	sscanf(accao,"%s",aux);
	
	if((strncmp(aux,"-a",strlen(aux)))==0){
		return 1; //Para agendar
	}else if((strncmp(aux,"-c",strlen(aux)))==0){
		return 2;//Para cancelar
	}else if((strncmp(aux,"-r",strlen(aux)))==0){
		return 3;//Para consultar
	}else if((strncmp(aux,"-l",strlen(aux)))==0){
		return 4;// Para listar
	}else if((strncmp(aux,"-e",strlen(aux)))==0){
		return 5;// Para enviar o resultado
	}else if((strncmp(aux,"-n",strlen(aux)))==0){
		return 6;// Para definir numero de programas a executar
	}
	return -1;
}

int segundosdata_now(int ano, int mes, int dia, int hora, int min, int sec){
	int seg_data=0, seg_now=0, nsegundos=0, hour, minu, seg;
	time_t tmp = time(NULL);
	struct tm tm = *localtime(&tmp);
	sec=sec%60;
	min=min*60;
	hora=hora*3600;	
	seg_data=hora+min+sec;
	seg=tm.tm_sec%60;
	minu=tm.tm_min*60;
	hour=tm.tm_hour*3600;	
	seg_now=hour+minu+seg;
	if(ano == (tm.tm_year + 1900)){
		if(mes < (tm.tm_mon + 1)){
			seg_data=0;
		}else if(mes == (tm.tm_mon + 1)){
			if(dia < (tm.tm_mday)){
				seg_data=0;
			}else if(dia == (tm.tm_mday)){
				if(seg_data>seg_now){
					nsegundos += (seg_data-seg_now);
				}else{
					seg_data=0;
				}
			}else if(dia > (tm.tm_mday)){
				if(seg_data<seg_now){
					nsegundos += ((24*60*60)-seg_now)+seg_data;
				}else if(seg_data>seg_now){
					nsegundos += (seg_data-seg_now)+(24*60*60);
				}else{
					seg_data=0;
				}
			}
		}
	}else{
		seg_data=0;
	}
	return nsegundos;
}

void dadosexecucao(Execucao t, int fd){	
	char dados[128];
	snprintf(dados,512,"%d %s %s %d-%d-%d %d:%d:%d\n",t.id,t.estado,t.programa,t.ano,t.mes,t.dia,t.hora,t.minuto,t.segundo);
	write(fd,dados,strlen(dados));
}