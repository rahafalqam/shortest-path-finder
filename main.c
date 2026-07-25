//Rahaf Alqam 1232633 Sec.2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CITIES 60
#define NAME_LENGTH 50
#define INF 999999

#define HEAP 120 // max. cities 60 but bc. may the same city repeated

typedef struct { //heap for dijikstra to reduce time comp.
    int v;
    int dist;
} HeapNode;

typedef struct {
    HeapNode a[HEAP];
    int size;
} MinHeap;


//Heap Prototypes
void heapSwap(HeapNode *x, HeapNode *y);
int heapEmpty(MinHeap *h);
void heapPush(MinHeap *h, int v, int dist);
HeapNode heapPop(MinHeap *h);



typedef struct node* Node; //node for the graph
struct node {
    int cityIndex; //destination
    int weight;
    Node next;
};

char cityNames[MAX_CITIES][NAME_LENGTH];
Node adjList[MAX_CITIES];
int cityCount = 0;

int currentSource = -1; // no source
double dijTimeMs = 0.0; //initial time
double bfsTimeMs = 0.0;

//for dij.
int dijDist[MAX_CITIES];
int dijPrev[MAX_CITIES];
int dijVisited[MAX_CITIES];


//for BFS
int bfsPrev[MAX_CITIES];
int bfsVisited[MAX_CITIES]; //0 not visited
int bfsHops[MAX_CITIES]; //edges num.


//FUN. PROTOTYPE
int findCityIndex(const char *name);
void addEdge(int u, int v, int w);
int getWeight(int u, int v);

void GraphMenu();


//OP1
int getCityIndex(const char *name);
void LoadCitiesFile();


//OP2
void dijkstra(int src);
void bfs(int src);
void enterSource();

 long long diff_ns(struct timespec a, struct timespec b) { // from girl on Dr.Alaa section (he said to her to use it so avoide time to become ZERO all time )
    return (long long)(b.tv_sec - a.tv_sec) * 1000000000LL
         + (long long)(b.tv_nsec - a.tv_nsec);
}


//OP3
void displayDestination();
void printPathRecursive(int v, int prev[]);
void printDetailedPath(int dest, int parent[]);


//OP4
int lastDest = -1; // no des.
void SaveShortestPathToFile();
void savePathRecursiveToFile(int v, int prev[], FILE *out);

void freeGraph();


int main()
{

int choice;

do {
    GraphMenu();

    printf("Enter your choice: ");
    scanf("%d", &choice);

    switch(choice) {
        case 1:
            LoadCitiesFile();
            break;
        case 2:
            enterSource();
            break;
        case 3:
            displayDestination();
            break;
       case 4:
          SaveShortestPathToFile();
          freeGraph();
          printf("Exiting...\n");
          break;

        default:
            printf("Invalid choice! Try again.\n");
    }
} while(choice != 4);

    return 0;
}


//fun.
int findCityIndex(const char *name) { //check if we have this city
    for (int i = 0; i < cityCount; i++) {
        if (strcmp(cityNames[i], name) == 0)
            return i;
    }
    return -1;
}


//fun.
void addEdge(int u, int v, int w) { // add edge between 2 cities using adj. list

    Node newNode = (Node)malloc(sizeof(struct node));
    newNode->cityIndex = v;
    newNode->weight = w;
    newNode->next = adjList[u];
    adjList[u] = newNode;

    //(Undirected)
    newNode = (Node)malloc(sizeof(struct node));
    newNode->cityIndex = u;
    newNode->weight = w;
    newNode->next = adjList[v];
    adjList[v] = newNode;
}

//fun.
int getWeight(int u, int v) { // for print
    Node p = adjList[u];
    while (p != NULL) {
        if (p->cityIndex == v)
            return p->weight;
        p = p->next;
    }
    return INF;
}


// main menu
void GraphMenu() {
    printf("\n==== Graph Menu ====\n");
    printf("1. Load cities.txt file\n");
    printf("2. Enter source city (apply Dijkstra and BFS algorithms)\n");
    printf("3. Enter destination city\n");
    printf("4. Save to shortest_path.txt AND Exit\n");
}


//HELPER FOR OP1
int getCityIndex(const char *name) { //return index of city
    for (int i = 0; i < cityCount; i++) {
        if (strcmp(cityNames[i], name) == 0)
            return i;
    }
    if (cityCount >= MAX_CITIES)
        return -1;
    strcpy(cityNames[cityCount], name);
    return cityCount++;
}


//Option 1
void LoadCitiesFile() { //Load cities.txt and build the graph
    FILE* file = fopen("cities.txt", "r");
    if (file == NULL) {
        printf("Cannot open cities.txt file!\n");
        return;
    }

    cityCount = 0;
    for (int i = 0; i < MAX_CITIES; i++) {
        adjList[i] = NULL;
    }

    char line[512];

    while (fgets(line, sizeof(line), file)) {
        char city1[NAME_LENGTH], city2[NAME_LENGTH];
        int distance;
        char *token;

        token = strtok(line, "#");
        if (token == NULL)
            continue;
        strcpy(city1, token);

        token = strtok(NULL, "#");
        if (token == NULL)
            continue;
        strcpy(city2, token);

        token = strtok(NULL, "#");
        if (token == NULL)
            continue;
        distance = atoi(token);

        int i = getCityIndex(city1);
        int j = getCityIndex(city2);

        if (i == -1 || j == -1) { // NUM. OF CITY MORE THAN 60
            printf("ERROR\n");
            break;
        }

          addEdge(i, j, distance);
    }

    fclose(file);
    printf("cities.txt loaded successfully! Cities count = %d\n", cityCount);
}


//HELPER FOR OP2 (dij. fun)
void dijkstra(int src) { // algo. code
    for (int i = 0; i < cityCount; i++) {
        dijDist[i] = INF;
        dijPrev[i] = -1;
        dijVisited[i] = 0;
    }
    dijDist[src] = 0;

    MinHeap h;
    h.size = 0;
    heapPush(&h, src, 0);

    while (!heapEmpty(&h)) {
        HeapNode cur = heapPop(&h);
        int u = cur.v;

        if (cur.dist != dijDist[u]) //skip old
            continue;

             if (dijVisited[u])
                continue;
               dijVisited[u] = 1;

        Node p = adjList[u];
        while (p != NULL) {
            int v = p->cityIndex;
            int w = p->weight;

            if (dijDist[u] != INF) {
                int newCost = dijDist[u] + w;
                if (newCost < dijDist[v]) {
                    dijDist[v] = newCost;
                    dijPrev[v] = u;
                    heapPush(&h, v, newCost);
                }
            }
            p = p->next;
        }
    }
}


//HELPER FOR OP2 (BFS fun)
void bfs(int src) { // algo code
    for (int i = 0; i < cityCount; i++) {
        bfsVisited[i] = 0;
        bfsPrev[i] = -1;
        bfsHops[i] = INF;
    }

    int q[MAX_CITIES];
    int head = 0, tail = 0;

    bfsVisited[src] = 1;
    bfsHops[src] = 0;
    q[tail++] = src;

    while (head < tail) {
        int u = q[head++];

        Node p = adjList[u];
        while (p != NULL) {
            int v = p->cityIndex;

            if (!bfsVisited[v]) {
                bfsVisited[v] = 1;
                bfsPrev[v] = u;
                bfsHops[v] = bfsHops[u] + 1;
                q[tail++] = v;
            }
            p = p->next;
        }
    }
}


//OPTION 2
void enterSource() { // choose the source
    char srcName[NAME_LENGTH];

    if (cityCount == 0) {
        printf("Please load cities first (Option 1).\n");
        return;
    }

    printf("Enter source city: ");
    scanf("%s", srcName);

    int src = findCityIndex(srcName);
    if (src == -1) {
        printf("City not found: %s\n", srcName);
        return;
    }

    currentSource = src;
    lastDest = -1;

    struct timespec s, e;

    //Dijkstra
    clock_gettime(CLOCK_MONOTONIC, &s);
    dijkstra(src);
    clock_gettime(CLOCK_MONOTONIC, &e);
    dijTimeMs = diff_ns(s, e) / 1e6;   //milliseconds

    //BFS
    clock_gettime(CLOCK_MONOTONIC, &s);
    bfs(src);
    clock_gettime(CLOCK_MONOTONIC, &e);
    bfsTimeMs = diff_ns(s, e) / 1e6;   //milliseconds

    printf("\n\nSource set to: %s\n", cityNames[src]);
    printf("Dijkstra time: %.6f ms\n", dijTimeMs);
    printf("BFS time: %.6f ms\n", bfsTimeMs);
}


//HELPER FOR OP3
void printPathRecursive(int v, int prev[]) { // to show all path from S. to D.
    if (v == -1)
        return;

    if (prev[v] == -1) {   // source
        printf("%s", cityNames[v]);
        return;
    }

    printPathRecursive(prev[v], prev);
    printf(" >> %s", cityNames[v]);
}

//HELPER FOR OP3
void printDetailedPath(int dest, int parent[]) { //detail
    int path[MAX_CITIES];
    int count = 0;
    int curr = dest;

    while (curr != -1 && count < MAX_CITIES) {
        path[count++] = curr;
        curr = parent[curr];
    }

    printf("\nFull Route Details:\n");
    for (int i = count - 1; i > 0; i--) {
        int u = path[i];
        int v = path[i - 1];
        int w = getWeight(u, v);

        if (w >= INF) {
            printf("- %s to %s (no direct edge)\n", cityNames[u], cityNames[v]);
        } else {
            printf("- %s to %s (%d km)\n", cityNames[u], cityNames[v], w);
        }
    }
}


//OPTION3
void displayDestination() {
    if (currentSource == -1) {
            printf("Set source first!\n");
    return;
     }
    char destName[NAME_LENGTH];
    printf("Enter destination city: ");
     scanf("%s", destName);
    int dest = findCityIndex(destName);
    if (dest == -1) {
        printf("City not found!\n");
     return;
     }
      lastDest = dest;

    printf("\n\nDijkstra Result (Minimum Distance)\n");
    if (dijDist[dest] == INF)
        printf("No path exists.\n");
    else {
        printf("\nPath: ");
        printPathRecursive(dest, dijPrev);
        printDetailedPath(dest, dijPrev);
        printf("\nTotal Minimum Cost: %d km\n", dijDist[dest]);
        printf("Execution Time: %.3f ms\n", dijTimeMs);
    }

    printf("\n\nBFS Result (Shortest Hops)\n");
    if (bfsPrev[dest] == -1 && dest != currentSource)
        printf("No path exists.\n");
    else {
        printf("\nPath: ");
         printPathRecursive(dest, bfsPrev);
        int totalBFSCost = 0, curr = dest;
        while (bfsPrev[curr] != -1) {
            totalBFSCost += getWeight(bfsPrev[curr], curr);
            curr = bfsPrev[curr];
        }
        printf("\nTotal Path Distance: %d km\nNumber of Hops: %d\n", totalBFSCost, bfsHops[dest]);
        printf("Execution Time: %.3f ms\n", bfsTimeMs);
    }
}


//HELPER FOR OP4
void savePathRecursiveToFile(int v, int prev[], FILE *out) {
    if (v == -1)
        return;
    if (prev[v] == -1) {
        fprintf(out, "%s", cityNames[v]);
        return;
    }
    savePathRecursiveToFile(prev[v], prev, out);
    fprintf(out, " -> %s", cityNames[v]);
}


//OPTION4
void SaveShortestPathToFile() {
    if (currentSource == -1) {
        printf("Error: Please set source city first (Option 2).\n");
        return;
    }

    if (lastDest == -1) {
        printf("Error: Please run Option 3 first (choose destination) before Exit.\n");
        return;
    }

    int dest = lastDest;

    FILE *out = fopen("shortest_path.txt", "w");
    if (out == NULL) {
        printf("Error: Cannot open shortest_path.txt for writing.\n");
        return;
    }

    fprintf(out, "Source City: %s\n", cityNames[currentSource]);
    fprintf(out, "Destination City: %s\n", cityNames[dest]);

    //Dijkstra
    fprintf(out, "\nDijkstra Result (Minimum Distance)\n");
    if (dijDist[dest] == INF) {
        fprintf(out, "No path exists.\n");
    } else {
        fprintf(out, "Path: ");
        savePathRecursiveToFile(dest, dijPrev, out);
        fprintf(out, "\n");

        fprintf(out, "Full Route Details:\n");
        int path[MAX_CITIES], count = 0, curr = dest;
        while (curr != -1 && count < MAX_CITIES) {
            path[count++] = curr;
            curr = dijPrev[curr];
        }
        for (int i = count - 1; i > 0; i--) {
            int u = path[i], v = path[i - 1];
            int w = getWeight(u, v);
            fprintf(out, "- %s to %s (%d km)\n", cityNames[u], cityNames[v], w);
        }

        fprintf(out, "Total Minimum Cost: %d km\n", dijDist[dest]);
        fprintf(out, "Execution Time: %.6f ms\n", dijTimeMs);
    }

    fprintf(out, "\n");

    //BFS
    fprintf(out, "\nBFS Result (Shortest Hops)\n");
    if (bfsPrev[dest] == -1 && dest != currentSource) {
        fprintf(out, "No path exists.\n");
    } else {
        fprintf(out, "Path: ");
        savePathRecursiveToFile(dest, bfsPrev, out);
        fprintf(out, "\n");

        int totalBFSCost = 0, curr_bfs = dest;
        while (curr_bfs != currentSource && bfsPrev[curr_bfs] != -1) {
            totalBFSCost += getWeight(bfsPrev[curr_bfs], curr_bfs);
            curr_bfs = bfsPrev[curr_bfs];
        }

        fprintf(out, "Total Path Distance: %d km\n", totalBFSCost);
        fprintf(out, "Number of Hops: %d\n", bfsHops[dest]);
        fprintf(out, "Execution Time: %.6f ms\n", bfsTimeMs);
    }

    fclose(out);
    printf("shortest_path.txt saved successfully.\n");
}

//FREE FUN.
void freeGraph() {
    for (int i = 0; i < cityCount; i++) {
        Node cur = adjList[i];
        while (cur != NULL) {
            Node tmp = cur;
            cur = cur->next;
            free(tmp);
        }
        adjList[i] = NULL;
    }

    cityCount = 0;
    currentSource = -1;
    lastDest = -1;
}


//Heap Functions
void heapSwap(HeapNode *x, HeapNode *y) {
    HeapNode t = *x;
    *x = *y;
    *y = t;
}

int heapEmpty(MinHeap *h) {
    return h->size == 0;
}

void heapPush(MinHeap *h, int v, int dist) { // add to Min-Heap
    if (h->size >= HEAP)
        return;

    int i = h->size++;
    h->a[i].v = v;
    h->a[i].dist = dist;

    while (i > 0) {
        int p = (i - 1) / 2;
        if (h->a[p].dist <= h->a[i].dist)
            break;
        heapSwap(&h->a[p], &h->a[i]);
        i = p;
    }
}

HeapNode heapPop(MinHeap *h) {// delete from Min-Heap
    HeapNode bad;
    bad.v = -1;
    bad.dist = INF;

    if (h->size == 0)
        return bad;

    HeapNode root = h->a[0];
    h->a[0] = h->a[--h->size];

    int i = 0;
    while (1) {
        int l = 2*i + 1;
        int r = 2*i + 2;
        int smallest = i;

        if (l < h->size && h->a[l].dist < h->a[smallest].dist) smallest = l;
        if (r < h->size && h->a[r].dist < h->a[smallest].dist) smallest = r;

        if (smallest == i)
            break;
        heapSwap(&h->a[i], &h->a[smallest]);
        i = smallest;
    }
    return root;
}
