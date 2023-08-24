/*
 * echo - read and echo text lines until client closes connection
 */
/* $begin echo */
#include "csapp.h"
void check_clients(pool *p)
{
    // printf("check_client error\n");
    int i, connfd, n;
    char buf[MAXLINE];
    rio_t rio;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++)
    {
        connfd = p->clientfd[i];
        rio = p->clientrio[i];

        /* If the descriptor is ready, echo a text line from it */
        if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set)))
        {
            p->nready--;
            if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
            {
                printf("Server received %d bytes\n", n);
                if (buf[0] == 's' && buf[1] == 'h')
                {
                    show(connfd, buf);
                }

                else if (buf[0] == 'b' && buf[1] == 'u')
                {
                    buy(connfd, buf);
                }

                else if (buf[0] == 's' && buf[1] == 'e')
                {
                    sell(connfd, buf);
                }

                else if (buf[0] == 'e' && buf[1] == 'x')
                {
                    Close(connfd);
                    FD_CLR(connfd, &p->read_set);
                    p->clientfd[i] = -1;
                }
            }

            /* EOF detected, remove descriptor from pool */
            else
            {
                Close(connfd);
                FD_CLR(connfd, &p->read_set);
                p->clientfd[i] = -1;
            }
        }
    }
}

stock *search(stock *current, int id)
{
    // printf("search error\n");
    if (current == NULL)
    { // 값을 찾지 못한 경우
        return NULL;
    }

    if (id == current->id)
    { // 값을 찾음
        return current;
    }
    else if (id < current->id)
    { // 왼쪽 서브트리 탐색
        return search(current->left, id);
    }
    else
    { // 오른쪽 서브트리 탐색
        return search(current->right, id);
    }
}

void buy(int connfd, char *buf)
{
    // printf("buy error\n");
    int id, count;
    if (sscanf(buf, "buy %d %d\n", &id, &count) == 2)
    {
        stock *current = root;
        stock *s = search(current, id);
        if (s->left_stock >= count)
        {
            s->left_stock -= count;
            strcpy(buf, "[buy] success\n");
        }
        else
        {
            strcpy(buf, "Not enough left stock\n");
        }
        Rio_writen(connfd, buf, MAXLINE);
    }
}

void sell(int connfd, char *buf)
{
    // printf("sell error\n");
    int id, count;
    if (sscanf(buf, "sell %d %d\n", &id, &count) == 2)
    {
        stock *current = root;
        stock *s = search(current, id);
        s->left_stock += count;
        strcpy(buf, "[sell] success\n");
        Rio_writen(connfd, buf, MAXLINE);
    }
}

void stock_to_buf(stock *root, char *buf)
{
    // printf("preorder error\n");
    if (root != NULL)
    {
        char temp[100];
        sprintf(temp, "%d %d %d\n", root->id, root->left_stock, root->price);
        strcat(buf, temp);
        // printf("buf\n%s", buf);
        stock_to_buf(root->left, buf);
        stock_to_buf(root->right, buf);
    }
}

void show(int connfd, char *buf)
{
    // printf("show error\n");
    stock *current = root;
    buf[0] = '\0';
    stock_to_buf(current, buf);
    // printf("show buf\n%s", buf);
    Rio_writen(connfd, buf, MAXLINE);
}

/* $end echo */
