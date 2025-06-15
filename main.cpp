#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h> // for INT_MAX

#define MAX_CITIES 10000
#define MAX_NAME_LENGTH 20
#define MAX_DISTANCE 1000
#define MAX_COST 1000

const char *CITY_NAMES[] = {
    "CityA", "CityB", "CityC", "CityD", "CityE",
    "CityF", "CityG", "CityH", "CityI", "CityJ"
};

typedef struct {
    char source[MAX_NAME_LENGTH];
    char destination[MAX_NAME_LENGTH];
    int distance;
    int cost;
} Route;

void generate_route(FILE *file, int index) {
    char source[MAX_NAME_LENGTH];
    char destination[MAX_NAME_LENGTH];
    int distance = 1 + rand() % MAX_DISTANCE;
    int cost = 1 + rand() % MAX_COST;
    int source_index = rand() % (sizeof(CITY_NAMES) / sizeof(CITY_NAMES[0]));
    int dest_index;

    do {
        dest_index = rand() % (sizeof(CITY_NAMES) / sizeof(CITY_NAMES[0]));
    } while (dest_index == source_index);

    strcpy(source, CITY_NAMES[source_index]);
    strcpy(destination, CITY_NAMES[dest_index]);

    fprintf(file, "%s %s %d %d\n", source, destination, distance, cost);
}

void merge(Route arr[], int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;
    Route *L = (Route *)malloc(n1 * sizeof(Route));
    Route *R = (Route *)malloc(n2 * sizeof(Route));

    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (L[i].distance <= R[j].distance) arr[k++] = L[i++];
        else arr[k++] = R[j++];
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

void merge_sort(Route arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        merge_sort(arr, l, m);
        merge_sort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

int knapsack(Route routes[], int n, int budget, int selected[]) {
    int **dp = (int **)malloc((n + 1) * sizeof(int *));
    for (int i = 0; i <= n; i++) {
        dp[i] = (int *)malloc((budget + 1) * sizeof(int));
    }

    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= budget; j++) {
            if (i == 0 || j == 0)
                dp[i][j] = 0;
            else if (routes[i - 1].cost <= j)
                dp[i][j] = (dp[i - 1][j] > dp[i - 1][j - routes[i - 1].cost] + routes[i - 1].distance)
                            ? dp[i - 1][j]
                            : dp[i - 1][j - routes[i - 1].cost] + routes[i - 1].distance;
            else
                dp[i][j] = dp[i - 1][j];
        }
    }

    int remaining_budget = budget;
    for (int i = n; i > 0; i--) {
        if (dp[i][remaining_budget] != dp[i - 1][remaining_budget]) {
            selected[i - 1] = 1;
            remaining_budget -= routes[i - 1].cost;
        }
    }

    int result = dp[n][budget];
    for (int i = 0; i <= n; i++) free(dp[i]);
    free(dp);
    return result;
}

int coin_change(Route routes[], int n, int budget, int selected[]) {
    int *dp = (int *)malloc((budget + 1) * sizeof(int));
    for (int i = 0; i <= budget; i++) dp[i] = INT_MAX;
    dp[0] = 0;

    for (int i = 1; i <= budget; i++) {
        for (int j = 0; j < n; j++) {
            if (routes[j].cost <= i && dp[i - routes[j].cost] != INT_MAX) {
                if (dp[i - routes[j].cost] + 1 < dp[i]) {
                    dp[i] = dp[i - routes[j].cost] + 1;
                }
            }
        }
    }

    int remaining_budget = budget;
    for (int i = n - 1; i >= 0; i--) {
        while (remaining_budget >= routes[i].cost &&
               dp[remaining_budget] == dp[remaining_budget - routes[i].cost] + 1) {
            selected[i] = 1;
            remaining_budget -= routes[i].cost;
        }
    }

    int result = dp[budget];
    free(dp);
    return result;
}

long long get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int main() {
    Route *routes = (Route *)malloc(MAX_CITIES * sizeof(Route));
    int *selected = (int *)calloc(MAX_CITIES, sizeof(int));
    int n = 0, budget = 0;
    long long start, end;
    double merge_sort_time, knapsack_time, coin_change_time;

    srand(time(NULL));

    printf("Enter the number of routes to generate (100 to 10000): ");
    if (scanf("%d", &n) != 1 || n < 100 || n > MAX_CITIES) {
        printf("Invalid input. Please enter a number between 100 and %d.\n", MAX_CITIES);
        return 1;
    }

    printf("Enter the budget for travel: ");
    if (scanf("%d", &budget) != 1 || budget <= 0) {
        printf("Invalid input. Please enter a positive budget.\n");
        return 1;
    }

    FILE *file = fopen("input.txt", "w");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    fprintf(file, "%d\n", budget);
    for (int i = 0; i < n; i++) {
        generate_route(file, i);
    }
    fclose(file);

    FILE *input_file = fopen("input.txt", "r");
    if (!input_file) {
        perror("Error opening input file");
        return 1;
    }

    fscanf(input_file, "%d", &budget);
    n = 0;
    while (fscanf(input_file, "%s %s %d %d", routes[n].source, routes[n].destination,
                  &routes[n].distance, &routes[n].cost) != EOF) {
        n++;
    }
    fclose(input_file);

    start = get_time_ns();
    merge_sort(routes, 0, n - 1);
    end = get_time_ns();
    merge_sort_time = (end - start) / 1e9;

    memset(selected, 0, MAX_CITIES * sizeof(int));
    start = get_time_ns();
    int max_distance = knapsack(routes, n, budget, selected);
    end = get_time_ns();
    knapsack_time = (end - start) / 1e9;

    FILE *output_file = fopen("output.txt", "w");
    if (!output_file) {
        perror("Error opening output file");
        return 1;
    }

    fprintf(output_file, "Selected Routes (Knapsack, Max Distance: %d):\n", max_distance);
    for (int i = 0; i < n; i++) {
        if (selected[i]) {
            fprintf(output_file, "%s -> %s (Distance: %d, Cost: %d)\n",
                    routes[i].source, routes[i].destination, routes[i].distance, routes[i].cost);
        }
    }

    memset(selected, 0, MAX_CITIES * sizeof(int));
    start = get_time_ns();
    int min_routes = coin_change(routes, n, budget, selected);
    end = get_time_ns();
    coin_change_time = (end - start) / 1e9;

    fprintf(output_file, "\nSelected Routes (Coin Change, Min Routes: %d):\n", min_routes);
    for (int i = 0; i < n; i++) {
        if (selected[i]) {
            fprintf(output_file, "%s -> %s (Distance: %d, Cost: %d)\n",
                    routes[i].source, routes[i].destination, routes[i].distance, routes[i].cost);
        }
    }
    fclose(output_file);

    FILE *time_file = fopen("time_results.txt", "w");
    if (!time_file) {
        perror("Error opening time results file");
        return 1;
    }

    fprintf(time_file, "Merge Sort Time: %.9f seconds\n", merge_sort_time);
    fprintf(time_file, "Knapsack Time: %.9f seconds\n", knapsack_time);
    fprintf(time_file, "Coin Change Time: %.9f seconds\n", coin_change_time);
    fclose(time_file);

    free(routes);
    free(selected);

    printf("Program executed successfully. Check output.txt and time_results.txt for results.\n");
    return 0;
}
