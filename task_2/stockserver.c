/*
 * echoserveri.c - An iterative echo server
 */
/* $begin echoserverimain */
#include "csapp.h"

void INT_handler(int sig)
{
    sbuf_deinit(&sbuf);
    exit(0);
}

void insert(stock *new_stock)
{
    // printf("insert \n");
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
    // printf("read_stock \n");
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
        Sem_init(&new_stock->mutex, 0, 1);
        Sem_init(&new_stock->w, 0, 1);
        insert(new_stock);
    }
    fclose(fp);
}
int main(int argc, char **argv)
{
    int i, listenfd;
    int *connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    pthread_t tid;
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    read_stock();
    signal(SIGINT, INT_handler);
    listenfd = Open_listenfd(argv[1]);
    sbuf_init(&sbuf, LISTENQ);
    for (i = 0; i < LISTENQ; i++) /* Create a pool of worker threads */
        Pthread_create(&tid, NULL, thread, NULL);
    while (1)
    {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Malloc(sizeof(int));
        *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        sbuf_insert(&sbuf, connfd); /* Insert connfd in buffer */
    }
    
}

void *thread(void *vargp)
{
    // printf("thread\n");
    Pthread_detach(pthread_self());
    while (1)
    {
        int connfd = sbuf_remove(&sbuf); /* Remove connfd from buf */
        echo(connfd);                    /* Service client */
        Close(connfd);
    }
}

void sbuf_init(sbuf_t *sp, int n)
{
    // printf("sbuf_init\n");
    sp->buf = Calloc(n, sizeof(int));
    sp->n = n;                  /* Buffer holds max of n items */
    sp->front = sp->rear = 0;   /* Empty buffer iff front == rear */
    Sem_init(&sp->mutex, 0, 1); /* Binary semaphore for locking */
    Sem_init(&sp->slots, 0, n); /* Initially, buf has n empty slots */
    Sem_init(&sp->items, 0, 0); /* Initially, buf has 0 items */
}
/* Clean up buffer sp */
void sbuf_deinit(sbuf_t *sp)
{
    // printf("sbuf_deinit\n");
    Free(sp->buf);
}
void sbuf_insert(sbuf_t *sp, int item)
{
    // printf("sbuf_insert\n");
    P(&sp->slots);                          /* Wait for available slot */
    P(&sp->mutex);                          /* Lock the buffer */
    sp->buf[(++sp->rear) % (sp->n)] = item; /* Insert the item */
    V(&sp->mutex);                          /* Unlock the buffer */
    V(&sp->items);                          /* Announce available item */
}
/* Remove and return the first item from buffer sp */
int sbuf_remove(sbuf_t *sp)
{
    int item;
    P(&sp->items); /* Wait for available item */
    P(&sp->mutex); /* Lock the buffer */
    // printf("sbuf_remove\n");
    item = sp->buf[(++sp->front) % (sp->n)]; /* Remove the item */
    V(&sp->mutex);                           /* Unlock the buffer */
    V(&sp->slots);                           /* Announce available slot */
    return item;
}

/* $end echoserverimain */
