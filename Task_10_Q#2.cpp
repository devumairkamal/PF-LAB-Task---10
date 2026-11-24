#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME  "lostfound.txt"
#define FIELD_SIZE 100
#define BUF_SIZE   256
typedef struct {
    char item[FIELD_SIZE];
    char location[FIELD_SIZE];
    char contact_id[FIELD_SIZE];
} LostRecord;
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
static int input_field(const char *label, char *field)
{
    for (;;) {
        printf("%s: ", label);
        if (!read_line(field, FIELD_SIZE))
            return 0;
        if (strlen(field) == 0)
            printf("This field cannot be empty.\n");
        else if (strchr(field, '|') != NULL)
            printf("Please do not use the '|' character.\n");
        else
            return 1;
    }
}
static void add_record(void)
{
    LostRecord rec;
    FILE *fp;
    printf("\n--- Add New Record ---\n");
    if (!input_field("Item name", rec.item))
        return;
    if (!input_field("Location", rec.location))
        return;
    if (!input_field("Contact ID", rec.contact_id))
        return;
    fp = fopen(FILE_NAME, "a");
    if (fp == NULL) {
        perror("Error: could not open lostfound.txt");
        return;
    }
    fprintf(fp, "%s|%s|%s\n", rec.item, rec.location, rec.contact_id);
    fclose(fp);
    printf("Record saved to %s.\n", FILE_NAME);
}
static int parse_record(char *line, LostRecord *rec)
{
    char *sep1 = strchr(line, '|');
    char *sep2;
    if (sep1 == NULL)
        return 0;
    sep2 = strchr(sep1 + 1, '|');
    if (sep2 == NULL)
        return 0;
    *sep1 = '\0';
    *sep2 = '\0';
    snprintf(rec->item, FIELD_SIZE, "%s", line);
    snprintf(rec->location, FIELD_SIZE, "%s", sep1 + 1);
    snprintf(rec->contact_id, FIELD_SIZE, "%s", sep2 + 1);
    rec->contact_id[strcspn(rec->contact_id, "\n")] = '\0';
    return 1;
}
static void print_record(const LostRecord *rec, int number)
{
    printf("\nRecord %d\n", number);
    printf("  Item name  : %s\n", rec->item);
    printf("  Location   : %s\n", rec->location);
    printf("  Contact ID : %s\n", rec->contact_id);
}
static int same_text(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return *a == *b;   
}
static void display_all(void)
{
    FILE *fp;
    char line[BUF_SIZE];
    LostRecord rec;
    int number = 0;
    fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        if (errno == ENOENT)
            printf("\nNo records yet. %s does not exist.\n", FILE_NAME);
        else
            perror("Error: could not open lostfound.txt");
        return;
    }
    printf("\n- - - - - All Lost and Found Records - - - -  -\n");
    while (fgets(line, BUF_SIZE, fp) != NULL) {
        if (parse_record(line, &rec))
            print_record(&rec, ++number);
    }
    fclose(fp);
    if (number == 0)
        printf("The file contains no valid records.\n");
    else
        printf("\nTotal records: %d\n", number);
}
static void search_by_name(void)
{
    char name[FIELD_SIZE];
    char line[BUF_SIZE];
    LostRecord rec;
    FILE *fp;
    int found = 0;
    printf("\nEnter item name to search: ");
    if (!read_line(name, FIELD_SIZE))
        return;
    if (strlen(name) == 0) {
        printf("No item name was entered.\n");
        return;
    }
    fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        if (errno == ENOENT)
            printf("No matching record was found. (%s does not exist yet)\n", FILE_NAME);
        else
            perror("Error: could not open lostfound.txt");
        return;
    }
    while (fgets(line, BUF_SIZE, fp) != NULL) {
        if (parse_record(line, &rec) && same_text(rec.item, name)) {
            found++;
            print_record(&rec, found);
        }
    }
    fclose(fp);
    if (found == 0)
        printf("No matching record was found for \"%s\".\n", name);
}
int main(void)
{
    char buffer[BUF_SIZE];
    int choice;
    printf("- - - - Lost and Found Record Manager - - - - -\n");
    for (;;) {
        printf("\n1. Add a new record\n");
        printf("2. Display all records\n");
        printf("3. Search by item name\n");
        printf("4. Exit\n");
        printf("Choose an option: ");
        if (!read_line(buffer, BUF_SIZE))
            break;
        choice = atoi(buffer);
        switch (choice) {
        case 1:
            add_record();
            break;
        case 2:
            display_all();
            break;
        case 3:
            search_by_name();
            break;
        case 4:
            printf("Goodbye!\n");
            return EXIT_SUCCESS;
        default:
            printf("Invalid option. Please enter a number from 1 to 4.\n");
            break;
        }
    }
    return EXIT_SUCCESS;
}

