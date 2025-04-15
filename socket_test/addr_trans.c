#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

int main(){
    unsigned short p1 = 0x1234;
    printf("%x\n", p1);

    //整数大小端转换
    unsigned short p2 = htons(p1);//端口号从小端转换成大端
    printf("%x\n", p2);
    printf("%x\n\n", ntohs(p2));


    //32位网络字节序IP地址和点分十进制的IP地址互相转换 
    struct sockaddr_in addr;
    inet_aton("127.0.0.1", &addr.sin_addr);//将点分十进制转换成32位网络字节序
    printf("addr = %x\n",addr.sin_addr.s_addr);
    printf("addr = %s\n\n",inet_ntoa(addr.sin_addr));//将32位网络字节序转换成点分十进制


    //dbs
    struct hostent* pHost = gethostbyname("www.baidu.com");
    if(pHost == NULL){
        perror("gethostbyname");
    }
    printf("%s", pHost->h_name);//真实主机名
    for(int i = 0; pHost->h_aliases[i] != NULL; i++) {
    printf("aliases = %s\n",pHost->h_aliases[i]);//别名列表
    }
    printf("addrtype = %d\n", pHost->h_addrtype);//地址类型
    printf("addrlength = %d\n", pHost->h_length);//地址长度

    char buf[128] = {0};
    for(int i = 0; pHost->h_addr_list[i] != NULL; i++) {
        memset(buf, 0, sizeof(buf));
        inet_ntop(pHost->h_addrtype, pHost->h_addr_list[i], buf, sizeof(buf));
        printf("addr = %s\n",buf);
    }
    return 0;
}