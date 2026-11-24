#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <common/mavlink.h>
#include <sys/socket.h>

constexpr const char *SITL_IP = "127.0.0.1";
constexpr int SITL_PORT = 5762;

int main()
{
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0)
  {
    std::perror("socket failed");
    return 1;
  }

  sockaddr_in serv_addr{};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SITL_PORT);
  if (inet_pton(AF_INET, SITL_IP, &serv_addr.sin_addr) <= 0)
  {
    std::cerr << "Invalid address\n";
    close(sock_fd);
    return 1;
  }

  if (connect(sock_fd, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0)
  {
    std::perror("connect failed");
    close(sock_fd);
    return 1;
  }

  uint8_t read_buf[512];
  mavlink_message_t msg;
  mavlink_status_t status;

  while (true)
  {
    ssize_t bytes_read = read(sock_fd, read_buf, sizeof(read_buf));
    if (bytes_read <= 0)
    {
      break;
    }

    for (ssize_t i = 0; i < bytes_read; ++i)
    {
      if (mavlink_parse_char(MAVLINK_COMM_0, read_buf[i], &msg, &status))
      {
        if (msg.msgid == MAVLINK_MSG_ID_HEARTBEAT)
        {
          mavlink_heartbeat_t hb;
          mavlink_msg_heartbeat_decode(&msg, &hb);
          std::cout << "HEARTBEAT sysid=" << static_cast<int>(msg.sysid)
                    << " mode=" << static_cast<int>(hb.base_mode)
                    << " status=" << static_cast<int>(hb.system_status) << "\n";
          close(sock_fd);
          return 0;
        }
      }
    }
  }

  close(sock_fd);
  return 1;
}