int a[100];

void qsort(int l, int r)
{
    int i, j, mid;
    i = l;
    j = r;
    mid = a[(l + r) / 2];
    while (i <= j) {
        while (a[i] < mid) i++;
        while (a[j] > mid) j--;
        if (i <= j) {
            int t;
            t = a[i];
            a[i] = a[j];
            a[j] = t;
            i++;
            j--;
        }
    }
    if (i < r) qsort(i, r);
    if (j > l) qsort(l, j);
}

int main()
{
    srand(time(NULL));
    int i;
    for (i = 0; i < 100; i++) a[i] = rand() % 128;

    puts("Before sort:");
    for (i = 0; i < 100; i++) printf("%d ", a[i]);
    putchar(10);
    qsort(0, 99);
    puts("After sort:");
    for (i = 0; i < 100; i++) printf("%d ", a[i]);
    putchar(10);

    return 0;
}
