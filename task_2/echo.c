/*
 * echo - read and echo text lines until client closes connection
 */
/* $begin echo */
#include "csapp.h"

void echo(int connfd)
{
    // printf("echo error\n");
    int n;
    char buf[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
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
            break;
        }
    }
    stock_to_buf(root, buf);
    FILE *fp = fopen("stock.txt", "w");
    fprintf(fp, "%s", buf);
}

stock *search(stock *current, int id)
{
    // printf("search current->id : %d\n", current->id);
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
    // printf("buy\n");
    int id, count;
    if (sscanf(buf, "buy %d %d\n", &id, &count) == 2)
    {
        stock *s = search(root, id);
        if (s->left_stock >= count)
        {
            P(&s->w);
            s->left_stock -= count;
            V(&s->w);
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
    // printf("sell\n");
    int id, count;
    if (sscanf(buf, "sell %d %d\n", &id, &count) == 2)
    {
        stock *current = root;
        stock *s = search(current, id);
        P(&s->w);
        s->left_stock += count;
        V(&s->w);
        strcpy(buf, "[sell] success\n");
        Rio_writen(connfd, buf, MAXLINE);
    }
}



void stock_to_buf(stock *current, char *buf)
{
    if (current != NULL)
    {
        // printf("show current->id : %d\n", current->id);
        char temp[100];
        P(&current->mutex);
        current -> readcnt++;
        if (current -> readcnt == 1)
            P(&current->w);
        V(&current->mutex);

        sprintf(temp, "%d %d %d\n", current->id, current->left_stock, current->price);
        strcat(buf, temp);

        P(&current->mutex);
        current -> readcnt--;
        if (current -> readcnt == 0)
            V(&current->w);
        V(&current->mutex);
        stock_to_buf(current->left, buf);
        stock_to_buf(current->right, buf);
    }
}

void show(int connfd, char *buf)
{
    // printf("show\n");
    buf[0] = '\0';
    stock_to_buf(root, buf);
    Rio_writen(connfd, buf, MAXLINE);
}

/* $end echo */
