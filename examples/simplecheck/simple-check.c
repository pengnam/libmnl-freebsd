/* This example is placed in the public domain. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <strings.h>

#include <libmnl/libmnl.h>
#include <linux/genetlink.h>


int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct genlmsghdr *genl;
	int ret;
	unsigned int seq, portid;

	if (argc > 2) {
		printf("%s [family name]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type	= NLMSG_MIN_TYPE;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);

	/*
	genl = mnl_nlmsg_put_extra_header(nlh, sizeof(struct genlmsghdr));
	genl->cmd = CTRL_CMD_GETFAMILY;
	genl->version = 1;
	mnl_attr_put_u32(nlh, CTRL_ATTR_FAMILY_ID, GENL_ID_CTRL);
	if (argc >= 2)
		mnl_attr_put_strz(nlh, CTRL_ATTR_FAMILY_NAME, argv[1]);
	else
		nlh->nlmsg_flags |= NLM_F_DUMP;

	*/
	nl = mnl_socket_open(NETLINK_GENERIC);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}
	printf("opened\n");
	fflush(stdout);

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}
	portid = mnl_socket_get_portid(nl);
	printf("binded\n");
	fflush(stdout);
	printf("SENDING\n");
	mnl_nlmsg_fprintf(stdout, buf, sizeof(struct nlmsghdr), 0);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		perror("mnl_socket_sendto");
		exit(EXIT_FAILURE);
	}
	printf("sended\n");
	fflush(stdout);
	bzero(buf, MNL_SOCKET_BUFFER_SIZE);

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		printf("RECEIVED\n");
		mnl_nlmsg_fprintf(stdout, buf, ret, 0);
		if (ret < sizeof(struct nlmsghdr)) {
			printf("too small\n");
			exit(EXIT_FAILURE);
		}
		printf("WITHIN\n");
		struct nlmsgerr* content = buf + sizeof(struct nlmsghdr);
		printf("ERROR RETURNED: %d\n", content->error);

		mnl_nlmsg_fprintf(stdout, (void*) &(content->msg) , ret, 0);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		perror("error");
		exit(EXIT_FAILURE);
	}
	printf("received");
	fflush(stdout);

	mnl_socket_close(nl);

	return 0;
}
