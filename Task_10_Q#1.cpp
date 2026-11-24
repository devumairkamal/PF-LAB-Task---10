#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME   "expenses.txt"
#define MAX_RECORDS 100
#define MAX_DESC    100
#define BUF_SIZE    256
typedef struct {
    char   description[MAX_DESC];
    double amount;
} Expense;
static int read_line(char *buffer, int size)
{
    size_t len;
    int ch;
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return 0;
    }
    len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
    }
    return 1;
}
static int input_description(char *desc)
{
    for (;;) {
        printf("Description: ");
        if (!read_line(desc, MAX_DESC))
            return 0;
        if (strlen(desc) == 0)
            printf("The description cannot be empty.\n");
        else if (strchr(desc, '|') != NULL)
            printf("Please do not use the '|' character.\n");
        else
            return 1;
    }
}
static int input_amount(double *amount)
{
    char buffer[BUF_SIZE];
    char *end;
    for (;;) {
        printf("Amount: ");
        if (!read_line(buffer, BUF_SIZE))
            return 0;
        *amount = strtod(buffer, &end);
        if (end == buffer) {
            printf("Please enter a number.\n");
            continue;
        }
        while (isspace((unsigned char)*end))
            end++;
        if (*end != '\0' || *amount < 0)
            printf("Please enter a valid amount (0 or more).\n");
        else
            return 1;
    }
}
int main(void)
{
    Expense records[MAX_RECORDS];
    char    answer[BUF_SIZE];
    char    line[BUF_SIZE];
    double  total = 0.0;
    int     count = 0;
    int     number = 0;
    int     i;
    FILE   *fp;
    printf("- - - - - - - -  Personal Expense Diary - - - - --\n");
    while (count < MAX_RECORDS) {
        printf("\nExpense %d\n", count + 1);
        if (!input_description(records[count].description))
            break;
        if (!input_amount(&records[count].amount))
            break;
        count++;
        printf("Add another expense? (y/n): ");
        if (!read_line(answer, BUF_SIZE))
            break;
        if (answer[0] != 'y' && answer[0] != 'Y')
            break;
    }
    if (count > 0) {
        fp = fopen(FILE_NAME, "a");
        if (fp == NULL) {
            perror("Error: could not open expenses.txt for writing");
            return EXIT_FAILURE;
        }
        for (i = 0; i < count; i++)
            fprintf(fp, "%s|%.2f\n", records[i].description, records[i].amount);
        fclose(fp);
        printf("\n%d expense(s) saved to %s.\n", count, FILE_NAME);
    } else {
        printf("\nNo expenses were entered, so nothing was saved.\n");
    }
    fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        perror("Error: could not open expenses.txt for reading");
        return EXIT_FAILURE;
    }
    printf("\n       Recorded Expenses       \n");
    printf("%-5s %-30s %10s\n", "No.", "Description", "Amount");
    printf("-  -  -  -  -  -  -  -  -  -  -  -\n");
    while (fgets(line, BUF_SIZE, fp) != NULL) {
        char *sep = strrchr(line, '|');
        double amount;
        if (sep == NULL)
            continue;
        *sep = '\0';
        amount = atof(sep + 1);
        total += amount;
        number++;
        printf("%-5d %-30s %10.2f\n", number, line, amount);
    }
    fclose(fp);
    printf("-  -  -  -  -  -  -  -  -  -  -  -  -\n");
    printf("%-5s %-30s %10.2f\n", "", "Total", total);
    return EXIT_SUCCESS;
}

