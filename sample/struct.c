struct Student {
    int id;
    char gender;
    int age;
};
struct Student A;

int main()
{
    printf("Struct variable test.\n");
    printf("A is a global struct named Student.\n");
    printf("Input some information for it.\n");
    printf("\tStudent ID: ");
    scanf("%d", &A.id);
    printf("\tGender ('F' for female, 'M' for male): ");
    scanf(" %c", &A.gender);
    printf("\tAge: ");
    scanf("%d", &A.age);
    printf("Information for student A:\n");
    printf("\tStudent ID: %d\n", A.id);
    printf("\tGender: %c\n", A.gender);
    printf("\tAge: %d\n", A.age);
    putchar(10);

    struct Student B;
    printf("B is a local Student structure.\n");
    printf("Input some information for it.\n");
    printf("\tStudent ID: ");
    scanf("%d", &B.id);
    printf("\tGender ('F' for female, 'M' for male): ");
    scanf(" %c", &B.gender);
    printf("\tAge: ");
    scanf("%d", &B.age);
    printf("Information for student B:\n");
    printf("\tStudent ID: %d\n", B.id);
    printf("\tGender: %c\n", B.gender);
    printf("\tAge: %d\n", B.age);

    return 0;
}
