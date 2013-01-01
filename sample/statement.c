int main()
{
    printf("Statement test.\n");
    printf("1. if-else\n");
    printf("Input an integer (if it is zero, 'FALSE' is printed; otherwise 'TRUE'): ");
    int n;
    scanf("%d", &n);
    if (n) puts("TRUE");
    else puts("FALSE");
    printf("I guess you want to try one more time: ");
    scanf("%d", &n);
    if (n) puts("TRUE");
    else puts("FALSE");
    putchar(10);

    printf("2. while\n");
    printf("Input an integer n and I will print n downto 0: ");
    scanf("%d", &n);
    while (n >= 0) {
        printf("%d ", n--);
    }
    putchar(10);
    putchar(10);

    printf("3. for\n");
    printf("Input an integer n and I will print 0 to n: ");
    scanf("%d", &n);
    int i;
    for (i = 0; i <= n; i++) {
        printf("%d ", i);
    }
    putchar(10);

    return 0;
}
