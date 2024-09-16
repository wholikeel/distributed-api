#include <iostream>
#include <node.hh>
#include <variant>
#include <bit>
#include <cstring>

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <csignal>


using in_addr_f = std::variant<struct in_addr *, struct in6_addr *, std::monostate>;


constexpr auto get_in_addr(struct sockaddr *sockaddr) -> in_addr_f {
    switch (sockaddr->sa_family) {
        case AF_INET:
            return &std::bit_cast<sockaddr_in *>(sockaddr)->sin_addr;
        case AF_INET6:
            return &std::bit_cast<sockaddr_in6 *>(sockaddr)->sin6_addr;
        default:
           return std::monostate(); 
    }
}


auto sigchld_handler(int s) {
    int saved_errno = errno;
    while (waitpid(-1, nullptr, WNOHANG) > 0) {}
    errno = saved_errno;
}

auto main() -> int {
    int sockfd = 0;
    int new_fd = 0;
    struct addrinfo hints{};
    struct addrinfo *servinfo = nullptr;
    struct addrinfo *p = nullptr;
    struct sockaddr_storage their_addr{};
    socklen_t sin_size = 0;

    struct sigaction sig_action{};

    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(nullptr, "3490", &hints, &servinfo)) != 0) {
        std::cout << "getaddrinfo: " << gai_strerror(rv) << '\n';
        return 1;
    }

    for (p = servinfo; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            return 1;
        }
        if (setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            return 1;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    
    freeaddrinfo(servinfo);

    if (p == nullptr) {
        std::cout << "server: failed to bind\n";
        return 1;
    }

    if (listen(sockfd, 10) == -1) {
        perror("listen");
        return 1;
    }
    sig_action.sa_handler = sigchld_handler;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sig_action, nullptr) == -1) {
        perror("sigaction");
        return 1;
    }

    std::cout << "server: waiting for connections...\n"; 

    while (true) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, std::bit_cast<struct sockaddr *>(&their_addr), &sin_size);
        if (new_fd == -1 ) {
            perror("accept");
            continue;
        }
        auto addr_v = get_in_addr(std::bit_cast<struct sockaddr *>(&their_addr));
        auto *addr_4 = std::get<struct in_addr *>(addr_v);
        inet_ntop(their_addr.ss_family, addr_4, s, sizeof(s));
        std::cout << "server: got connection from " << s << '\n';

        if (!fork()) {
            close(sockfd);
            if (send(new_fd, "Hello, world!\n", 14, 0) == -1) {
                perror("send");
            }
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }

    return 0;
}

