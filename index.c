#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define MAX_CONNECTIONS 10
#define MAX_DEPARTMENTS 100
#define MAX_PATHS 1000
#define MAX_THREADS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct Connection {
    int destination;
    struct Connection *next;
} Connection;

typedef struct Department {
    int id;
    char name[50];
    Connection *connections;
} Department;

typedef struct Graph {
    Department departments[MAX_DEPARTMENTS];
    int numDepartments;
} Graph;

typedef struct Path {
    int nodes[MAX_DEPARTMENTS];
    int length;
} Path;

typedef struct Queue {
    Path paths[MAX_PATHS];
    int front, rear;
} Queue;

void initQueue(Queue *queue) {
    queue->front = 0;
    queue->rear = 0;
}

int isEmpty(Queue *queue) {
    return queue->front == queue->rear;
}

void enqueue(Queue *queue, Path path) {
    if (queue->rear < MAX_PATHS) {
        queue->paths[queue->rear++] = path;
    }
}

Path dequeue(Queue *queue) {
    return queue->paths[queue->front++];
}

void addConnection(Graph *graph, int from, int to) {
    Connection *newConnection = (Connection *)malloc(sizeof(Connection));
    newConnection->destination = to;
    newConnection->next = graph->departments[from].connections;
    graph->departments[from].connections = newConnection;
}

void initGraph(Graph *graph) {
    graph->numDepartments = 0;
}

int addDepartment(Graph *graph, const char *name) {
    int id = graph->numDepartments;
    graph->departments[id].id = id;
    strcpy(graph->departments[id].name, name);
    graph->departments[id].connections = NULL;
    graph->numDepartments++;
    return id;
}

void printPath(Graph *graph, Path path) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < path.length; i++) {
        printf("%s", graph->departments[path.nodes[i]].name);
        if (i < path.length - 1) printf(" -> ");
    }
    printf("\n");
    pthread_mutex_unlock(&mutex);
}

void bfsSequential(Graph *graph, int start, int end) {
    Queue queue;
    initQueue(&queue);

    Path initialPath;
    initialPath.nodes[0] = start;
    initialPath.length = 1;
    enqueue(&queue, initialPath);

    while (!isEmpty(&queue)) {
        Path currentPath = dequeue(&queue);
        int lastNode = currentPath.nodes[currentPath.length - 1];

        if (lastNode == end) {
            printPath(graph, currentPath);
            continue;
        }

        Connection *conn = graph->departments[lastNode].connections;
        while (conn) {
            int alreadyVisited = 0;
            for (int i = 0; i < currentPath.length; i++) {
                if (currentPath.nodes[i] == conn->destination) {
                    alreadyVisited = 1;
                    break;
                }
            }
            if (!alreadyVisited) {
                Path newPath = currentPath;
                newPath.nodes[newPath.length++] = conn->destination;
                enqueue(&queue, newPath);
            }
            conn = conn->next;
        }
    }
}

typedef struct {
    Graph *graph;
    int start;
    int end;
    int initialNode;
} ThreadArgs;

void *threadBFS(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    Graph *graph = args->graph;
    int start = args->start;
    int end = args->end;
    int initialNode = args->initialNode;

    Queue queue;
    initQueue(&queue);

    Path initialPath;
    initialPath.nodes[0] = start;
    initialPath.nodes[1] = initialNode;
    initialPath.length = 2;
    enqueue(&queue, initialPath);

    while (!isEmpty(&queue)) {
        Path currentPath = dequeue(&queue);
        int lastNode = currentPath.nodes[currentPath.length - 1];

        if (lastNode == end) {
            printPath(graph, currentPath);
            continue;
        }

        Connection *conn = graph->departments[lastNode].connections;
        while (conn) {
            int alreadyVisited = 0;
            for (int i = 0; i < currentPath.length; i++) {
                if (currentPath.nodes[i] == conn->destination) {
                    alreadyVisited = 1;
                    break;
                }
            }
            if (!alreadyVisited) {
                Path newPath = currentPath;
                newPath.nodes[newPath.length++] = conn->destination;
                enqueue(&queue, newPath);
            }
            conn = conn->next;
        }
    }
    return NULL;
}

void runSequentialAndParallel(Graph *graph, int start, int end, const char *label) {
    printf("\n--- %s ---\n", label);

    printf("Sequential BFS:\n");
    clock_t startTime = clock();
    bfsSequential(graph, start, end);
    clock_t endTime = clock();
    double seqTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    printf("Time (sequential): %f seconds\n", seqTime);

    printf("Parallel BFS:\n");
    pthread_t threads[MAX_THREADS];
    ThreadArgs args[MAX_THREADS];
    int threadCount = 0;

    Connection *conn = graph->departments[start].connections;
    startTime = clock();
    while (conn && threadCount < MAX_THREADS) {
        args[threadCount].graph = graph;
        args[threadCount].start = start;
        args[threadCount].end = end;
        args[threadCount].initialNode = conn->destination;
        pthread_create(&threads[threadCount], NULL, threadBFS, &args[threadCount]);
        threadCount++;
        conn = conn->next;
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }
    endTime = clock();
    double parTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    printf("Time (parallel): %f seconds\n", parTime);
}

int main() {
    Graph smallGraph;
    initGraph(&smallGraph);
    int A = addDepartment(&smallGraph, "A");
    int B = addDepartment(&smallGraph, "B");
    int C = addDepartment(&smallGraph, "C");
    addConnection(&smallGraph, A, B);
    addConnection(&smallGraph, B, C);
    addConnection(&smallGraph, A, C);
    runSequentialAndParallel(&smallGraph, A, C, "Example 1: Sequential is faster (small graph)");

    Graph largeGraph;
    initGraph(&largeGraph);
    int nodes[20];
    for (int i = 0; i < 20; i++) {
        char name[10];
        sprintf(name, "D%d", i);
        nodes[i] = addDepartment(&largeGraph, name);
    }
    for (int i = 0; i < 20; i++) {
        for (int j = i + 1; j < 20 && j < i + 5; j++) {
            addConnection(&largeGraph, nodes[i], nodes[j]);
            addConnection(&largeGraph, nodes[j], nodes[i]);
        }
    }
    runSequentialAndParallel(&largeGraph, nodes[0], nodes[19], "Example 2: Parallel is faster (large graph)");

    return 0;
}
