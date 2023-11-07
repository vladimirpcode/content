#include <iostream>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <cstring>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(){
    int raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_socket < 0){
        std::cout << "не удалось создать сокет \n";
        return -1;
    }

    const char interface_name[100] = "eth4";
    ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_ifrn.ifrn_name, interface_name, IFNAMSIZ);
    if (ioctl(raw_socket, SIOGIFINDEX, &ifr) == -1){
        std::cout << "не удалось получить индекс интерфейса\n";
        return -1;
    }
    int interface_index = ifr.ifr_ifru.ifru_ivalue;

    sockaddr_ll s_ll;
    s_ll.sll_ifindex = interface_index;
    s_ll.sll_family = AF_PACKET;

    // пробуем прикрепить сокет к определенному порту (для отправки не обязательно)
    if (bind(raw_socket, (sockaddr *)&s_ll, sizeof(s_ll)) == -1){
        std::cout << "не удалось привязать сокет к порту\n";
    }

    // пробуем включить неразборчивый режим (для отправки не обязательно)
    packet_mreq mreq;
    mreq.mr_ifindex = interface_index;
    mreq.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(raw_socket, SOL_SOCKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1){
        std::cout << "не удалось включить неразборчивый режим\n";
    }
    // у меня это не работает, поэтому я просто использую консольную команду через system():
    using namespace std::string_literals;
    std::string cmd = "ifconfig "s + std::string{interface_name} + " promisc"s;
    system(cmd.c_str());

    //============================================
    constexpr size_t BUFFSIZE = 65536;
    uint8_t *buffer = (uint8_t*)malloc(BUFFSIZE);
    memset(buffer, 0, BUFFSIZE);

    // получение кадра
    sockaddr s;
    int saddr_len = sizeof(s);
    int bytes_received = recvfrom(raw_socket, buffer, BUFFSIZE, 0, &s, (socklen_t *)&saddr_len);
    if (bytes_received > 0){
        printf("%x:%x:%x:%x:%x:%x -> %x:%x:%x:%x:%x:%x\n", buffer[6], buffer[7], buffer[8],
                buffer[9], buffer[10], buffer[11], buffer[0], buffer[1], buffer[2],
                buffer[3], buffer[4], buffer[5]);
    }

    // отправка кадра
    s_ll.sll_family = AF_PACKET;
    s_ll.sll_ifindex = interface_index;
    for (int i = 0; i < 64; ++i){
        buffer[i] = i;
    }
    int send_len = sendto(raw_socket, buffer, 64, 0, (sockaddr *)&s_ll, sizeof(s_ll));
    if (send_len <= 0){
        std::cout << "отправка не удалась\n";
    }

}