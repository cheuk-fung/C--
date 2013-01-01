int main()
{
    printf("Arithmetic expression test.\n");

    int a, b, c;
    printf("Input an integer a: ");
    scanf("%d", &a);
    printf("Input an integer b (b != 0): ");
    scanf("%d", &b);
    printf("Input an integer c (c < 32): ");
    scanf("%d", &c);

    printf("a = %d\n", a);
    printf("b = %d\n", b);
    printf("a + b = %d\n", a + b);
    printf("a - b = %d\n", a - b);
    printf("a * b = %d\n", a * b);
    printf("a / b = %d\n", a / b);
    printf("a %% b = %d\n", a % b);
    printf("a & b = %d\n", a & b);
    printf("a | b = %d\n", a | b);
    printf("a ^ b = %d\n", a ^ b);
    printf("a << c = %d\n", a << c);
    printf("a >> c = %d\n", a >> c);
    printf("~a = %d\n", ~a);
    putchar(10);
    printf("before a++:\ta = %d\n", a);
    printf("now a++:\ta = %d\n", a++);
    printf("after a++:\ta = %d\n", a);
    putchar(10);
    printf("before ++a:\ta = %d\n", a);
    printf("now ++a:\ta = %d\n", ++a);
    printf("after ++a:\ta = %d\n", a);
    putchar(10);
    printf("before a--:\ta = %d\n", a);
    printf("now a--:\ta = %d\n", a--);
    printf("after a--:\ta = %d\n", a);
    putchar(10);
    printf("before --a:\ta = %d\n", a);
    printf("now --a:\ta = %d\n", --a);
    printf("after --a:\ta = %d\n", a);
    putchar(10);

    double x, y;
    printf("Input a floating point number x: ");
    scanf("%lf", &x);
    printf("Input a floating point number y (y != 0): ");
    scanf("%lf", &y);

    printf("x = %.16f\n", x);
    printf("y = %.16f\n", y);
    printf("x + y = %.16f\n", x + y);
    printf("x - y = %.16f\n", x - y);
    printf("x * y = %.16f\n", x * y);
    printf("x / y = %.16f\n", x / y);


    return 0;
}
