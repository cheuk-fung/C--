int a;
char c;
double d;

int main()
{
    srand(time(NULL)); // Any header file is needless here to use these functions.

    puts("Type and variable support test.");
    puts("Global variables:");
    puts("\tint a;");
    puts("\tchar c;");
    puts("\tdouble d;");
    printf("Input an integer (a): ");
    scanf("%d", &a);
    printf("Input a char (c): ");
    scanf(" %c", &c);
    printf("Input a double number (d): ");
    scanf("%lf", &d);
    printf("Result:\n");
    printf("\ta = %d\n", a);
    printf("\tc = %c\n", c);
    printf("\td = %.16f\n", d);

    putchar(10);

    int x;
    puts("Local variable: int x;");
    x = rand();
    printf("Give x a random value by rand(): x = %d\n", x);
    x = rand();
    printf("again: x = %d\n", x);

    putchar(10);

    printf("Global variable c is '%c'.\n", c);
    char c;
    printf("Declare a local variable char c, the value of c is %c\n", c);
    printf("Probably we see a random char or even nothing after 'is'.\n");
    printf("Well, because it is not initialed.\n");
    c = 'A';
    printf("Give c an initial value 'A', then of course c is '%c'.\n", c);
    char d;
    printf("Declare a local variable char d, the same name as the global variable double d.\n");
    d = rand() % 26 + c;
    printf("Give d a random value by rand() % 26 + c: d = %c\n", d);
    d = rand() % 26 + c;
    printf("again: d = %c\n", d);

    putchar(10);
    double f;
    printf("Declare a local variable: double f;\n");
    printf("It's not so easy to generate a random value of floating point number.\n");
    f = 3.141592653589793238;
    printf("Thus I give f the value of pi, namely that,\nf = %.16f\n", f);

    return 0;
}
