/*
 * echoserveri.c - An iterative echo server
 */
/* $begin echoserverimain */
#include "csapp.h"
void INT_handler(int sig)
{
    exit(0);
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    static pool pool;
    signal(SIGINT, INT_handler);
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    read_stock();
    // listenfd를 통해 60038에서 connection을 듣는다.
    listenfd = Open_listenfd(argv[1]);
    init_pool(listenfd, &pool);

    while (1)
    {
        /* Wait for listening/connected descriptor(s) to become ready */
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);

        /* If listening descriptor ready, add new client to pool */
        if (FD_ISSET(listenfd, &pool.ready_set))
        {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
            printf("Connected to (%s, %s)\n", client_hostname, client_port);
            add_client(connfd, &pool);
        }

        /* Echo a text line from each ready connected descriptor */
        check_clients(&pool);

        if (pool.nready == 0)
        {
            char buf[MAXLINE];
            stock_to_buf(root, buf);
            FILE *fp = fopen("stock.txt", "w");
            fprintf(fp, "%s", buf);
        }
    }
    return (0);
}

void init_pool(int listenfd, pool *p)
{
    // printf("init_pool error\n");
    /* Initially, there are no connected descriptors */
    int i;
    // pool의 가장 큰 index 1로 초기화
    /*maxi는 가장 큰 인덱스를 저장하고 있기 때문에 너무*/
    p->maxi = -1;
    // 연결된 클라이언트 fd가 없기 때문에 모두 -1로 초기화
    for (i = 0; i < FD_SETSIZE; i++)
        p->clientfd[i] = -1;

    /* Initially, listenfd is only member of select read set */
    // listenfd만 있기 때문에
    p->maxfd = listenfd;
    // read_set은 select가 검사할 집합 -> 초기화
    FD_ZERO(&p->read_set);
    // listenfd만 열어놓는다
    FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, pool *p)
{
    // printf("add_client error\n");
    int i;
    p->nready--;
    for (i = 0; i < FD_SETSIZE; i++) /* Find an available slot */
        if (p->clientfd[i] < 0)
        {
            /* Add connected descriptor to the pool */
            p->clientfd[i] = connfd;
            Rio_readinitb(&p->clientrio[i], connfd);

            /* Add the descriptor to descriptor set */
            FD_SET(connfd, &p->read_set);

            /* Update max descriptor and pool high water mark */
            if (connfd > p->maxfd)
                p->maxfd = connfd;
            if (i > p->maxi)
                p->maxi = i;
            break;
        }
    if (i == FD_SETSIZE) /* Couldn’t find an empty slot */
        app_error("add_client error: Too many clients");
}

void insert(stock *new_stock)
{
    // printf("insert error\n");
    stock *x = root;
    stock *y = NULL;

    while (x != NULL)
    {
        y = x;
        if (new_stock->id < x->id)
            x = x->left;
        else
            x = x->right;
    }

    if (y == NULL)
    {
        root = new_stock;
    }
    else if (new_stock->id < y->id)
    {
        y->left = new_stock;
    }
    else
    {
        y->right = new_stock;
    }
}

void read_stock()
{
    // printf("read_stock error\n");
    FILE *fp = fopen("stock.txt", "r");
    while (!feof(fp))
    {
        stock *new_stock = malloc(sizeof(stock));
        if (fscanf(fp, "%d %d %d", &new_stock->id, &new_stock->left_stock, &new_stock->price) != 3)
        {
            free(new_stock);
            break;
        };
        new_stock->left = new_stock->right = NULL;
        insert(new_stock);
    }
    fclose(fp);
}
