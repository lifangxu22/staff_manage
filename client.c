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

#include "common.h"
char *COLUM[12] = {"staffno","usertype","name","passwd",\
			    	"age","phone","addr","work","data",\
			    	"level","salary"};
/**************************************
 *函数名：do_query
 *参   数：消息结构体
 *功   能：登陆
 ****************************************/
void do_admin_query(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);

	msg->msgtype = ADMIN_QUERY;
	
	//发送登陆请求
	//
	while(1)
	{
		int n;
		send(sockfd, msg, sizeof(MSG), 0);
		usleep(1000);
		printf("*************************************************************\n");
		printf("******* 1：按人名查找       	2：查找所有 	     3:退出	**********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
		case 1:
			//char *char_temp[NAMELEN];
			
			printf("请输入您要查询的姓名>>");
			
			scanf("%s",msg->recvmsg);			
			getchar();		
			msg->flags = 1;
			send(sockfd, msg, sizeof(MSG), 0);
			usleep(1000);
			//接收请求
			recv(sockfd, msg, sizeof(MSG), 0);
			if(!strcmp(msg->recvmsg, "error")){
				printf("输入姓名有误,请重新输入\n");
				break;
			}else{
			
				printf("编号\t用户\t姓名   	\t\t年龄\t电话    \t地址\t\t职位	\t\t入职  \t等级\t工资\n");
				printf("%s",msg->recvmsg);
			}
			break;
		case 2:
			msg->flags = 2;
			send(sockfd, msg, sizeof(MSG), 0);
			//usleep(1000);
			//strcpy(msg->recvmsg,"");
			
			printf("编号\t用户\t姓名		\t\t年龄\t电话    \t地址\t\t职位\t\t入职    \t等级\t工资\n");
			recv(sockfd, msg, sizeof(MSG), 0);
			while(strcmp(msg->recvmsg, "end")){
				printf("%s",msg->recvmsg);
				recv(sockfd, msg, sizeof(MSG), 0);
			}
		
			break;
		case 3:
			msg->msgtype = QUIT;
			send(sockfd, msg, sizeof(MSG), 0);
			//usleep(1000);
			close(sockfd);
			exit(0);

			break;
		default:
			printf("您输入有误，请重新输入！\n");
		}
	}	

}	




/**************************************
 *函数名：admin_modification
 *参   数：消息结构体
 *功   能：管理员修改
 ****************************************/
void do_admin_modification(int sockfd,MSG *msg)//管理员修改
{
	msg->msgtype = ADMIN_MODIFY;
	printf("请输入要修改的员工工号>>");
	scanf("%d",&msg->info.no);			
	getchar();	

	printf("*******************请输入要修改的选项***************\n");
	printf("******	1：姓名	   2：密码	3：年龄           4：电话  ******\n");
	printf("******	5：地址	   6：职位 7：入职年月         8：评级  ******\n");
	printf("******	9：工资	   10：退出				     *******\n");
	printf("****************************************************\n");

	do{
	printf("请输入您的选择（数字）>>");
	scanf("%d",&msg->flags);			
	getchar();	
	}while((msg->flags < 1)&&(msg->flags > 10));
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	if(msg->flags != 10){
		printf("输入修改后的信息>>");
		switch(msg->flags){
			case 1:
				scanf("%s",msg->info.name);		break;	
			case 2:
				scanf("%s",msg->info.passwd);	break;
			case 3:
				scanf("%d",&msg->info.age);		break;
			case 4:
				scanf("%s",msg->info.phone);	break;	
			case 5:
				scanf("%s",msg->info.addr);		break;	
			case 6:
				scanf("%s",msg->info.work);		break;	
			case 7:
				scanf("%s",msg->info.date);		break;	
			case 8:
				scanf("%d",&msg->info.level);	break;
			case 9:
				scanf("%lf",&msg->info.salary);	break;
			case 10:
				msg->msgtype = QUIT;
				send(sockfd, msg, sizeof(MSG), 0);
				usleep(1000);
				//close(sockfd);
				//exit(0);			
			default:
			break;
		}
		getchar();		
		printf("------------%s-----------%d.\n",__func__,__LINE__);
		send(sockfd, msg, sizeof(MSG), 0);
		//usleep(1000);
		printf("------------%s-----------%d.\n",__func__,__LINE__);
		recv(sockfd, msg, sizeof(MSG), 0);
		printf("------------%s-----------%d.\n",__func__,__LINE__);
		if(strcmp(msg->recvmsg,"OK")){
			printf(" 员工信息修改失败\n");
		}
		printf("------------%s-----------%d.\n",__func__,__LINE__);
		printf(" 员工信息修改成功\n");
	}
}

/**************************************
 *函数名：admin_adduser
 *参   数：消息结构体
 *功   能：管理员创建用户
 ****************************************/
void do_admin_adduser(int sockfd,MSG *msg)//管理员添加用户
{	

	msg->msgtype = ADMIN_ADDUSER;
	msg->usertype = ADMIN;
	while(1){
		char ch_temp;
		printf("------------%s-----------%d.\n",__func__,__LINE__);
		printf("***************热烈欢迎新员工*****************\n");
		printf("请输入工号>>");
		scanf("%d",&msg->info.no);			
		getchar();	
		printf("您输入的工号是%d\n",msg->info.no);
		printf("请再次确认y/n:");
		scanf("%c",&ch_temp);
		getchar();
		if((ch_temp == 'n')||(ch_temp == 'N'))
			continue;
		
		else if ((ch_temp == 'y')||(ch_temp == 'Y')){
			printf("请输入新员工姓名:");
			scanf("%s",msg->info.name);	
			getchar();	
			printf("请输入登录密码:");
			scanf("%s",msg->info.passwd);	
			getchar();				
			printf("请输入员工年龄:");
			scanf("%d",&msg->info.age);	
			getchar();			
			printf("请输入联系电话:");
			scanf("%s",msg->info.phone);	
			getchar();	
			printf("请输入员工邮寄地址:");
			scanf("%s",msg->info.addr);	
			getchar();	
			printf("请输入员工职位:");
			scanf("%s",msg->info.work);	
			getchar();	
			printf("请输入员工入职时间:");
			scanf("%s",msg->info.date);	
			getchar();	
			printf("请输入员工评级:");
			scanf("%d",&msg->info.level);	
			getchar();	
			printf("请输入员工工资:");
			scanf("%lf",&msg->info.salary);	
			getchar();	
			printf("是否添加为管理员(y/n):");
			
			scanf("%c",&ch_temp);
			getchar();
			if((ch_temp == 'n')||(ch_temp == 'N'))
				msg->info.usertype = 1;
			
			else if ((ch_temp == 'y')||(ch_temp == 'Y')){

				msg->info.usertype = 0;
				}

			send(sockfd, msg, sizeof(MSG), 0);
			//usleep(1000);
			printf("数据添加成功, 是否继续添加:(y/n)");
			scanf("%c",&ch_temp);
			getchar();
			if((ch_temp == 'n')||(ch_temp == 'N'))
				break;
			
			else if ((ch_temp == 'y')||(ch_temp == 'Y')){
				}	
			
		}
	}
}

/**************************************
 *函数名：admin_deluser
 *参   数：消息结构体
 *功   能：管理员删除用户
 ****************************************/
void do_admin_deluser(int sockfd,MSG *msg)//管理员删除用户
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = ADMIN_DELUSER;
	msg->usertype = ADMIN;
	printf("请输入删除的工号>>");
	scanf("%d",&msg->info.no);	
	getchar();		
	printf("请输入删除的员工姓名>>");
	scanf("%s",msg->info.name);	
	getchar();
	send(sockfd, msg, sizeof(MSG), 0);
	//usleep(1000);
	recv(sockfd, msg, sizeof(MSG), 0);
	if(strcmp(msg->recvmsg,"OK")){
		printf("删除用户失败\n");
	}
	printf("删除用户完成\n");
	

	
}

/**************************************
 *函数名：do_history
 *参   数：消息结构体
 *功   能：查看历史记录
 ****************************************/
void do_admin_history (int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = ADMIN_HISTORY;
	//msg->usertype = ADMIN;	
	send(sockfd, msg, sizeof(MSG), 0);
	//usleep(1000);
	printf("时间\t\t\t用户\t\t\t操作\n");
	recv(sockfd, msg, sizeof(MSG), 0);
	while(strcmp(msg->recvmsg, "end")){
		printf("%s",msg->recvmsg);
		recv(sockfd, msg, sizeof(MSG), 0);
	}	
	printf("finish history search\n");
}

/**************************************
 *函数名：admin_menu
 *参   数：套接字、消息结构体
 *功   能：管理员菜单
 ****************************************/
void admin_menu(int sockfd,MSG *msg)
{
	int n;

	while(1)
	{
		printf("*************************************************************\n");
		printf("* 1：查询  2：修改 3：添加用户  4：删除用户  5：查询历史记录*\n");
		printf("* 6：退出													*\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
		case 1:
			do_admin_query(sockfd,msg);
			break;
		case 2:
			do_admin_modification(sockfd,msg);
			break;
		case 3:
			do_admin_adduser(sockfd,msg);
			break;
		case 4:
			do_admin_deluser(sockfd,msg);
			break;
		case 5:
			do_admin_history(sockfd,msg);
			break;
		case 6:
			msg->msgtype = QUIT;
			send(sockfd, msg, sizeof(MSG), 0);
			//usleep(1000);
			close(sockfd);
			exit(0);
		default:
			printf("您输入有误，请重新输入！\n");
		}
	}
}



/**************************************
 *函数名：do_query
 *参   数：消息结构体
 *功   能：登陆
 ****************************************/
void do_user_query(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = USER_QUERY;
	//发送登陆请求
	//

	send(sockfd, msg, sizeof(MSG), 0);
	//usleep(1000);
	//接收请求
	recv(sockfd, msg, sizeof(MSG), 0);
	printf("姓名        \t\t年龄\t电话\t地址\t\t职位              \t入职\t工资\n");
	printf("%s",msg->recvmsg);
}



/**************************************
 *函数名：do_modification
 *参   数：消息结构体
 *功   能：修改
 ****************************************/
void do_user_modification(int sockfd,MSG *msg)
{
	int n;
	printf("------------%s-----------%d.\n",__func__,__LINE__);
		printf("*************************************************************\n");
		printf("* 1：姓名  2：密码 3：年龄  4：电话  5：地址*\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
	
	//printf("修改内容:1:%s 		2:%s 	3:%s 	4:%s 	5:%s",COLUM[2],COLUM[3],COLUM[4],COLUM[5],COLUM[6]);
	scanf("%d",&n);
	getchar();
	msg->msgtype = USER_MODIFY;
	msg->usertype = USER;
	msg->flags = n;
	//send(sockfd,msg,sizeof(MSG),0);
	//msg->flags = 0;
	printf("请输入您要修改的内容：");
	scanf("%s",msg->recvmsg);
	getchar();
	send(sockfd,msg,sizeof(MSG),0);
	//usleep(1000);
	recv(sockfd, msg, sizeof(MSG), 0);
	if(strcmp(msg->recvmsg,"OK")){
		printf("用户信息修改失败\n");
	}
	printf("用户信息修改成功\n");
}


/**************************************
 *函数名：user_menu
 *参   数：消息结构体
 *功   能：管理员菜单
 ****************************************/
void user_menu(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int n;

	while(1)
	{
		printf("*************************************************************\n");
		printf("*************  1：查询  	2：修改		3：退出	 *************\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
		case 1:
			do_user_query(sockfd,msg);
			break;
		case 2:
			do_user_modification(sockfd,msg);
			break;
		case 3:
			msg->msgtype = QUIT;
			send(sockfd, msg, sizeof(MSG), 0);
			//usleep(1000);
			close(sockfd);
			exit(0);
		default:
			printf("您输入有误，请输入数字\n");
			break;
		}
	}
}




int admin_or_user_login(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//输入用户名和密码
	memset(msg->username, 0, NAMELEN);
	printf("请输入用户名：");
	scanf("%s",msg->username);
	getchar();

	memset(msg->passwd, 0, DATALEN);
	printf("请输入密码（6位）");
	scanf("%s",msg->passwd);
	getchar();

	//发送登陆请求
	send(sockfd, msg, sizeof(MSG), 0);
	//usleep(1000);
	//接受服务器响应
	recv(sockfd, msg, sizeof(MSG), 0);
	printf("msg->recvmsg :%s\n",msg->recvmsg);

	//判断是否登陆成功
	if(strncmp(msg->recvmsg, "OK", 2) == 0)
	{
		if(msg->usertype == ADMIN)
		{
			printf("亲爱的管理员，欢迎您登陆员工管理系统！\n");
			admin_menu(sockfd,msg);
		}
		else if(msg->usertype == USER)
		{
			printf("亲爱的用户，欢迎您登陆员工管理系统！\n");
			user_menu(sockfd,msg);
		}
	}
	else
	{
		printf("登陆失败！%s\n", msg->recvmsg);
		return -1;
	}

	return 0;
}


/************************************************
 *函数名：do_login
 *参   数：套接字、消息结构体
 *返回值：是否登陆成功
 *功   能：登陆
 *************************************************/
int do_login(int sockfd)
{	
	int n;
	MSG msg;
	while(1){
		printf("*************************************************************\n");
		printf("********  1：管理员模式    2：普通用户模式    3：退出********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
		case 1:
			msg.msgtype  = ADMIN_LOGIN;
			msg.usertype = ADMIN;
			break;
		case 2:
			msg.msgtype =  USER_LOGIN;
			msg.usertype = USER;
			break;
		case 3:
			msg.msgtype = QUIT;
			if(send(sockfd, &msg, sizeof(MSG), 0)<0)
			{
				perror("do_login send");
				return -1;
			}
			close(sockfd);
			exit(0);
		default:
			printf("您的输入有误，请重新输入\n"); 
		}

		admin_or_user_login(sockfd,&msg);
	}

}


int main(int argc, const char *argv[])
{
	//socket->填充->绑定->监听->等待连接->数据交互->关闭 
	int sockfd;
	int acceptfd;
	ssize_t recvbytes,sendbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	socklen_t cli_len = sizeof(clientaddr);

	//创建网络通信的套接字
	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("socket failed.\n");
		exit(-1);
	}
	printf("sockfd :%d.\n",sockfd); 

	//填充网络结构体
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port   = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	//	serveraddr.sin_port   = htons(5001);
	//	serveraddr.sin_addr.s_addr = inet_addr("192.168.1.200");

	if(connect(sockfd,(const struct sockaddr *)&serveraddr,addrlen) == -1){
		perror("connect failed.\n");
		exit(-1);
	}

	do_login(sockfd);

	close(sockfd);

	return 0;
}



