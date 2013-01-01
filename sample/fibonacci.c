int fibonacci(int n)
{
    if (n == 0 || n == 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void fibonacci_recursive(int n)
{
    printf("by recursive algorithm: ");
    int i;
    for (i = 0; i <= n; i++) {
        printf("%d ", fibonacci(i));
    }
    putchar(10);
}

void fibonacci_dp(int n)
{
    printf("by dynamic programming: ");
    int *a;
    a = malloc(n * 4); // assume that an int is 4 bytes long
    *a = 0;
    if (n >= 0) printf("%d ", *a);
    *(a + 1) = 1;
    if (n >= 1) printf("%d ", *(a + 1));
    int i;
    for (i = 2; i <= n; i++) {
        *(a + i) = *(a + (i - 1)) + *(a + (i - 2));
        printf("%d ", *(a + i));
    }
    putchar(10);
    free(a);
}

int main()
{
    printf("Generates the first n Fibonacci numbers.\n");
    printf("Input n: ");

    int n;
    scanf("%d", &n);

    printf("Results\n");
    fibonacci_dp(n);
    fibonacci_recursive(n);

    return 0;
}
