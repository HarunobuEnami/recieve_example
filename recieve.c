#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#define ID 0

int initialize_can2(struct canfd_frame *frame,struct sockaddr_can *addr,struct ifreq *ifr,const char *ifname,int canid,int frame_bytes);
static void phex(uint8_t* str,int len);
void print_usage(void);
int main(int argc,char *argv[])
{
    struct sockaddr_can addr;
	struct canfd_frame frame;
    struct ifreq ifr;
    int s;
    int nbytes;
    int i;
    unsigned int address_length =CANFD_MTU;
    if(argc != 2)
    {
        print_usage();
    } else {
    s = initialize_can2(&frame,&addr,&ifr,argv[1],ID,-1);
     nbytes = recvfrom(s, &frame, CANFD_MTU,0, (struct sockaddr*) &addr, &address_length);
    printf("recieved %d bytes\n",frame.len);
    for(i=0;i<frame.len; ++i)
    {
        printf("%02x ",frame.data[i]);
    }
    putchar('\n');
    return 0;
    }
    
}

int initialize_can2(struct canfd_frame *frame,struct sockaddr_can *addr,struct ifreq *ifr,const char *ifname,int canid,int frame_bytes) //受信時，frame_bytesは-1にする
{
  int s;
  size_t mtu;
  int enable_canfd = 1;
  //recieve only
     const int dropmonitor_on = 1;
     int nbytes;
     socklen_t len = sizeof(addr);
    can_err_mask_t err_mask = CAN_ERR_MASK;
     struct iovec iov = {
		.iov_base = frame,
	};
  struct msghdr msg = {
		.msg_iov = &iov,
	};
  //______
 strcpy(ifr->ifr_name, ifname);  
ifr->ifr_ifindex = if_nametoindex(ifr->ifr_name);
     if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Error while opening socket");
		exit(-1);
	}
   if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES,
			       &enable_canfd, sizeof(enable_canfd))){
			printf("error when enabling CAN FD support\n");
			exit(1);
		}
 mtu = ifr->ifr_mtu;
 if(frame_bytes!=-1)
{
   if (ioctl(s, SIOCGIFMTU, ifr) < 0) {
			perror("SIOCGIFMTU");
			exit(1);
		}
     mtu = ifr->ifr_mtu;
    	if (mtu != CANFD_MTU) {
			printf("CAN interface is not CAN FD capable - sorry.\n");
			exit(1);
		}
}

 
if(frame_bytes!=-1)
{
    if (ioctl(s, SIOCGIFMTU, ifr) < 0) {
			perror("SIOCGIFMTU");
			exit(1);
		}
		mtu = ifr->ifr_mtu;
    if (mtu != CANFD_MTU) {
			printf("CAN interface is not CAN FD capable - sorry.\n");
			exit(1);
		}

}

  if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask))) {
		perror("setsockopt()");
		exit(1);
	}
   
     if(frame_bytes!=-1)
     {
       setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
        ioctl(s, SIOCGIFINDEX, ifr);
        frame->len = frame_bytes+4+8;
      frame->can_id = canid;
     }
      addr->can_family = AF_CAN;
  addr->can_ifindex = ifr->ifr_ifindex;
  if(frame_bytes==-1)
  {
     if (setsockopt(s, SOL_SOCKET, SO_RXQ_OVFL,
		       &dropmonitor_on, sizeof(dropmonitor_on)) < 0) {
		perror("setsockopt() SO_RXQ_OVFL not supported by your Linux Kernel");
	}

  if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask))) {
		perror("setsockopt()");
		exit(1);
	}
  }
     if(bind(s,(struct sockaddr *)addr, sizeof( *addr)) < 0) {
		perror("Error in socket bind");
		exit(-2);
	}


  return s;
}

void print_usage(void)
{
    fprintf(stderr,"usage : ./recieve (interface name)\n");
    exit(1);
}