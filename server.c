/*************************************************************************
#	 FileName	: server.c
#	 Author		: fengjunhui 
#	 Email		: 18883765905@163.com 
#	 Created	: 2018年12月29日 星期六 13时44分59秒
 ************************************************************************/

#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>

#include "common.h"

sqlite3 *db;  //仅服务器使 
char *COLUM[12] = {"staffno","usertype","name","passwd",\
				"age","phone","addr","work","data",\
				"level","salary"};
HIST *hist;


int do_History_records(MSG *msg){
	char sql[DATALEN] = {0};
	char *errmsg;
	time_t timeNow;
	struct tm *t;
	time(&timeNow);
	t = localtime(&timeNow);
	printf("------------%s-----------%d.\n",__func__,__LINE__);

	sprintf(msg->hist.time,"%4d-%02d-%02d-%02d:%02d:%02d",t->tm_year+1900, t->tm_mon, t->tm_mday,\
				 									  t->tm_hour,      t->tm_min, t->tm_sec);
	strcpy(msg->info.name,msg->username);	
	sprintf(sql,"insert into historyinfo values('%s','%s','%s');" ,msg->hist.time,msg->info.name,msg->hist.words);	
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		fprintf(stderr,"sqlite3_exec failed %s\n",sqlite3_errmsg(db));
		return -1;	
	}
	printf("history record\n");
	return 0;
	
}

int process_user_or_admin_login_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//封装sql命令，表中查询用户名和密码－存在－登录成功－发送响应－失败－发送失败响应	
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;


	msg->info.usertype =  msg->usertype;
	strcpy(msg->info.name,msg->username);
	strcpy(msg->info.passwd,msg->passwd);
	
	printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);		
	}else{
		//printf("----nrow-----%d,ncolumn-----%d.\n",nrow,ncolumn);		
		if(nrow == 0){
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
			usleep(1000);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd,msg,sizeof(MSG),0);
			usleep(1000);
			//历史记录
			strcpy(msg->hist.words,"user_or_admin_login");
			do_History_records(msg);

		}
	}
	return 0;	
}

int process_user_modify_request(int acceptfd,MSG *msg)
{
	char sql[DATALEN] = {0};
	char *errmsg = NULL;
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//recv(acceptfd,msg,sizeof(MSG),0);
	int option = msg->flags;
	//recv(acceptfd,msg,sizeof(MSG),0);
	msg->info.usertype =  msg->usertype;
	if(option == 3){

		sprintf(sql,"update usrinfo set %s=%s where %s='%s';",COLUM[option + 1],msg->recvmsg,COLUM[2],msg->username);	
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
			{
				fprintf(stderr,"sqlite3_exec failed %s\n",sqlite3_errmsg(db));
				strcpy(msg->recvmsg,"name or passwd failed.\n");
				send(acceptfd,msg,sizeof(MSG),0);	
				usleep(1000);
				
				return -1;  
			}
	}
	else{
		sprintf(sql,"update usrinfo set %s='%s' where %s='%s';",COLUM[option + 1],msg->recvmsg,COLUM[2],msg->username);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
		{
			fprintf(stderr,"sqlite3_exec failed %s\n",sqlite3_errmsg(db));
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);	
			usleep(1000);
			return -1;	
		}
	}
	//历史记录
	sprintf(msg->hist.words,"user %s modify %s to %s",msg->username,COLUM[option +1],msg->recvmsg);
	do_History_records(msg);	
	
	option = 0;
	strcpy(msg->recvmsg,"OK");
	send(acceptfd,msg,sizeof(MSG),0);	
	usleep(1000);

	//printf("modify finished \n"); 	
	return 0;
}



int process_user_query_request(int acceptfd,MSG *msg)
{
	char sql[DATALEN];
	ssize_t recvbytes;
	char recvbuf[DATALEN] = {0};
	//sqlite3_get_table 返回结果初始化

	char *errmsg;
	char **result;
	int nrow,ncolumn;

	printf("------------%s-----------%d.\n",__func__,__LINE__);
	sprintf(sql,"select * from usrinfo where name='%s';",msg->info.name);

	//printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	//sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);		
	}else{
		//printf("----nrow-----%d,ncolumn-----%d.\n",nrow,ncolumn);		
		if(nrow == 0){
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
			usleep(1000);
		}else{
			//strcpy(msg->recvmsg,"OK");
			sprintf(msg->recvmsg,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t\n"
				 ,result[0+11]
				 ,result[1+11]
				 ,result[2+11]
				 ,result[4+11]
			  	 ,result[5+11]
			 	 ,result[6+11]
				 ,result[7+11]
				 ,result[8+11]
				 ,result[9+11]
				 ,result[10+11]);	
			
			send(acceptfd,msg,sizeof(MSG),0);
			usleep(1000);

		}
	}

	return 0;	

}


int process_admin_modify_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	
	char sql[DATALEN] = {0};
	char *errmsg = NULL;
	
	//recv(acceptfd,msg,sizeof(MSG),0);
	int option = msg->flags;
	switch(option){
		case 1:
			sprintf(sql,"update usrinfo set %s='%s' where %s=%d;",COLUM[option + 1],msg->info.name,COLUM[0],msg->info.no);
			sprintf(msg->hist.words,"user %s modify %s to %s",msg->username,COLUM[option +1],msg->info.name);
		break;
		case 2:
			sprintf(sql,"update usrinfo set %s='%s' where %s=%d;",COLUM[option + 1],msg->info.passwd,COLUM[0],msg->info.no);
			sprintf(msg->hist.words,"user %s modify %s to %s",msg->username,COLUM[option +1],msg->info.passwd);
		break;
		case 3:		
			sprintf(sql,"update usrinfo set %s=%d where %s=%d;",COLUM[option + 1],msg->info.age,COLUM[0],msg->info.no);
			sprintf(msg->hist.words,"user %s modify %s to %d",msg->username,COLUM[option +1],msg->info.age);
		break;
		case 4:
			sprintf(sql,"update usrinfo set %s='%s' where %s=%d;",COLUM[option + 1],msg->info.phone,COLUM[0],msg->info.no);
			sprintf(msg->hist.words,"user %s modify %s to %s",msg->username,COLUM[option +1],msg->info.phone);
		break;
		case 5:
			sprintf(sql,"update usrinfo set %s='%s' where %s=%d;",COLUM[option + 1],msg->info.addr,COLUM[0],msg->info.no);
			sprintf(msg->hist.words,"user %s modify %s to %s",msg->username,COLUM[option +1],msg->info.addr);
		break;
		case 6:
			sprintf(sql,"update usrinfo set %s='%s' where %s=%d;",COLUM[option + 1],msg->info.work,COLUM[0],msg->info.no);
			sprintf(msg->hist.words,"user %s modify %s to %s",msg->username,COLUM[option +1],msg->info.work);
		break;
		case 7:
			sprintf(sql,"update usrinfo set %s='%s' where %s=%d;",COLUM[option + 1],msg->info.date,COLUM[0],msg->info.no);
			sprintf(msg->hist.words,"user %s modify %s to %s",msg->username,COLUM[option +1],msg->info.date);
		break;
		case 8:
			sprintf(sql,"update usrinfo set %s=%d where %s=%d;",COLUM[option + 1],msg->info.level,COLUM[0],msg->info.no);
			sprintf(msg->hist.words,"user %s modify %s to %d",msg->username,COLUM[option +1],msg->info.no);
		break;	
		case 9:
			sprintf(sql,"update usrinfo set %s=%lf where %s=%d;",COLUM[option + 1],msg->info.salary,COLUM[0],msg->info.no);
			sprintf(msg->hist.words,"user %s modify %s to %lf",msg->username,COLUM[option +1],msg->info.salary);
		break;		
		default:
			break;
		//}else if (option != 10){
			//sprintf(sql,"update usrinfo set %s='%s' where %s=%d;",COLUM[option + 1],msg->recvmsg,COLUM[0],msg->info.no);
		//	}
	}
		//历史记录
		//sprintf(msg->hist.words,"user %s modify %s to %lf",msg->username,COLUM[option +1],msg->info.salary);
		do_History_records(msg);	
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		fprintf(stderr,"sqlite3_exec failed %s\n",sqlite3_errmsg(db));
		strcpy(msg->recvmsg,"ERROR");
		send(acceptfd,msg,sizeof(MSG),0);
		usleep(1000);
		return -1;	
	}


	strcpy(msg->recvmsg,"OK");
	send(acceptfd,msg,sizeof(MSG),0);
	usleep(1000);

	option = 0;
	//printf("modify finished \n"); 	
	return 0;	
}


int process_admin_adduser_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char *errmsg = NULL;
	char sql[DATALEN] = {0};
	recv(acceptfd,msg,sizeof(MSG),0);

	//插入整条数据
	
	sprintf(sql,"insert into usrinfo values(%d,%d,'%s','%s',%d,'%s','%s','%s','%s',%d,%lf);", 
										   	msg->info.no,
											msg->info.usertype,
											msg->info.name,
											msg->info.passwd,
											msg->info.age,
											msg->info.phone,
											msg->info.addr,
											msg->info.work,
											msg->info.date,
											msg->info.level,
										   	msg->info.salary);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		fprintf(stderr,"sqlite3_exec failed %s\n",sqlite3_errmsg(db));
		return -1;	
	}

	//历史记录
	sprintf(msg->hist.words,"admin %s adduser %s to usrinfo",msg->username,msg->info.name);
	do_History_records(msg);	

	
	printf("add finished\n");
		//sprintf(sql,"update usrinfo set %s=%s where %s='%s';",COLUM[option + 1],msg->recvmsg,COLUM[2],msg->username);

	return 0;	
	
}

int process_admin_deluser_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN];
	char *errmsg;

	//删除整条数据
	sprintf(sql,"delete from usrinfo where staffno=%d and name='%s';",msg->info.no,msg->info.name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		strcpy(msg->recvmsg,"ERROR");
		fprintf(stderr,"sqlite3_exec failed %s\n",sqlite3_errmsg(db));
		return -1;	
	}
	//历史记录
	sprintf(msg->hist.words,"admin %s delete user:%s no:%d",msg->username,msg->info.name,msg->info.no);
	do_History_records(msg);

	
	strcpy(msg->recvmsg,"OK");
	send(acceptfd,msg,sizeof(MSG),0);
	usleep(1000);
	
}

int process_admin_query_request(int acceptfd,MSG *msg)
{
	

	char sql[DATALEN];
	ssize_t recvbytes;
	char recvbuf[DATALEN] = {0};
	//sqlite3_get_table 返回结果初始化
	char *errmsg;
	char **result;
	int nrow,ncolumn;
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	recv(acceptfd,msg,sizeof(MSG),0);
	//printf("msg->flags: %d\n",msg->flags);
	switch(msg->flags)
	{
	
		case 1://姓名搜索
			
			sprintf(sql,"select * from usrinfo where name='%s';",msg->recvmsg);
			
			//printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
			//sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
			if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
				printf("---****----%s.\n",errmsg);		
			}else{
				//printf("----nrow-----%d,ncolumn-----%d.\n",nrow,ncolumn);		
				if(nrow == 0){
					strcpy(msg->recvmsg,"error");
					send(acceptfd,msg,sizeof(MSG),0);
					usleep(1000);
				}else{
				sprintf(msg->recvmsg,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t\n"
					, result[0+11]
					, result[1+11]
					 ,result[2+11]
					, result[4+11]
				  	 ,result[5+11]
				 	 ,result[6+11]
					 ,result[7+11]
					 ,result[8+11]
					, result[9+11]
					, result[10+11]);		
					send(acceptfd,msg,sizeof(MSG),0);
					usleep(1000);
				}
				
			}
			break;

			
		
		case 2:
		
			sprintf(sql,"select * from usrinfo;");
			
			//printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
			//sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
			if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
				printf("---****----%s.\n",errmsg);		
			}else{

				if(nrow == 0){
					strcpy(msg->recvmsg,"name or passwd failed.\n");
					send(acceptfd,msg,sizeof(MSG),0);
					usleep(1000);
				}else{
					int i = 1;
					//strcpy(msg->recvmsg,"OK");

					for(i = 1; i <= nrow; i++)
					{

						sprintf(msg->recvmsg,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t\n"
							 ,result[0+11*i]
							 ,result[1+11*i]
							 ,result[2+11*i]
							 ,result[4+11*i]
							 ,result[5+11*i]
							 ,result[6+11*i]
							 ,result[7+11*i]
							 ,result[8+11*i]
							 ,result[9+11*i]
							 ,result[10+11*i]);	

						send(acceptfd,msg,sizeof(MSG),0);
						usleep(1000);
					}
					strcpy(msg->recvmsg, "end");

					send(acceptfd,msg,sizeof(MSG),0);
					usleep(1000);
					printf("finish sending all the staffs\n");

				}
				
			}

		break;
		default:
			break;
	}
	return 0;	

}

int process_admin_history_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);

	char sql[DATALEN];
	ssize_t recvbytes;
	char recvbuf[DATALEN] = {0};
	//sqlite3_get_table 返回结果初始化
	char *errmsg;
	char **result;
	int nrow,ncolumn;
	
	sprintf(sql,"select * from historyinfo order by time desc limit %d;",History_limit);
	
	//printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	//sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);		
	}else{

		if(nrow == 0){
			strcpy(msg->recvmsg,"failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
			usleep(1000);
		}else{
			int i = 1;
			//strcpy(msg->recvmsg,"OK");

			for(i = 1; i <= nrow; i++)
			{	
				sprintf(msg->recvmsg,"%s\t%s\t%s\n"
					 ,result[0+3*i]
					 ,result[1+3*i]
					 ,result[2+3*i]); 
	
				send(acceptfd,msg,sizeof(MSG),0);
				usleep(1000);
			}
			strcpy(msg->recvmsg, "end");
			send(acceptfd,msg,sizeof(MSG),0);
			usleep(1000);
			printf("finish sending all the historyinfo\n");
	
		}
		
	}


}


int process_client_quit_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	
	//历史记录
	sprintf(msg->hist.words,"client %s loged out",msg->info.name);
	do_History_records(msg);

}


int process_client_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	switch (msg->msgtype)
	{
		case USER_LOGIN:
		case ADMIN_LOGIN:
			process_user_or_admin_login_request(acceptfd,msg);
			break;
		case USER_MODIFY:
			process_user_modify_request(acceptfd,msg);
			break;
		case USER_QUERY:
			process_user_query_request(acceptfd,msg);
			break;
		case ADMIN_MODIFY:
			process_admin_modify_request(acceptfd,msg);
			break;
		case ADMIN_ADDUSER:
			process_admin_adduser_request(acceptfd,msg);
			break;
		case ADMIN_DELUSER:
			process_admin_deluser_request(acceptfd,msg);
			break;
		case ADMIN_QUERY:
			process_admin_query_request(acceptfd,msg);
			break;
		case ADMIN_HISTORY:
			process_admin_history_request(acceptfd,msg);
			break;
		case QUIT:
			process_client_quit_request(acceptfd,msg);
			break;
		default:
			break;
	}

}


int main(int argc, const char *argv[])
{
	//socket->填充->绑定->监听->等待连接->数据交互->关闭 
	int sockfd;
	int acceptfd;
	ssize_t recvbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	socklen_t cli_len = sizeof(clientaddr);

	MSG msg;
	//thread_data_t tid_data;
	char *errmsg;

	if(sqlite3_open(STAFF_DATABASE,&db) != SQLITE_OK){
		printf("%s.\n",sqlite3_errmsg(db));
	}else{
		printf("the database open success.\n");
	}

	if(sqlite3_exec(db,"create table usrinfo(staffno integer,usertype integer,name text,passwd text,age integer,phone text,addr text,work text,date text,level integer,salary REAL);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("create usrinfo table success.\n");
	}

	if(sqlite3_exec(db,"create table historyinfo(time text,name text,words text);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{ //华清远见创客学院         嵌入式物联网方向讲师
		printf("create historyinfo table success.\n");
	}

	//创建网络通信的套接字
	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("socket failed.\n");
		exit(-1);
	}
	printf("sockfd :%d.\n",sockfd); 

	
	/*优化4： 允许绑定地址快速重用 */
	int b_reuse = 1;
	setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof (int));
	
	//填充网络结构体
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port   = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
//	serveraddr.sin_port   = htons(5001);
//	serveraddr.sin_addr.s_addr = inet_addr("192.168.1.200");


	//绑定网络套接字和网络结构体
	if(bind(sockfd, (const struct sockaddr *)&serveraddr,addrlen) == -1){
		printf("bind failed.\n");
		exit(-1);
	}

	//监听套接字，将主动套接字转化为被动套接字
	if(listen(sockfd,10) == -1){
		printf("listen failed.\n");
		exit(-1);
	}

	//定义一张表
	fd_set readfds,tempfds;
	//清空表
	FD_ZERO(&readfds);
	FD_ZERO(&tempfds);
	//添加要监听的事件
	FD_SET(sockfd,&readfds);
	int nfds = sockfd;
	int retval;
	int i = 0;

#if 0 //添加线程控制部分
	pthread_t thread[N];
	int tid = 0;
#endif

	while(1){
		tempfds = readfds;
		//记得重新添加
		retval =select(nfds + 1, &tempfds, NULL,NULL,NULL);
		//判断是否是集合里关注的事件
		for(i = 0;i < nfds + 1; i ++){
			if(FD_ISSET(i,&tempfds)){
				if(i == sockfd){
					//数据交互 
					acceptfd = accept(sockfd,(struct sockaddr *)&clientaddr,&cli_len);
					if(acceptfd == -1){
						printf("acceptfd failed.\n");
						exit(-1);
					}
					printf("ip : %s.\n",inet_ntoa(clientaddr.sin_addr));
					FD_SET(acceptfd,&readfds);
					nfds = nfds > acceptfd ? nfds : acceptfd;
				}else{
					recvbytes = recv(i,&msg,sizeof(msg),0);
					printf("msg.type :%#x.\n",msg.msgtype);
					if(recvbytes == -1){
						printf("recv failed.\n");
						continue;
					}else if(recvbytes == 0){
						printf("peer shutdown.\n");
						close(i);
						FD_CLR(i, &readfds);  //删除集合中的i
					}else{
						process_client_request(i,&msg);
					}
				}
			}
		}
	}
	close(sockfd);

	return 0;
}







#if 0
					//tid_data.acceptfd = acceptfd;   //暂时不使用这种方式
					//tid_data.state	  = 1;
					//tid_data.thread   = thread[tid++];	
					//pthread_create(&tid_data.thread, NULL,client_request_handler,(void *)&tid_data);
#endif 

#if 0
void *client_request_handler(void * args)
{
	thread_data_t *tiddata= (thread_data_t *)args;

	MSG msg;
	int recvbytes;
	printf("tiddata->acceptfd :%d.\n",tiddata->acceptfd);

	while(1){  //可以写到线程里--晚上的作业---- UDP聊天室
		//recv 
		memset(msg,sizeof(msg),0);
		recvbytes = recv(tiddata->acceptfd,&msg,sizeof(msg),0);
		if(recvbytes == -1){
			printf("recv failed.\n");
			close(tiddata->acceptfd);
			pthread_exit(0);
		}else if(recvbytes == 0){
			printf("peer shutdown.\n");
			pthread_exit(0);
		}else{
			printf("msg.recvmsg :%s.\n",msg.recvmsg);
			strcat(buf,"*-*");
			send(tiddata->acceptfd,&msg,sizeof(msg),0);
		}
	}

}

#endif 













