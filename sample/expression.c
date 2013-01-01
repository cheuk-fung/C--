int main()
{
    printf("Arithmetic expression test for integer.\n");
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
    printf("Press enter to continue...");
    getchar(); getchar();
    putchar(10);

    printf("Arithmetic expression test for integer.\n");
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
    printf("Press enter to continue...");
    getchar(); getchar();
    putchar(10);

    printf("Relationship expression test.\n");
    printf("a = %d\n", a);
    printf("b = %d\n", b);
    printf("a == b\t=> %d\n", a == b);
    printf("a != b\t=> %d\n", a != b);
    printf("a > b\t=> %d\n", a > b);
    printf("a >= b\t=> %d\n", a >= b);
    printf("a < b\t=> %d\n", a < b);
    printf("a <= b\t=> %d\n", a <= b);
    printf("Press enter to continue...");
    getchar();
    putchar(10);

    printf("Logical expression test.\n");
    printf("0 && 0\t=> %d\n", 0 && 0);
    printf("0 && 1\t=> %d\n", 0 && 1);
    printf("1 && 0\t=> %d\n", 1 && 0);
    printf("1 && 1\t=> %d\n", 1 && 1);
    printf("0 || 0\t=> %d\n", 0 || 0);
    printf("0 || 1\t=> %d\n", 0 || 1);
    printf("1 || 0\t=> %d\n", 1 || 0);
    printf("1 || 1\t=> %d\n", 1 || 1);
    printf("!0\t=> %d\n", !0);
    printf("!1\t=> %d\n", !1);

    return 0;
}
