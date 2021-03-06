#include <sys/epoll.h>

#include <stdint.h>
#include <stdlib.h>

#include "cgi.h"
#include "factory/cgi_factory.h"
#include "async/cgi_async.h"
#include "utils/cgi_dltrie.h"

void *cgi_factory_create(CGI_OBJECT item)
{
    void *object = NULL;
    cgi_http_connection_t *connection = NULL;
    cgi_url_dltrie_t *url_trie = NULL;
    cgi_event_dispatcher_t *dispatcher = NULL;
    async_p async = NULL;

    switch (item) {
        case HTTP_CONNECTION:
            connection = (cgi_http_connection_t *)
                         malloc(sizeof(cgi_http_connection_t));
            connection->rbuffer = (char *)
                                  malloc(sizeof(char) * CGI_HTTP_CONNECTION_READ_BUFFER_SIZE);
            connection->wbuffer = (char *)
                                  malloc(sizeof(char) * CGI_HTTP_CONNECTION_WRITE_BUFFER_SIZE);
            connection->rsize = CGI_HTTP_CONNECTION_READ_BUFFER_SIZE;
            connection->wsize = CGI_HTTP_CONNECTION_WRITE_BUFFER_SIZE;
            object = connection;
            break;

        case PARAM_SLIST:
            object = malloc(sizeof(cgi_pslist_t));
            break;

        case URL_DLTRIE:
            url_trie = (cgi_url_dltrie_t *) malloc(sizeof(cgi_url_dltrie_t));
            url_trie->key = (char *)
                            malloc(sizeof(char) * CGI_URL_DLTRIE_KEY_SIZE);
            url_trie->ksize = 0;
            CGI_DLTRIE_CHILD(url_trie, linker) = NULL;
            CGI_DLTRIE_SIBLING(url_trie, linker) = NULL;
            object = url_trie;
            break;

        case EVENT_DISPATCHER:
            dispatcher = (cgi_event_dispatcher_t *)
                         malloc(sizeof(cgi_event_dispatcher_t));
            dispatcher->events = (struct epoll_event *)
                                 malloc(sizeof(struct epoll_event) * CGI_EVENT_SIZE);
            dispatcher->evsize = CGI_EVENT_SIZE;
            dispatcher->connections =
                cgi_factory_create_vector(HTTP_CONNECTION,
                                          CGI_CONNECTION_SIZE);
            dispatcher->isconn = malloc(sizeof(char) * CGI_CONNECTION_SIZE);
            dispatcher->csize = CGI_CONNECTION_SIZE;
            object = dispatcher;
            break;

        case ASYNC:
            async = Async.create(CGI_THREAD_POOL_SIZE);
            object = async;
            break;

        default:
            break;
    }

    return object;
}

void* cgi_factory_create_vector(CGI_OBJECT item,uint32_t size)
{
    void *objects = NULL;
    cgi_http_connection_t *connections = NULL;
    switch (item) {
        case HTTP_CONNECTION:
            connections = (cgi_http_connection_t *)
                          malloc(sizeof(cgi_http_connection_t) * size);
            cgi_http_connection_t *connection = NULL;
            for (int i = 0; i < size; ++i) {
                connection = connections + i;
                connection->rbuffer = (char *)
                                      malloc(sizeof(char) *
                                             CGI_HTTP_CONNECTION_READ_BUFFER_SIZE);
                connection->wbuffer = (char *)
                                      malloc(sizeof(char) *
                                             CGI_HTTP_CONNECTION_WRITE_BUFFER_SIZE);
                connection->rsize = CGI_HTTP_CONNECTION_READ_BUFFER_SIZE;
                connection->wsize = CGI_HTTP_CONNECTION_WRITE_BUFFER_SIZE;
            }
            objects = connections;
            break;

        default:
            break;
    }
    return objects;
}

void cgi_factory_destroy(void *object,CGI_OBJECT item)
{
    cgi_http_connection_t *connection = NULL;
    cgi_url_dltrie_t *url_trie = NULL;
    cgi_event_dispatcher_t *dispatcher = NULL;

    switch (item) {
        case HTTP_CONNECTION:
            connection = (cgi_http_connection_t *) object;
            free(connection->rbuffer);
            free(connection->wbuffer);
            break;

        case URL_DLTRIE:
            url_trie = (cgi_url_dltrie_t *) object;
            free(url_trie->key);
            break;

        case EVENT_DISPATCHER:
            dispatcher = (cgi_event_dispatcher_t *) object;
            free(dispatcher->events);
            cgi_factory_destroy_vector(dispatcher->connections,
                                       HTTP_CONNECTION);
            free(dispatcher->isconn);
            break;

        default:
            break;
    }
    free(object);
}

void cgi_factory_destroy_vector(void *objects, CGI_OBJECT item)
{
    cgi_http_connection_t *connections = NULL;
    switch (item) {
        case HTTP_CONNECTION:
            connections = objects;
            for (int i = 0; i < CGI_CONNECTION_SIZE; ++i) {
                free(connections[i].rbuffer);
                free(connections[i].wbuffer);
            }
            free(connections);
            break;

        default:
            break;
    }
}
